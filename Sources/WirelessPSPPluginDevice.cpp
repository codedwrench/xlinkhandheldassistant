#include "../Includes/WirelessPSPPluginDevice.h"

/* Copyright (c) 2021 [Rick de Bondt] - WirelessPSPPluginDevice.cpp */

#include <chrono>
#include <cstring>
#include <functional>
#include <string>
#include <thread>

#include "../Includes/NetConversionFunctions.h"
#include "../Includes/XLinkKaiConnection.h"

using namespace std::chrono;

/**
 * Constructor for the PSP plugin device.
 * @param aAutoConnect - Whether or not to connect to networks automatically.
 * @param aReconnectionTimeOut - How long to wait when the connection is stale to reconnect.
 * @param aCurrentlyConnected - Reference to string where the currently connected SSID is stored.
 * @param aPCapWrapper - Shared pointer to the wrapper with pcap functions.
 */
WirelessPSPPluginDevice::WirelessPSPPluginDevice(bool                          aAutoConnect,
                                                 std::chrono::seconds          aReconnectionTimeOut,
                                                 std::string*                  aCurrentlyConnected,
                                                 std::shared_ptr<IPCapWrapper> aPcapWrapper) :
    mAutoConnect(aAutoConnect),
    mWrapper(aPcapWrapper), mReConnectionTimeOut(aReconnectionTimeOut), mCurrentlyConnected(aCurrentlyConnected)
{}

// Keeping the SSID filter in because of future autoconnect
bool WirelessPSPPluginDevice::Open(std::string_view                aName,
                                   std::vector<std::string>&       aSSIDFilter,
                                   std::shared_ptr<IWifiInterface> aInterface)
{
    bool lReturn{true};

    mWifiInterface = aInterface;
    mSSIDFilter    = aSSIDFilter;

    if (mAutoConnect) {
        Connect();
    }

    mAdapterMACAddress = mWifiInterface->GetAdapterMACAddress();
    // Do not try to negiotiate with localhost
    BlackList(mAdapterMACAddress);

    std::array<char, PCAP_ERRBUF_SIZE> lErrorBuffer{};

    mWrapper->Create(aName.data(), lErrorBuffer.data());
    mWrapper->SetSnapLen(cSnapshotLength);
    mWrapper->SetTimeOut(cPCAPTimeoutMs);
    mWrapper->SetDirection(PCAP_D_IN);
    mWrapper->SetImmediateMode(1);

    int lStatus{mWrapper->Activate()};
    if (lStatus == 0) {
        mConnected = true;
    } else {
        lReturn = false;
        Logger::GetInstance().Log("pcap_activate failed, " + std::string(pcap_statustostr(lStatus)),
                                  Logger::Level::ERROR);
    }

    // Reset the timer
    mReadWatchdog = std::chrono::system_clock::now();

    return lReturn;
}

bool WirelessPSPPluginDevice::Open(std::string_view aName, std::vector<std::string>& aSSIDFilter)
{
    return Open(aName, aSSIDFilter, std::make_shared<WifiInterface>(aName));
}

void WirelessPSPPluginDevice::Close()
{
    mConnected = false;

    mWrapper->BreakLoop();

    if (mReceiverThread != nullptr) {
        while (!mReceiverThread->joinable()) {
            // Wait
            std::this_thread::sleep_for(1ms);
        };
        mReceiverThread->join();
    }

    if (mAutoConnect && mWifiTimeoutThread != nullptr) {
        while (!mWifiTimeoutThread->joinable()) {
            // Wait
            std::this_thread::sleep_for(1ms);
        };
        mWifiTimeoutThread->join();
    }

    mWrapper->Close();

    mWrapper        = nullptr;
    SetData(nullptr);
    SetHeader(nullptr);
    mReceiverThread = nullptr;
    mWifiInterface  = nullptr;
}

