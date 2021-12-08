#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - WifiInterfaceLinuxApple.h
 *
 * This file contains Apple specific functions for managing WiFi adapters.
 *
 **/

#include <objc/objc-runtime.h>

#include "IWifiInterface.h"


#ifdef __OBJC__
    @class WifiInterfaceAppleImplementation;
#else
    typedef struct objc_object WifiInterfaceAppleImplementation;
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
    ~WifiInterface();

    // just placeholders for now
    bool                                          Connect(const IWifiInterface::WifiInformation& aConnection) override;
    bool                                          LeaveIBSS() override;
    uint64_t                                      GetAdapterMacAddress() override;
    std::vector<IWifiInterface::WifiInformation>& GetAdhocNetworks() override;

private:
    // Objective-C++ wrapper using PIMPL-idiom
    WifiInterfaceAppleImplementation* mImplementation;
    std::string mAdapterName;
};
