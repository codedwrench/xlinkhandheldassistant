#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - Handler8023.h
 *
 * This file reads packets from a promiscuous format and converts to a monitor format.
 *
 **/

#include <memory>
#include <string>
#include <vector>

#include "IHandler.h"
#include "NetworkingHeaders.h"
#include "Parameter80211Reader.h"
#include "RadioTapReader.h"

/**
 * This class reads packets from a monitor format and converts to a promiscuous format.
 **/
class Handler8023 : public IHandler
{
public:
    /**
     * This function converts a promiscuous mode packet to a monitor mode packet, adding the radiotap and
     * 802.11 header and removing the 802.3 header.
     * @param aBSSID - BSSID to use when inserting the 80211 header.
     * @param aParameters - Parameters to use to convert to 80211.
     * @return converted packet data, empty string if failed.
     */
    std::string ConvertPacketOut(uint64_t aBSSID, RadioTapReader::PhysicalDeviceParameters aParameters);

    MacBlackList& GetBlackList() override;

    [[nodiscard]] uint64_t GetDestinationMac() const override;

    [[nodiscard]] uint16_t GetEtherType() const override;

    std::string_view GetPacket() override;

    [[nodiscard]] uint64_t GetSourceMac() const override;

    [[nodiscard]] bool IsBroadcastPacket() const override;

    /**
     * Preloads data from 802.3 header into this object.
     * @param aPacket - Packet to use for loading data.
     */
    void Update(std::string_view aPacket) override;

private:
    MacBlackList mBlackList{};
    uint16_t     mEtherType{};
    bool         mIsBroadcastPacket{};
    std::string  mLastReceivedData{};
    uint64_t     mSourceMac{0};
    uint64_t     mDestinationMac{0};
};
