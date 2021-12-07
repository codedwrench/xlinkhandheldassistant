/* Copyright (c) 2021 [Rick de Bondt] - WifiInterfaceApple.cpp */

#include "../Includes/WifiInterfaceApple.h"

using namespace WifiInterface_Constants;

WifiInterface::WifiInterface(std::string_view aAdapterName)
{
}

WifiInterface::~WifiInterface()
{
}

bool WifiInterface::Connect(const IWifiInterface::WifiInformation& aConnection)
{
    return false;
}

bool WifiInterface::LeaveIBSS()
{
    return false;
}

uint64_t WifiInterface::GetAdapterMacAddress()
{
    return 0x001234567890;
}

std::vector<IWifiInterface::WifiInformation>& WifiInterface::GetAdhocNetworks()
{
    return mAdhocNetworks;
}

