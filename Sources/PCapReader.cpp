/* Copyright (c) 2020 [Rick de Bondt] - PCapReader.cpp */

#include "PCapReader.h"

#include <chrono>
#include <thread>

#include "Logger.h"
#include "NetConversionFunctions.h"

using namespace std::chrono;

PCapReader::PCapReader(bool                          aMonitorCapture,
                       bool                          aMonitorOutput,
                       bool                          aTimeAccurate,
                       std::shared_ptr<IPCapWrapper> aPcapWrapper) :
    mWrapper(aPcapWrapper),
    mMonitorCapture(aMonitorCapture), mMonitorOutput(aMonitorOutput), mTimeAccurate(aTimeAccurate)
{}

void PCapReader::BlackList(uint64_t aMac)
{
    auto lHandler = std::dynamic_pointer_cast<Handler80211>(mPacketHandler);
    if (lHandler != nullptr) {
        mPacketHandler->GetBlackList().AddToMacBlackList(aMac);
    }
}

void PCapReader::Close()
{
    if ((mReplayThread != nullptr) && mReplayThread->joinable()) {
        mReplayThread->join();
    }

    mWrapper->Close();

    mWrapper = nullptr;
    SetData(nullptr);
    SetHeader(nullptr);
    mReplayThread       = nullptr;
    mAcknowledgePackets = false;
}

bool PCapReader::Connect(std::string_view /*aESSID*/)
{
    // Unused in the simulator for now
    return false;
}

uint64_t PCapReader::GetBSSID() const
{
    uint64_t lBSSID{mBSSID};

    if (mMonitorCapture) {
        auto lHandler = std::dynamic_pointer_cast<Handler80211>(mPacketHandler);
        if (lHandler != nullptr) {
            lBSSID = lHandler->GetLockedBSSID();
        }
    }

    return lBSSID;
}

std::shared_ptr<IHandler> PCapReader::GetPacketHandler()
{
    return mPacketHandler;
}

std::shared_ptr<RadioTapReader::PhysicalDeviceParameters> PCapReader::GetDataParameters()
{
    std::shared_ptr<RadioTapReader::PhysicalDeviceParameters> lParameters{mParameters};

    if (mMonitorCapture) {
        auto lHandler = std::dynamic_pointer_cast<Handler80211>(mPacketHandler);
        if (lHandler != nullptr) {
            lParameters =
                std::make_shared<RadioTapReader::PhysicalDeviceParameters>(lHandler->GetDataPacketParameters());
        }
    }

    return lParameters;
}

bool PCapReader::IsDoneReceiving() const
{
    return mDoneReceiving;
}

bool PCapReader::Open(std::string_view aArgument)
{
    mMonitorCapture = false;
    // Create an 8023 handler, this is going to be a promiscuous capture
    mPacketHandler = std::make_shared<Handler8023>();

    bool                               lReturn{true};
    std::array<char, PCAP_ERRBUF_SIZE> lErrorBuffer{};
    mWrapper->OpenOffline(aArgument.data(), lErrorBuffer.data());

    if (mWrapper == nullptr) {
        lReturn = false;
        Logger::GetInstance().Log("pcap_open_offline failed, " + std::string(lErrorBuffer.data()),
                                  Logger::Level::ERROR);
    }

    return lReturn;
}


bool PCapReader::Open(std::string_view aName, std::vector<std::string>& aSSIDFilter)
{
    mMonitorCapture = true;
    // Create an 80211 handler, this is going to ge a monitor capture
    mPacketHandler = std::make_shared<Handler80211>();

    bool                               lReturn{true};
    std::array<char, PCAP_ERRBUF_SIZE> lErrorBuffer{};
    mWrapper->OpenOffline(aName.data(), lErrorBuffer.data());
    if (mWrapper->IsActivated()) {
        // If we have a monitor device we want the 80211 handler.
        auto lHandler{std::dynamic_pointer_cast<Handler80211>(mPacketHandler)};
        lHandler->SetSSIDFilterList(aSSIDFilter);
    } else {
        lReturn = false;
        Logger::GetInstance().Log("pcap_open_offline failed, " + std::string(lErrorBuffer.data()),
                                  Logger::Level::ERROR);
    }

    return lReturn;
}

