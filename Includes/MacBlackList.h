#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - MacBlacklist.h
 *
 * This file contains functions related to the Mac address blacklist.
 *
 **/

#include <cstdint>
#include <vector>

class MacBlackList
{
public:
    /**
     * Add the source Mac address to blacklist.
     * @param aMac - Mac address to blacklist.
     */
    void AddToMacBlackList(uint64_t aMac);

    /**
     * Add the source Mac address to whitelist. Whitelist takes prevalence over the blacklist.
     * @param aMac - Mac address to whitelist.
     */
    void AddToMacWhiteList(uint64_t aMac);

    /**
     * Clears blacklist.
     */
    void ClearMacBlackList();

    /**
     * Clears whitelist.
     */
    void ClearMacWhiteList();

    /**
     * Checks if Mac is in receiver blacklist.
     * @param aMac - Mac to check
     * @return true if Mac is blacklisted.
     */
    [[nodiscard]] bool IsMacBlackListed(uint64_t aMac) const;

    /**
     * Checks if this Mac is not blacklisted / whitelisted.
     * @param aMac - Mac to check
     * @return true if Mac address is allowed.
     */
    bool IsMacAllowed(uint64_t aMac);

    /**
     * Sets the source Mac addresses blacklist.
     */
    void SetMacBlackList(std::vector<uint64_t>& aBlackList);

    /**
     * Sets the source Mac addresses whitelist.
     */
    void SetMacWhiteList(std::vector<uint64_t>& aWhiteList);

private:
    std::vector<uint64_t> mBlackList{};
    std::vector<uint64_t> mWhiteList{};
};
