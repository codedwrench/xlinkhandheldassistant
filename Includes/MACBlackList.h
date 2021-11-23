#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - MacBlacklist.h
 *
 * This file contains functions related to the MAC address blacklist.
 *
 **/

#include <cstdint>
#include <vector>

class MACBlackList
{
public:
    /**
     * Add the source MAC address to blacklist.
     * @param aMac - MAC address to blacklist.
     */
    void AddToMACBlackList(uint64_t aMAC);

    /**
     * Add the source MAC address to whitelist. Whitelist takes prevalence over the blacklist.
     * @param aMac - MAC address to whitelist.
     */
    void AddToMACWhiteList(uint64_t aMAC);

    /**
     * Clears blacklist.
     */
    void ClearMACBlackList();

    /**
     * Clears whitelist.
     */
    void ClearMACWhiteList();

    /**
     * Checks if MAC is in receiver blacklist.
     * @param aMAC - MAC to check
     * @return true if MAC is blacklisted.
     */
    [[nodiscard]] bool IsMACBlackListed(uint64_t aMAC) const;

    /**
     * Checks if this MAC is not blacklisted / whitelisted.
     * @param aMAC - MAC to check
     * @return true if MAC address is allowed.
     */
    bool IsMACAllowed(uint64_t aMAC);

    /**
     * Sets the source MAC addresses blacklist.
     */
    void SetMACBlackList(std::vector<uint64_t>& aBlackList);

    /**
     * Sets the source MAC addresses whitelist.
     */
    void SetMACWhiteList(std::vector<uint64_t>& aWhiteList);

private:
    std::vector<uint64_t> mBlackList{};
    std::vector<uint64_t> mWhiteList{};
};
