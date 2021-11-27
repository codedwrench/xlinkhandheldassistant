#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - WirelessPromiscuousDevice.h
 *
 * This file contains functions to capture data from a wireless device in promiscuous mode (or with L2MOD).
 *
 **/

#include <chrono>
#include <memory>
#include <thread>

#include "Handler8023.h"
#include "WirelessPromiscuousBase.h"

/**
 * Class which allows a wireless device normal mode using the PSP plugin to capture data and send wireless frames.
 */
class WirelessPromiscuousDevice : public WirelessPromiscuousBase
{
public:
    explicit WirelessPromiscuousDevice(
        bool                          aAutoConnect         = false,
        std::chrono::seconds          aReConnectionTimeOut = WirelessPromiscuousBase_Constants::cReconnectionTimeOut,
        std::string*                  aCurrentlyConnected  = nullptr,
        std::shared_ptr<Handler8023>  aHandler             = std::make_shared<Handler8023>(),
        std::shared_ptr<IPCapWrapper> aPcapWrapper         = std::make_shared<PCapWrapper>());

    void BlackList(uint64_t aMac) override;

    // Public for easier testing
    bool ReadCallback(const unsigned char* aData, const pcap_pkthdr* aHeader) override;

    bool Send(std::string_view aData) override;

private:
    std::shared_ptr<Handler8023> mPacketHandler{nullptr};
};