bool WirelessPSPPluginDevice::ReadCallback(const unsigned char* aData, const pcap_pkthdr* aHeader)
{
    bool lReturn{false};

    // Load all needed information into the handler
    std::string lData{DataToString(aData, aHeader)};
    uint64_t    lSourceMac{
        (GetRawData<uint64_t>(lData, Net_8023_Constants::cSourceAddressIndex) & Net_Constants::cBroadcastMac)};

    if (!IsMACBlackListed(lSourceMac)) {
        if (((GetRawData<uint64_t>(lData, Net_8023_Constants::cDestinationAddressIndex) &
              Net_Constants::cBroadcastMac) == Net_Constants::cBroadcastMac) &&
            (GetRawData<uint16_t>(lData, Net_8023_Constants::cEtherTypeIndex) == Net_Constants::cPSPEtherType)) {
            // Reset the timer so it will not time out
            mReadWatchdog = std::chrono::system_clock::now();

            // Obtain required MACs
            uint64_t lAdapterMAC = mAdapterMACAddress;
            auto     lPSPMAC     = GetRawData<uint64_t>(lData, Net_8023_Constants::cSourceAddressIndex);

            // Tell the PSP what Mac address to use
            std::string lPacket{};
            // Reserve size
            lPacket.resize(Net_8023_Constants::cHeaderLength + Net_8023_Constants::cSourceAddressLength);

            // Now put it into a packet
            int lIndex{0};
            memcpy(lPacket.data(), &lPSPMAC, Net_8023_Constants::cDestinationAddressLength);
            lIndex += Net_8023_Constants::cDestinationAddressLength;

            memcpy(lPacket.data() + lIndex, &lAdapterMAC, Net_8023_Constants::cSourceAddressLength);
            lIndex += Net_8023_Constants::cSourceAddressLength;

            memcpy(lPacket.data() + lIndex, &Net_Constants::cPSPEtherType, Net_8023_Constants::cEtherTypeLength);
            lIndex += Net_8023_Constants::cEtherTypeLength;

            memcpy(lPacket.data() + lIndex, &lAdapterMAC, Net_8023_Constants::cDestinationAddressLength);

            // Log
            Logger::GetInstance().Log("Sending: " + PrettyHexString(lPacket), Logger::Level::TRACE);

            Send(lPacket, false);
        } else if ((GetRawData<uint16_t>(lData, Net_8023_Constants::cEtherTypeIndex) == Net_Constants::cPSPEtherType)) {
            // Log
            Logger::GetInstance().Log("Received: " + PrettyHexString(lData), Logger::Level::TRACE);

            // Reset the timer so it will not time out
            mReadWatchdog = std::chrono::system_clock::now();

            // With the plugin the destination mac is kept at the end of the packet
            std::string lActualDestinationMac{
                lData.substr(lData.size() - Net_8023_Constants::cDestinationAddressLength)};
            lData.replace(Net_8023_Constants::cDestinationAddressIndex,
                          Net_8023_Constants::cDestinationAddressLength,
                          lActualDestinationMac);
            lData.resize(lData.size() - Net_8023_Constants::cDestinationAddressLength);
            GetConnector()->Send(lData);

            SetData(aData);
            SetHeader(aHeader);
            IncreasePacketCount();
        }
    }

    return lReturn;
}

void WirelessPSPPluginDevice::BlackList(uint64_t aMAC)
{
    mBlackList.AddToMACBlackList(aMAC);
}

bool WirelessPSPPluginDevice::Send(std::string_view aData)
{
    return Send(aData, true);
}

bool WirelessPSPPluginDevice::Send(std::string_view aData, bool aModifyData)
{
    bool lReturn{false};
    if (mWrapper->IsActivated()) {
        if (!aData.empty()) {
            std::string lData{aData.data(), aData.size()};

            if (aModifyData) {
                uint64_t lAdapterMAC = mAdapterMACAddress;

                std::string lActualSourceMac{
                    lData.substr(Net_8023_Constants::cSourceAddressIndex, Net_8023_Constants::cSourceAddressLength)};

                memcpy(lData.data() + Net_8023_Constants::cSourceAddressIndex,
                       &lAdapterMAC,
                       Net_8023_Constants::cSourceAddressLength);

                lData.append(lActualSourceMac);
            }

            Logger::GetInstance().Log(std::string("Sent: ") + PrettyHexString(lData), Logger::Level::TRACE);

            if (mWrapper->SendPacket(lData) == 0) {
                lReturn = true;
            } else {
                Logger::GetInstance().Log("pcap_sendpacket failed, " + std::string(mWrapper->GetError()),
                                          Logger::Level::ERROR);
            }
        }
    } else {
        Logger::GetInstance().Log("Cannot send packets on a device that has not been opened yet!",
                                  Logger::Level::ERROR);
    }

    return lReturn;
}

bool WirelessPSPPluginDevice::Connect()
{
    return Connect("");
}

