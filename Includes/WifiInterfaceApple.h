#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - WifiInterfaceLinuxApple.h
 *
 * This file contains Apple specific functions for managing WiFi adapters.
 *
 **/

#include "IWifiInterface.h"

/**
 * Class that manages WiFi adapters.
 */
namespace WifiInterface_Constants
{
    static constexpr std::chrono::seconds cScanTimeout{30};
}  // namespace WifiInterface_Constants
class WifiInterface : public IWifiInterface
{
public:
    explicit WifiInterface(std::string_view aAdapterName) {};
    ~WifiInterface() {};

    // just placeholders for now
    bool                                          Connect(const IWifiInterface::WifiInformation& aConnection) override { return false; };
    bool                                          LeaveIBSS() override { return false; }
    uint64_t                                      GetAdapterMacAddress() override { return 0x012345678900; }
    std::vector<IWifiInterface::WifiInformation>& GetAdhocNetworks() override { return mAdhocNetworks; }

private:
    std::vector<IWifiInterface::WifiInformation> mAdhocNetworks{};
};
