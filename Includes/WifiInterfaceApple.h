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
    explicit WifiInterface(std::string_view aAdapterName);
    ~WifiInterface();

    // just placeholders for now
    bool                                          Connect(const IWifiInterface::WifiInformation& aConnection) override;
    bool                                          LeaveIBSS() override;
    uint64_t                                      GetAdapterMacAddress() override;
    std::vector<IWifiInterface::WifiInformation>& GetAdhocNetworks() override;

private:
    std::vector<IWifiInterface::WifiInformation> mAdhocNetworks{};
};
