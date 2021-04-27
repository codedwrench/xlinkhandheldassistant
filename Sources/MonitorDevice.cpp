#include "../Includes/MonitorDevice.h"

/* Copyright (c) 2020 [Rick de Bondt] - MonitorDevice.cpp */

#include <chrono>
#include <functional>
#include <string>
#include <thread>

#include "../Includes/NetConversionFunctions.h"
#include "../Includes/XLinkKaiConnection.h"

using namespace std::chrono;

MonitorDevice::MonitorDevice(uint64_t                      aSourceMacToFilter,
                             bool                          aAcknowledgeDataFrames,
                             std::string*                  aCurrentlyConnectedNetwork,
                             std::shared_ptr<IPCapWrapper> aPcapWrapper) :
    mAcknowledgePackets(aAcknowledgeDataFrames),
    mCurrentlyConnectedNetwork(aCurrentlyConnectedNetwork), mPcapWrapper(aPcapWrapper)
{
    if (aSourceMacToFilter != 0) {
        mPacketHandler.AddToMACWhiteList(aSourceMacToFilter);
    }
}

bool MonitorDevice::Open(std::string_view aName, std::vector<std::string>& aSSIDFilter)
{
    bool lReturn{true};

    mPacketHandler.SetSSIDFilterList(aSSIDFilter);

    std::array<char, PCAP_ERRBUF_SIZE> lErrorBuffer{};

    mPcapWrapper->Create(aName.data(), lErrorBuffer.data());
    mPcapWrapper->SetSnapLen(cSnapshotLength);
    mPcapWrapper->SetTimeOut(cTimeout);
    // TODO: Test without immediate mode, see if it helps
    // pcap_set_immediate_mode(mPcapWrapper, 1);

    int lStatus{mPcapWrapper->Activate()};

    if (lStatus == 0) {
        mConnected = true;
    } else {
        lReturn = false;
        Logger::GetInstance().Log("pcap_activate failed, " + std::string(pcap_statustostr(lStatus)),
                                  Logger::Level::ERROR);
    }
    return lReturn;
}

void MonitorDevice::BlackList(uint64_t aMAC)
{
    mPacketHandler.AddToMACBlackList(aMAC);
}

void MonitorDevice::Close()
{
    mConnected = false;

    mPcapWrapper->BreakLoop();

    if (mReceiverThread != nullptr && mReceiverThread->joinable()) {
        mReceiverThread->join();
    }

    mPcapWrapper->Close();

    mData               = nullptr;
    mHeader             = nullptr;
    mReceiverThread     = nullptr;
    mAcknowledgePackets = false;
}

bool MonitorDevice::ReadCallback(const unsigned char* aData, const pcap_pkthdr* aHeader)
{
    bool lReturn{false};

    // Load all needed information into the handler
    std::string lData{DataToString(aData, aHeader)};

    mPacketHandler.Update(lData);

    if (!mPacketHandler.IsDropped()) {
        ShowPacketStatistics(aHeader);
        Logger::GetInstance().Log("Received: " + PrettyHexString(lData), Logger::Level::TRACE);
    }

    if (mAcknowledgePackets && mPacketHandler.IsAckable()) {
        std::string lAcknowledgementFrame =
            ConstructAcknowledgementFrame(mPacketHandler.GetSourceMAC(), mPacketHandler.GetControlPacketParameters());

        Logger::GetInstance().Log("Sent ACK", Logger::Level::TRACE);
        Send(lAcknowledgementFrame);
    }

    // If this packet is convertible to something XLink can understand, send
    if (mPacketHandler.ShouldSend()) {
        std::string lPacket{mPacketHandler.ConvertPacket()};
        mConnector->Send(lPacket);
    }

    mData   = aData;
    mHeader = aHeader;

    // For use in userinterface
    if (mCurrentlyConnectedNetwork != nullptr) {
        *mCurrentlyConnectedNetwork = mPacketHandler.GetLockedSSID();
        if (mHosting) {
            // Send this over XLink Kai
            mConnector->Send(std::string(XLinkKai_Constants::cSetESSIDString), mPacketHandler.GetLockedSSID());
        }
    }

    mPacketCount++;

    return lReturn;
}

