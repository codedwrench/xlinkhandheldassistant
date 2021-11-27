#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - WifiInterfaceLinuxBSD.h
 *
 * This file contains Linux and BSD specific functions for managing WiFi adapters.
 *
 **/

#if not defined(_WIN32) && not defined(_WIN64)
#include "IWifiInterface.h"

#include <array>
#include <chrono>
#include <mutex>
#include <string_view>
#include <utility>

#include <linux/nl80211.h>

#include <netlink/errno.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>
#include <netlink/netlink.h>

struct nl_sock;

/**
 * Class that manages WiFi adapters.
 */
namespace WifiInterface_Constants
{
    static constexpr std::string_view cDriverName{"nl80211"};
    static constexpr std::string_view cScanCommand{"scan"};
    static constexpr std::string_view cControlCommand{"nlctrl"};
    struct TriggerResults
    {
        int done;
        int aborted;
    } __attribute__((aligned(8)));

    struct HandlerArguments
    {  // For FamilyHandler() and nl_get_multicast_id().
        std::string group;
        int         id;
    } __attribute__((aligned(64))) __attribute__((packed));

    struct DumpResultArgument
    {
        std::array<nla_policy, NL80211_BSS_MAX + 1>&  bssserviceinfo;
        std::vector<IWifiInterface::WifiInformation>& adhocnetworks;
    } __attribute__((aligned(16)));

    static constexpr std::chrono::seconds cScanTimeout{30};

}  // namespace WifiInterface_Constants
class WifiInterface : public IWifiInterface
{
public:
    explicit WifiInterface(std::string_view aAdapterName);
    ~WifiInterface();

    bool                                          Connect(const IWifiInterface::WifiInformation& aConnection) override;
    bool                                          LeaveIBSS() override;
    uint64_t                                      GetAdapterMacAddress() override;
    std::vector<IWifiInterface::WifiInformation>& GetAdhocNetworks() override;

private:
    void ClearSocket();
    bool ScanTrigger();
    int  GetMulticastId();
    void SetBSSPolicy();

    std::string                                  mAdapterName{};
    std::array<nla_policy, NL80211_BSS_MAX + 1>  mBSSPolicy{};
    std::mutex                                   mLocked{};
    nl_sock*                                     mSocket{nullptr};
    int                                          mDriverId{0};
    unsigned int                                 mNetworkAdapterIndex{0};
    std::vector<IWifiInterface::WifiInformation> mLastReceivedScanInformation{};
};
#endif
