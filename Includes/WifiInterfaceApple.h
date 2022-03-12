#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - WifiInterfaceLinuxApple.h
 *
 * This file contains Apple specific functions for managing WiFi adapters.
 *
 **/

#include <chrono>

#include "IWifiInterface.h"

#include <objc/objc-runtime.h>


#ifdef __OBJC__
@class WifiInterfaceAppleImplementation;
#else
using WifiInterfaceAppleImplementation = struct objc_object;
#endif

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

    WifiInterface(WifiInterface const& aInterface)     = default;
    WifiInterface(WifiInterface&& aInterface) noexcept = default;

    WifiInterface& operator=(WifiInterface const& aInterface) = default;
    WifiInterface& operator=(WifiInterface&& aInterface) = default;

    ~WifiInterface();

    // just placeholders for now
    bool                                          Connect(const IWifiInterface::WifiInformation& aConnection) override;
    bool                                          LeaveIBSS() override;
    uint64_t                                      GetAdapterMacAddress() override;
    std::vector<IWifiInterface::WifiInformation>& GetAdhocNetworks() override;

private:
    // Objective-C++ wrapper using PIMPL-idiom
    WifiInterfaceAppleImplementation* mImplementation;
    std::string                       mAdapterName;
};
