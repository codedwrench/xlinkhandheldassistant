/* Copyright (c) 2021 [Rick de Bondt] - MacBlackList.cpp */

#include "MacBlackList.h"

#include "Logger.h"
#include "NetConversionFunctions.h"

void MacBlackList::AddToMacBlackList(uint64_t aMac)
{
    if (IsMacAllowed(aMac)) {
        Logger::GetInstance().Log("Added: " + IntToMac(aMac) + " to blacklist.", Logger::Level::TRACE);
        mBlackList.push_back(aMac);
    }
}

void MacBlackList::AddToMacWhiteList(uint64_t aMac)
{
    Logger::GetInstance().Log("Added: " + IntToMac(aMac) + " to whitelist.", Logger::Level::TRACE);
    mWhiteList.push_back(aMac);
}

void MacBlackList::ClearMacBlackList()
{
    mBlackList.clear();
}

void MacBlackList::ClearMacWhiteList()
{
    mWhiteList.clear();
}

bool MacBlackList::IsMacAllowed(uint64_t aMac)
{
    bool lReturn{false};

    if (mWhiteList.empty()) {
        if (std::find(mBlackList.begin(), mBlackList.end(), aMac) == mBlackList.end()) {
            lReturn = true;
        }
    } else {
        if (std::find(mWhiteList.begin(), mWhiteList.end(), aMac) != mWhiteList.end()) {
            lReturn = true;
        }
    }

    return lReturn;
}

bool MacBlackList::IsMacBlackListed(uint64_t aMac) const
{
    bool lReturn{false};

    if (std::find(mBlackList.begin(), mBlackList.end(), aMac) != mBlackList.end()) {
        lReturn = true;
    }

    return lReturn;
}

void MacBlackList::SetMacBlackList(std::vector<uint64_t>& aBlackList)
{
    mBlackList = std::move(aBlackList);
}

void MacBlackList::SetMacWhiteList(std::vector<uint64_t>& aWhiteList)
{
    mWhiteList = std::move(aWhiteList);
}