void MonitorDevice::ShowPacketStatistics(const pcap_pkthdr* aHeader) const
{
    Logger::GetInstance().Log("Packet # " + std::to_string(mPacketCount), Logger::Level::TRACE);

    // Show the size in bytes of the packet
    Logger::GetInstance().Log("Packet size: " + std::to_string(aHeader->len) + " bytes", Logger::Level::TRACE);

    // Show Epoch Time
    Logger::GetInstance().Log(
        "Epoch time: " + std::to_string(aHeader->ts.tv_sec) + ":" + std::to_string(aHeader->ts.tv_usec),
        Logger::Level::TRACE);

    // Show a warning if the length captured is different
    if (aHeader->len != aHeader->caplen) {
        Logger::GetInstance().Log("Capture size different than packet size:" + std::to_string(aHeader->len) + " bytes",
                                  Logger::Level::TRACE);
    }
}

const unsigned char* MonitorDevice::GetData()
{
    return mData;
}

const RadioTapReader::PhysicalDeviceParameters& MonitorDevice::GetDataPacketParameters()
{
    return mPacketHandler.GetDataPacketParameters();
}

const pcap_pkthdr* MonitorDevice::GetHeader()
{
    return mHeader;
}

uint64_t MonitorDevice::GetLockedBSSID()
{
    return mPacketHandler.GetLockedBSSID();
}

std::string MonitorDevice::DataToString(const unsigned char* aData, const pcap_pkthdr* aHeader)
{
    // Convert from char* to string
    std::string lData{};

    if ((aData != nullptr) && (aHeader != nullptr)) {
        lData.resize(aHeader->caplen);
        for (unsigned int lCount = 0; lCount < aHeader->caplen; lCount++) {
            lData.at(lCount) = aData[lCount];
        }
    }

    return lData;
}

bool MonitorDevice::Send(std::string_view aData)
{
    bool lReturn{false};
    if (mPcapWrapper->IsActivated()) {
        if (!aData.empty()) {
            Logger::GetInstance().Log(std::string("Sent: ") + PrettyHexString(aData), Logger::Level::TRACE);

            if (mPcapWrapper->SendPacket(aData) == 0) {
                lReturn = true;
            } else {
                Logger::GetInstance().Log("pcap_sendpacket failed, " + std::string(mPcapWrapper->GetError()),
                                          Logger::Level::ERROR);
            }
        }
    } else {
        Logger::GetInstance().Log("Cannot send packets on a device that has not been opened yet!",
                                  Logger::Level::ERROR);
    }

    return lReturn;
}

void MonitorDevice::SetConnector(std::shared_ptr<IConnector> aDevice)
{
    mConnector = aDevice;
}

bool MonitorDevice::StartReceiverThread()
{
    bool lReturn{true};
    if (mPcapWrapper->IsActivated()) {
        // Run
        if (mReceiverThread == nullptr) {
            mReceiverThread = std::make_shared<std::thread>([&] {
                // If we're receiving data from the receiver thread, send it off as well.
                bool lSendReceivedDataOld = mSendReceivedData;
                mSendReceivedData         = true;

                auto lCallbackFunction =
                    [](unsigned char* aThis, const pcap_pkthdr* aHeader, const unsigned char* aPacket) {
                        auto* lThis = reinterpret_cast<MonitorDevice*>(aThis);
                        lThis->ReadCallback(aPacket, aHeader);
                    };

                while (mConnected && (mPcapWrapper->IsActivated())) {
                    // Use pcap_dispatch instead of pcap_next_ex so that as many packets as possible will be processed
                    // in a single cycle.
                    if (mPcapWrapper->Dispatch(-1, lCallbackFunction, reinterpret_cast<u_char*>(this)) == -1) {
                        Logger::GetInstance().Log(
                            "Error occurred while reading packet: " + std::string(mPcapWrapper->GetError()),
                            Logger::Level::DEBUG);
                    }
                }

                mSendReceivedData = lSendReceivedDataOld;
            });
        }
    } else {
        Logger::GetInstance().Log("Can't start receiving without a handler!", Logger::Level::ERROR);
        lReturn = false;
    }

    return lReturn;
}

void MonitorDevice::SetSourceMACToFilter(uint64_t aMac)
{
    if (aMac != 0) {
        mPacketHandler.AddToMACWhiteList(aMac);
    }
}

void MonitorDevice::SetAcknowledgePackets(bool aAcknowledge)
{
    mAcknowledgePackets = aAcknowledge;
}

void MonitorDevice::SetHosting(bool aHosting)
{
    mHosting = aHosting;
}
