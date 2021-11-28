/* Copyright (c) 2021 [Rick de Bondt] - WirelessPromiscuousDevice.cpp */

#include "../Includes/WirelessPromiscuousDevice.h"

#include <chrono>
#include <string>

#include "../Includes/NetConversionFunctions.h"

using namespace std::chrono;

/**
 * Constructor for the PSP plugin device.
 * @param aAutoConnect - Whether or not to connect to networks automatically.
 * @param aReconnectionTimeOut - How long to wait when the connection is stale to reconnect.
 * @param aCurrentlyConnected - Reference to string where the currently connected SSID is stored.
 * @param aPacketHandler - The packet handler used to grab things like Mac addresses.
 * @param aPCapWrapper - Shared pointer to the wrapper with pcap functions.
 */
WirelessPromiscuousDevice::WirelessPromiscuousDevice(bool                          aAutoConnect,
                                                     std::chrono::seconds          aReconnectionTimeOut,
                                                     std::string*                  aCurrentlyConnected,
                                                     std::shared_ptr<Handler8023>  aHandler,
                                                     std::shared_ptr<IPCapWrapper> aPcapWrapper) :
    WirelessPromiscuousBase(aAutoConnect, aReconnectionTimeOut, aCurrentlyConnected, aPcapWrapper),
    mPacketHandler(aHandler)
{}

bool WirelessPromiscuousDevice::ReadCallback(const unsigned char* aData, const pcap_pkthdr* aHeader)
{
    bool lReturn{false};

    // Load all needed information into the handler
    std::string lData{DataToString(aData, aHeader)};
    mPacketHandler->Update(lData);

    if (!mPacketHandler->GetBlackList().IsMacBlackListed(mPacketHandler->GetSourceMac())) {
        // Log
        Logger::GetInstance().Log("Received: " + PrettyHexString(lData), Logger::Level::TRACE);

        // Reset the timer so it will not time out
        GetReadWatchdog() = std::chrono::system_clock::now();

        GetConnector()->Send(lData);

        SetData(aData);
        SetHeader(aHeader);
        IncreasePacketCount();
    }

    return lReturn;
}

void WirelessPromiscuousDevice::BlackList(uint64_t aMac)
{
    if (mPacketHandler != nullptr) {
        mPacketHandler->GetBlackList().AddToMacBlackList(aMac);
    }
}

bool WirelessPromiscuousDevice::Send(std::string_view aData)
{
    bool lReturn{false};
    if (GetWrapper()->IsActivated()) {
        if (!aData.empty()) {
            std::string lData{aData.data(), aData.size()};

            // If we got our specific DDS Mac Address, replace it by the one from our adapter.
            if ((GetRawData<uint64_t>(lData.data(), Net_8023_Constants::cSourceAddressIndex) &
                 Net_Constants::cBroadcastMac) == Net_Constants::cDDSReplaceMac) {
                uint64_t lAdapterMacAddress = GetAdapterMacAddress();
                memcpy(lData.data() + Net_8023_Constants::cSourceAddressIndex,
                       &lAdapterMacAddress,
                       Net_8023_Constants::cSourceAddressLength);

                // Check if we are an ARP-Something
                if (GetRawData<uint16_t>(lData.data(), Net_8023_Constants::cEtherTypeIndex) ==
                    Net_Constants::cARPEtherType) {
                    auto lOpCode = GetRawData<uint16_t>(lData.data(), Net_Constants::cARPOpCodeIndex);
                    if (lOpCode == Net_Constants::cARPOpCodeRequest) {
                        // This would also contain the XLink Kai VRRP Mac
                        memcpy(lData.data() + Net_Constants::cARPRequestSenderMacIndex,
                               &lAdapterMacAddress,
                               Net_Constants::cMacAddressLength);
                    } else if (lOpCode == Net_Constants::cARPOpCodeReply) {
                        // This would also contain the XLink Kai VRRP Mac
                        memcpy(lData.data() + Net_Constants::cARPReplyTargetMacIndex,
                               &lAdapterMacAddress,
                               Net_Constants::cMacAddressLength);
                    }
                }
            }

            Logger::GetInstance().Log(std::string("Sent: ") + PrettyHexString(lData), Logger::Level::TRACE);

            if (GetWrapper()->SendPacket(lData) == 0) {
                lReturn = true;
            } else {
                Logger::GetInstance().Log("pcap_sendpacket failed, " + std::string(GetWrapper()->GetError()),
                                          Logger::Level::ERROR);
            }
        }
    } else {
        Logger::GetInstance().Log("Cannot send packets on a device that has not been opened yet!",
                                  Logger::Level::ERROR);
    }

    return lReturn;
}