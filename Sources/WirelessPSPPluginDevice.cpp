/* Copyright (c) 2021 [Rick de Bondt] - WirelessPSPPluginDevice.cpp */

#include "WirelessPSPPluginDevice.h"

#include <chrono>
#include <string>

#include "NetConversionFunctions.h"
#include "XLinkKaiConnection.h"

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

void WirelessPSPPluginDevice::ObtainTitleId()
{
    try {
        std::string lTitleId{mPacketHandler->GetPacket().substr(Net_8023_Constants::cDataIndex +
                                                                    Net_Constants::cInfoToken.length() +
                                                                    XLinkKai_Constants::cSeparator.length(),
                                                                Net_Constants::cTitleIdLength)};

        // This title id could pick up only null terminators
        if (lTitleId[0] != '\0') {
            SetTitleId(lTitleId);
        }
    } catch (std::out_of_range& aException) {
        Logger::GetInstance().Log("Couldn't read the TitleId to send off to XLink Kai", Logger::Level::ERROR);
    }
}

bool WirelessPSPPluginDevice::ReadCallback(const unsigned char* aData, const pcap_pkthdr* aHeader)
{
    bool lReturn{false};

    // Load all needed information into the handler
    std::string lData{DataToString(aData, aHeader)};
    mPacketHandler->Update(lData);

    if (!mPacketHandler->GetBlackList().IsMacBlackListed(mPacketHandler->GetSourceMac())) {
        // If the packet is a broadcast packet from the psp, go ahead and handshake
        if (mPacketHandler->IsBroadcastPacket() && mPacketHandler->GetEtherType() == Net_Constants::cPSPEtherType) {
            // Log
            Logger::GetInstance().Log("Received: " + PrettyHexString(lData), Logger::Level::TRACE);

            // Reset the timer so it will not time out
            GetReadWatchdog() = std::chrono::system_clock::now();

            // If it's an information packet, just save the information, otherwise we probably need to handshake
            std::string lPacket{ConstructPSPPluginHandshake(mPacketHandler->GetSourceMac(), GetAdapterMacAddress())};

            // Log
            Logger::GetInstance().Log("Sending: " + PrettyHexString(lPacket), Logger::Level::TRACE);

            Send(lPacket, false);
        } else if (mPacketHandler->GetEtherType() == Net_Constants::cPSPEtherType) {
            // Log
            Logger::GetInstance().Log("Received: " + PrettyHexString(lData), Logger::Level::TRACE);

            // Reset the timer so it will not time out
            GetReadWatchdog() = std::chrono::system_clock::now();

            if (mPacketHandler->GetPacket().substr(Net_8023_Constants::cDataIndex,
                                                   Net_Constants::cInfoToken.length()) == Net_Constants::cInfoToken) {
                // Send the title id from the info packet over to XLink Kai.
                ObtainTitleId();
                if (!GetTitleId().empty()) {
                    GetConnector()->SendTitleId(GetTitleId());
                }
            } else {
                // From plugin mode -> 802.3
                lData = mPacketHandler->ConvertPacketOut();
                GetConnector()->Send(lData);

                SetData(aData);
                SetHeader(aHeader);
                IncreasePacketCount();
            }
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