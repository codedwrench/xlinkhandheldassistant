/* Copyright (c) 2021 [Rick de Bondt] - WirelessPSPPluginDevice.cpp */

#include "WirelessPSPPluginDevice.h"

#include <chrono>
#include <string>

#include "NetConversionFunctions.h"

using namespace std::chrono;

/**
 * Constructor for the PSP plugin device.
 * @param aAutoConnect - Whether or not to connect to networks automatically.
 * @param aReconnectionTimeOut - How long to wait when the connection is stale to reconnect.
 * @param aCurrentlyConnected - Reference to string where the currently connected SSID is stored.
 * @param aPacketHandler - The packet handler used to do the conversion from and to plugin mode.
 * @param aPCapWrapper - Shared pointer to the wrapper with pcap functions.
 */
WirelessPSPPluginDevice::WirelessPSPPluginDevice(bool                              aAutoConnect,
                                                 std::chrono::seconds              aReconnectionTimeOut,
                                                 std::string*                      aCurrentlyConnected,
                                                 std::shared_ptr<HandlerPSPPlugin> aHandler,
                                                 std::shared_ptr<IPCapWrapper>     aPcapWrapper) :
    WirelessPromiscuousBase(aAutoConnect, aReconnectionTimeOut, aCurrentlyConnected, aPcapWrapper),
    mPacketHandler(aHandler)
{}

bool WirelessPSPPluginDevice::ReadCallback(const unsigned char* aData, const pcap_pkthdr* aHeader)
{
    bool lReturn{false};

    // Load all needed information into the handler
    std::string lData{DataToString(aData, aHeader)};
    mPacketHandler->Update(lData);

    if (!mPacketHandler->GetBlackList().IsMacBlackListed(mPacketHandler->GetSourceMac())) {
        if (mPacketHandler->IsBroadcastPacket() && mPacketHandler->GetEtherType() == Net_Constants::cPSPEtherType) {
            // Reset the timer so it will not time out
            GetReadWatchdog() = std::chrono::system_clock::now();

            std::string lPacket{ConstructPSPPluginHandshake(mPacketHandler->GetSourceMac(), GetAdapterMacAddress())};

            // Log
            Logger::GetInstance().Log("Sending: " + PrettyHexString(lPacket), Logger::Level::TRACE);

            Send(lPacket, false);
        } else if (mPacketHandler->GetEtherType() == Net_Constants::cPSPEtherType) {
            // Log
            Logger::GetInstance().Log("Received: " + PrettyHexString(lData), Logger::Level::TRACE);

            // Reset the timer so it will not time out
            GetReadWatchdog() = std::chrono::system_clock::now();

            // From plugin mode -> 802.3
            lData = mPacketHandler->ConvertPacketOut();
            GetConnector()->Send(lData);

            SetData(aData);
            SetHeader(aHeader);
            IncreasePacketCount();
        }
    }

    return lReturn;
}

void WirelessPSPPluginDevice::BlackList(uint64_t aMac)
{
    if (mPacketHandler != nullptr) {
        mPacketHandler->GetBlackList().AddToMacBlackList(aMac);
    }
}

bool WirelessPSPPluginDevice::Send(std::string_view aData)
{
    return Send(aData, true);
}

bool WirelessPSPPluginDevice::Send(std::string_view aData, bool aModifyData)
{
    bool lReturn{false};
    if (GetWrapper()->IsActivated()) {
        if (!aData.empty()) {
            std::string lData{aData.data(), aData.size()};

            if (aModifyData) {
                // Convert 8023 -> PSP Plugin
                lData = mPacketHandler->ConvertPacketIn(lData, GetAdapterMacAddress());
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