bool WirelessPSPPluginDevice::Connect(std::string_view aESSID)
{
    mPausedAutoConnect = true;
    bool lReturn{};

    if (mWifiInterface != nullptr) {
        if (!aESSID.empty()) {
            mSSIDFilter   = std::vector<std::string>{std::string(aESSID)};
            mSSIDFromHost = true;
        } else {
            // If ReConnect was pressed manually
            // TODO: When adding filter from XLink Kai, that filter must be restored
            mSSIDFilter   = std::vector<std::string>{std::string(Net_Constants::cPSPSSIDFilterName)};
            mSSIDFromHost = false;
        }

        // Send leave IBSS command just in case
        mWifiInterface->LeaveIBSS();

        bool lDidConnect{};
        int  lCount{0};

        // If we are getting the SSID from the host, scanning is too slow, so rather than doing that, connect directly.
        if (!mSSIDFromHost) {
            std::vector<IWifiInterface::WifiInformation>& lNetworks = mWifiInterface->GetAdhocNetworks();
            for (const auto& lNetwork : lNetworks) {
                for (const auto& lFilter : mSSIDFilter) {
                    if (lNetwork.ssid.find(lFilter) != std::string::npos && lNetwork.isadhoc && !lNetwork.isconnected) {
                        lReturn = mWifiInterface->Connect(lNetwork);
                        if (mCurrentlyConnected != nullptr) {
                            *mCurrentlyConnected    = lNetwork.ssid;
                            mCurrentlyConnectedInfo = lNetwork;
                            lDidConnect             = true;
                            if (IsHosting()) {
                                GetConnector()->Send(std::string(XLinkKai_Constants::cSetESSIDString), lNetwork.ssid);
                            }
                        }
                    }
                }
                lCount++;
            }
        }

        // Connect anyway, even if the PSP is not hosting the network
        if (!lDidConnect && mSSIDFromHost && !aESSID.empty()) {
            IWifiInterface::WifiInformation lInformation{};
            lInformation.ssid = aESSID;
            // Use the frequency of the already connected network
            if (mCurrentlyConnectedInfo.frequency != 0) {
                lInformation.frequency = mCurrentlyConnectedInfo.frequency;
            } else {
                // If we have never connected to anything, assume channel 1, might be wrong, we don't know
                lInformation.frequency = 2412;
            }

            lInformation.isadhoc = true;

            Logger::GetInstance().Log("Switching networks due to host broadcast!", Logger::Level::DEBUG);
            lReturn              = mWifiInterface->Connect(lInformation);
            *mCurrentlyConnected = lInformation.ssid;
        }
    }

    // Reset the timer so it won't autoconnect anyway, unless we are hosting
    if (!mSSIDFromHost) {
        mReadWatchdog      = std::chrono::system_clock::now();
        mPausedAutoConnect = false;
    }
    return lReturn;
}

bool WirelessPSPPluginDevice::StartReceiverThread()
{
    bool lReturn{true};

    if (mWrapper->IsActivated()) {
        // Run
        if (mReceiverThread == nullptr) {
            if (mAutoConnect && mWifiTimeoutThread == nullptr && mReConnectionTimeOut.count() > 0) {
                mWifiTimeoutThread = std::make_shared<std::thread>([&] {
                    while (mConnected) {
                        if (!mPausedAutoConnect &&
                            std::chrono::system_clock::now() > (mReadWatchdog + mReConnectionTimeOut)) {
                            Logger::GetInstance().Log("Switching networks due to timeout!", Logger::Level::DEBUG);
                            // Read timed out try to connect to another network.
                            Connect();
                            mReadWatchdog = std::chrono::system_clock::now();
                        }
                        std::this_thread::sleep_for(1s);
                    }
                });
            }

            mReceiverThread = std::make_shared<std::thread>([&] {
                // If we're receiving data from the receiver thread, send it off as well.
                bool lSendReceivedDataOld = mSendReceivedData;
                mSendReceivedData         = true;
                mReadWatchdog             = std::chrono::system_clock::now();

                auto lCallbackFunction =
                    [](unsigned char* aThis, const pcap_pkthdr* aHeader, const unsigned char* aPacket) {
                        auto* lThis = reinterpret_cast<WirelessPSPPluginDevice*>(aThis);
                        lThis->ReadCallback(aPacket, aHeader);
                    };

                while (mConnected && (mWrapper->IsActivated())) {
                    // Use pcap_dispatch instead of pcap_next_ex so that as many packets as possible will be processed
                    // in a single cycle.
                    if (mWrapper->Dispatch(0, lCallbackFunction, reinterpret_cast<u_char*>(this)) == -1) {
                        Logger::GetInstance().Log(
                            "Error occurred while reading packet: " + std::string(mWrapper->GetError()),
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
