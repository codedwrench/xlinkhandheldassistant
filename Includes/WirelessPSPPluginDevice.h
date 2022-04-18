#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - WirelessPSPPluginDevice.h
 *
 * This file contains functions to capture data from a wireless device in normal mode with the PSP plugin.
 *
 **/

#include <chrono>
#include <memory>
#include <thread>

#include "HandlerPSPPlugin.h"
#include "WirelessPromiscuousBase.h"

/**
 * Class which allows a wireless device normal mode using the PSP plugin to capture data and send wireless frames.
 */
class WirelessPSPPluginDevice : public WirelessPromiscuousBase
{
public:
    explicit WirelessPSPPluginDevice(
        bool                 aAutoConnect              = false,
        std::chrono::seconds aReConnectionTimeOut      = WirelessPromiscuousBase_Constants::cReconnectionTimeOut,
        std::string*         aCurrentlyConnected       = nullptr,
        std::shared_ptr<HandlerPSPPlugin> aHandler     = std::make_shared<HandlerPSPPlugin>(),
        std::shared_ptr<IPCapWrapper>     aPcapWrapper = std::make_shared<PCapWrapper>());

    void BlackList(uint64_t aMac) override;

    // Public for easier testing
    bool ReadCallback(const unsigned char* aData, const pcap_pkthdr* aHeader) override;

    bool Send(std::string_view aData, bool aModifyData);

    bool Send(std::string_view aData) override;

private:
    /**
     * Sends the title id gotten from the handshake to XLink Kai.
     */
    void SendTitleId();

    std::shared_ptr<HandlerPSPPlugin> mPacketHandler{nullptr};
};
