/* Copyright (c) 2020 [Rick de Bondt] - MonitorDevice.cpp */

#include "MonitorDevice.h"

#include <chrono>
#include <functional>
#include <string>
#include <thread>

#include "NetConversionFunctions.h"
#include "XLinkKaiConnection.h"
namespace
{
    constexpr unsigned int cSnapshotLength{65535};
    constexpr unsigned int cTimeout{1};
}  // namespace

using namespace std::chrono;

MonitorDevice::MonitorDevice(uint64_t                      aSourceMacToFilter,
                             bool                          aAcknowledgeDataFrames,
                             std::string*                  aCurrentlyConnectedNetwork,
                             std::shared_ptr<IPCapWrapper> aPcapWrapper) :
    mAcknowledgePackets(aAcknowledgeDataFrames),
    mCurrentlyConnectedNetwork(aCurrentlyConnectedNetwork), mPcapWrapper(aPcapWrapper)
{
    if (aSourceMacToFilter != 0) {
        mPacketHandler.GetBlackList().AddToMacWhiteList(aSourceMacToFilter);
    }
}

bool MonitorDevice::Connect(std::string_view aESSID)
{
    if (!aESSID.empty()) {
        // Just zoom in on this SSID when using monitor mode
        std::vector<std::string> lList{std::string(aESSID)};
        mPacketHandler.SetSSIDFilterList(lList);
    }
    return true;
}

bool MonitorDevice::Open(std::string_view aName, std::vector<std::string>& aSSIDFilter)
{
    bool lReturn{true};

    mPacketHandler.SetSSIDFilterList(aSSIDFilter);

    std::array<char, PCAP_ERRBUF_SIZE> lErrorBuffer{};

    mPcapWrapper->Create(aName.data(), lErrorBuffer.data());
    mPcapWrapper->SetSnapLen(cSnapshotLength);
    mPcapWrapper->SetTimeOut(cTimeout);
    mPcapWrapper->SetImmediateMode(1);

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

void MonitorDevice::BlackList(uint64_t aMac)
{
    mPacketHandler.GetBlackList().AddToMacBlackList(aMac);
}

void MonitorDevice::Close()
{
    mConnected = false;

    mPcapWrapper->BreakLoop();

    if (mReceiverThread != nullptr && mReceiverThread->joinable()) {
        mReceiverThread->join();
    }

    mPcapWrapper->Close();

    SetData(nullptr);
    SetHeader(nullptr);
    mReceiverThread     = nullptr;
    mAcknowledgePackets = false;
}

bool MonitorDevice::ReadCallback(const unsigned char* aData, const pcap_pkthdr* aHeader)
{
    bool lReturn{false};

    // Load all needed information into the handler
    std::string lData{DataToString(aData, aHeader)};
    std::string lOldSSID{mPacketHandler.GetLockedSSID()};

    mPacketHandler.Update(lData);

    if (!mPacketHandler.IsDropped()) {
        ShowPacketStatistics(aHeader);
        Logger::GetInstance().Log("Received: " + PrettyHexString(lData), Logger::Level::TRACE);
    }

    if (mAcknowledgePackets && mPacketHandler.IsAckable()) {
        std::string lAcknowledgementFrame =
            ConstructAcknowledgementFrame(mPacketHandler.GetSourceMac(), mPacketHandler.GetControlPacketParameters());

        Logger::GetInstance().Log("Sent ACK", Logger::Level::TRACE);
        Send(lAcknowledgementFrame);
    }

    // If this packet is convertible to something XLink can understand, send
    if (mPacketHandler.ShouldSend()) {
        std::string lPacket{mPacketHandler.ConvertPacketOut()};
        GetConnector()->Send(lPacket);
    }

    SetData(aData);
    SetHeader(aHeader);

    if (lOldSSID != mPacketHandler.GetLockedSSID()) {
        // For use in userinterface
        if (mCurrentlyConnectedNetwork != nullptr) {
            *mCurrentlyConnectedNetwork = mPacketHandler.GetLockedSSID();
        }

        // Set this regardless of hosting, this is info for XLink Kai
        GetConnector()->Send(std::string(XLinkKai_Constants::cInfoSetESSIDString), mPacketHandler.GetLockedSSID());

        if (IsHosting()) {
            // Send this over XLink Kai
            GetConnector()->Send(std::string(XLinkKai_Constants::cSetESSIDString), mPacketHandler.GetLockedSSID());
        }
    }

    IncreasePacketCount();

    return lReturn;
}

const RadioTapReader::PhysicalDeviceParameters& MonitorDevice::GetDataPacketParameters()
{
    return mPacketHandler.GetDataPacketParameters();
}

uint64_t MonitorDevice::GetLockedBSSID()
{
    return mPacketHandler.GetLockedBSSID();
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

void MonitorDevice::SetSourceMacToFilter(uint64_t aMac)
{
    if (aMac != 0) {
        mPacketHandler.GetBlackList().AddToMacWhiteList(aMac);
    }
}

void MonitorDevice::SetAcknowledgePackets(bool aAcknowledge)
{
    mAcknowledgePackets = aAcknowledge;
}
