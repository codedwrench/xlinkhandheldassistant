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
 * This class reads packets from a PSP plugin format and converts to a promiscuous format.
 **/
class HandlerPSPPlugin : public IHandler
{
public:
    /**
     * This function converts a PSP plugin packet to a promiscuous mode packet.
     * @return converted packet data, empty string if failed.
     */
    std::string ConvertPacketOut();

    /**
     * This function converts a promiscuous mode packet to a PSP plugin packet.
     * @param aData - The data to convert to a PSP plugin packet.
     * @param aAdapterMac - The mac address to put into the original destination field.
     * @return converted packet data, empty string if failed.
     */
    std::string ConvertPacketIn(std::string_view aData, uint64_t aAdapterMac);

    MacBlackList& GetBlackList() override;

    [[nodiscard]] uint64_t GetDestinationMac() const override;

    [[nodiscard]] uint16_t GetEtherType() const override;

    std::string_view       GetPacket() override;
    [[nodiscard]] uint64_t GetSourceMac() const override;

    /**
     * Tells if the last received packet was a broacast packet.
     * @return true if it was.
     */
    [[nodiscard]] bool IsBroadcastPacket() const;

    /**
     * Preloads data from PSP header into this object.
     * @param aPacket - Packet to use for loading data.
     */
    void Update(std::string_view aPacket) override;

private:
    MacBlackList mBlackList{};
    bool         mIsBroadcastPacket{};
    std::string  mLastReceivedData{};
    uint64_t     mSourceMac{0};
    uint64_t     mDestinationMac{0};
    uint16_t     mEtherType{};
};
