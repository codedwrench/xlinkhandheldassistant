/* Copyright (c) 2021 [Rick de Bondt] - MacBlackList.cpp */

#include "../Includes/MACBlackList.h"

#include "../Includes/Logger.h"
#include "../Includes/NetConversionFunctions.h"

void MACBlackList::AddToMACBlackList(uint64_t aMAC)
{
    if (IsMACAllowed(aMAC)) {
        Logger::GetInstance().Log("Added: " + IntToMac(aMAC) + " to blacklist.", Logger::Level::TRACE);
        mBlackList.push_back(aMAC);
    }
}

void MACBlackList::AddToMACWhiteList(uint64_t aMAC)
{
    Logger::GetInstance().Log("Added: " + IntToMac(aMAC) + " to whitelist.", Logger::Level::TRACE);
    mWhiteList.push_back(aMAC);
}

void MACBlackList::ClearMACBlackList()
{
    mBlackList.clear();
}

void MACBlackList::ClearMACWhiteList()
{
    mWhiteList.clear();
}

bool MACBlackList::IsMACAllowed(uint64_t aMAC)
{
    bool lReturn{false};

    if (mWhiteList.empty()) {
        if (std::find(mBlackList.begin(), mBlackList.end(), aMAC) == mBlackList.end()) {
            lReturn = true;
        }
    } else {
        if (std::find(mWhiteList.begin(), mWhiteList.end(), aMAC) != mWhiteList.end()) {
            lReturn = true;
        }
    }

    return lReturn;
}

bool MACBlackList::IsMACBlackListed(uint64_t aMAC) const
{
    bool lReturn{false};

    if (std::find(mBlackList.begin(), mBlackList.end(), aMAC) != mBlackList.end()) {
        lReturn = true;
    }

    return lReturn;
}

void MACBlackList::SetMACBlackList(std::vector<uint64_t>& aBlackList)
{
    mBlackList = std::move(aBlackList);
}

void MACBlackList::SetMACWhiteList(std::vector<uint64_t>& aWhiteList)
{
    mWhiteList = std::move(aWhiteList);
}