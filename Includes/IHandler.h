#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - IHandler.h
 *
 * This file contains an interface for packet handlers.
 *
 **/

#include <string_view>

#include "MacBlackList.h"

class IHandler
{
public:
    /**
     * Gets destination Mac address of packet.
     * @return the destination Mac address.
     */
    [[nodiscard]] virtual uint64_t GetDestinationMac() const = 0;

    /**
     * Gets blacklist wrapper, so functions can be called on it.
     * @return the blacklist.
     */
    virtual MacBlackList& GetBlackList() = 0;

    /**
     * Gets the EtherType of specified packet.
     *
     * @return the EtherType
     */
    [[nodiscard]] virtual uint16_t GetEtherType() const = 0;

    /**
     * Gets packet saved in this class.
     * @return string_view with data.
     */
    virtual std::string_view GetPacket() = 0;

    /**
     * Gets source Mac address of packet.
     * @return the source Mac address.
     */
    [[nodiscard]] virtual uint64_t GetSourceMac() const = 0;

    /**
     * Preload data about this packet into this class.
     * @param aData - Packet to dissect.
     */
    virtual void Update(std::string_view aPacket) = 0;
};