bool PCapReader::ReadCallback(const unsigned char* aData, const pcap_pkthdr* aHeader)
{
    bool lReturn{false};

    // Load all needed information into the handler
    std::string lData{DataToString(aData, aHeader)};

    mPacketHandler->Update(lData);

    if (mMonitorCapture) {
        // If we have a monitor device we want the 80211 handler.
        auto lHandler{std::dynamic_pointer_cast<Handler80211>(mPacketHandler)};

        if (lHandler != nullptr) {
            if (!lHandler->IsDropped()) {
                ShowPacketStatistics(aHeader);
                Logger::GetInstance().Log("Received: " + PrettyHexString(lData), Logger::Level::TRACE);
            }

            if (mAcknowledgePackets && lHandler->IsAckable()) {
                std::string lAcknowledgementFrame = ConstructAcknowledgementFrame(
                    mPacketHandler->GetSourceMac(), lHandler->GetControlPacketParameters());

                Logger::GetInstance().Log("Sent ACK", Logger::Level::TRACE);
                Send(lAcknowledgementFrame);
            }

            // If this packet is convertible to something XLink can understand, send
            if (lHandler->ShouldSend()) {
                GetConnector()->Send(lHandler->ConvertPacketOut());
            }
        }
    } else {
        // Pretending to be XLink Kai or other outgoing connector
        if (mIncomingConnection != nullptr) {
            if (mMonitorOutput) {
                auto lHandler = std::dynamic_pointer_cast<Handler8023>(mPacketHandler);
                if (lHandler != nullptr) {
                    mIncomingConnection->Send(lHandler->ConvertPacketOut(mBSSID, *mParameters));
                }
            } else {
                mIncomingConnection->Send(lData);
            }
        } else {
            GetConnector()->Send(lData);
        }
    }
    IncreasePacketCount();

    return lReturn;
}

bool PCapReader::ReadNextData()
{
    bool lReturn = true;

    pcap_pkthdr*         lHeader = nullptr;
    const unsigned char* lData   = nullptr;
    if (mWrapper->NextEx(&lHeader, &lData) < 0) {
        Logger::GetInstance().Log("Reading offline capture failed: " + std::string(mWrapper->GetError()),
                                  Logger::Level::ERROR);
        lReturn = false;
    } else {
        SetHeader(lHeader);
        SetData(lData);
    }

    return lReturn;
}

bool PCapReader::Send(std::string_view aCommand, std::string_view aData)
{
    bool lReturn{false};
    if (mWrapper->IsActivated()) {
        if (!aData.empty()) {
            Logger::GetInstance().Log(std::string("Would have sent: ") + aCommand.data() + aData.data(),
                                      Logger::Level::TRACE);
        }
    } else {
        Logger::GetInstance().Log("Cannot send packets on a device that has not been opened yet!",
                                  Logger::Level::ERROR);
    }

    return lReturn;
}

bool PCapReader::Send(std::string_view aData)
{
    bool lReturn{false};
    if (mWrapper->IsActivated()) {
        if (!aData.empty()) {
            Logger::GetInstance().Log(std::string("Would have sent: ") + PrettyHexString(aData), Logger::Level::TRACE);
        }
    } else {
        Logger::GetInstance().Log("Cannot send packets on a device that has not been opened yet!",
                                  Logger::Level::ERROR);
    }

    return lReturn;
}

void PCapReader::SetBSSID(uint64_t aBSSID)
{
    mBSSID = aBSSID;
}

void PCapReader::SetParameters(std::shared_ptr<RadioTapReader::PhysicalDeviceParameters> aParameters)
{
    mParameters = std::move(aParameters);
}

void PCapReader::SetIncomingConnection(std::shared_ptr<IPCapDevice> aDevice)
{
    mIncomingConnection = aDevice;
}

bool PCapReader::StartReceiverThread()
{
    bool lReturn{true};
    if (mWrapper->IsActivated()) {
        // Run
        if (mReplayThread == nullptr) {
            mReplayThread = std::make_shared<std::thread>([&] {
                if (ReadNextData()) {
                    microseconds lTimeStamp{GetHeader()->ts.tv_sec * 1000000 + GetHeader()->ts.tv_usec};

                    ReadCallback(GetData(), GetHeader());

                    while (ReadNextData()) {
                        // Get time offset.
                        microseconds lSleepFor{GetHeader()->ts.tv_sec * 1000000 + GetHeader()->ts.tv_usec -
                                               lTimeStamp.count()};
                        lTimeStamp = microseconds(GetHeader()->ts.tv_sec * 1000000 + GetHeader()->ts.tv_usec);

                        if (mTimeAccurate) {
                            // Wait for next send.
                            std::this_thread::sleep_for(lSleepFor);
                        }

                        // Wait for next send.
                        ReadCallback(GetData(), GetHeader());
                    }
                }
                mDoneReceiving = true;
            });
        } else {
            Logger::GetInstance().Log("Can't start receiving without a handler!", Logger::Level::ERROR);
            lReturn = false;
        }
    }
    return lReturn;
}

void PCapReader::SetSourceMacToFilter(uint64_t aMac)
{
    if (aMac != 0) {
        mPacketHandler->GetBlackList().AddToMacWhiteList(aMac);
    }
}

void PCapReader::SetAcknowledgePackets(bool aAcknowledge)
{
    mAcknowledgePackets = aAcknowledge;
}
