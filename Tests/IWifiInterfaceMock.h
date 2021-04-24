/* Copyright (c) 2021 [Rick de Bondt] - IWifiInterfaceMock.cpp
 * This file contains a mock for IWifiInterface.
 **/

#include "../Includes/IWifiInterface.h"

class IWifiInterfaceMock : public IWifiInterface
{
public:
    MOCK_METHOD(bool, Connect, (const WifiInformation& aConnection));
    MOCK_METHOD(bool, LeaveIBSS, ());
    MOCK_METHOD(uint64_t, GetAdapterMACAddress, ());
    MOCK_METHOD(std::vector<WifiInformation>&, GetAdhocNetworks, ());
};
