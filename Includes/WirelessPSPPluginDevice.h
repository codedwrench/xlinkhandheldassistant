#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - WirelessPSPPluginDevice.h
 *
 * This file contains functions to capture data from a wireless device in monitor mode.
 *
 * */

#include <chrono>
#include <memory>
#include <thread>

#include "Handler80211.h"
#include "IConnector.h"
#include "IWifiInterface.h"
#include "PCapDeviceBase.h"
#include "PCapWrapper.h"

#if defined(_WIN32) || defined(_WIN64)
#include "WifiInterfaceWindows.h"
#else
#include "WifiInterfaceLinuxBSD.h"
#endif

namespace WirelessPSPPluginDevice_Constants
{
    static constexpr unsigned int         cSnapshotLength{65535};
    static constexpr unsigned int         cPCAPTimeoutMs{1};
    static constexpr std::chrono::seconds cReconnectionTimeOut{15};
}  // namespace WirelessPSPPluginDevice_Constants

using namespace WirelessPSPPluginDevice_Constants;

/**
 * Class which allows a wireless device in monitor mode to capture data and send wireless frames.
 */
class WirelessPSPPluginDevice : public PCapDeviceBase
{
public:
    explicit WirelessPSPPluginDevice(
        bool                          aAutoConnect         = false,
        std::chrono::seconds          aReConnectionTimeOut = WirelessPSPPluginDevice_Constants::cReconnectionTimeOut,
        std::string*                  aCurrentlyConnected  = nullptr,
        std::shared_ptr<IPCapWrapper> aPcapWrapper         = std::make_shared<PCapWrapper>());

    void BlackList(uint64_t aMAC) override;

    void Close() override;

    /**
     * Connects to PSP AdHoc network.
     * @return true if successful.
     */
    bool Connect();

    bool Connect(std::string_view aESSID) override;

    bool Open(std::string_view aName, std::vector<std::string>& aSSIDFilter) override;
    /**
     * Opens the device so it can be used for capture.
     * @param aName - Name of the interface or file to use.
     * @param aSSIDFilter - The SSIDS to listen to.
     * @param aInterface - The WifiInterface to use for connection/finding out the MAC address.
     * @return true if successful.
     */
    bool Open(std::string_view                aName,
              std::vector<std::string>&       aSSIDFilter,
              std::shared_ptr<IWifiInterface> aInterface);

    // Public for easier testing
    bool ReadCallback(const unsigned char* aData, const pcap_pkthdr* aHeader);

    bool Send(std::string_view aData, bool aModifyData);

    bool Send(std::string_view aData) override;
    bool StartReceiverThread() override;

private:
    MACBlackList                    mBlackList{};
    bool                            mConnected{false};
    bool                            mSSIDFromHost{false};
    std::shared_ptr<IPCapWrapper>   mWrapper{nullptr};
    uint64_t                        mAdapterMACAddress{};
    bool                            mAutoConnect{};
    std::string*                    mCurrentlyConnected{nullptr};
    IWifiInterface::WifiInformation mCurrentlyConnectedInfo{};
    bool                            mPausedAutoConnect{false};
    std::shared_ptr<std::thread>    mReceiverThread{nullptr};
    bool                            mSendReceivedData{false};
    std::vector<std::string>        mSSIDFilter{};
    std::chrono::seconds            mReConnectionTimeOut{WirelessPSPPluginDevice_Constants::cReconnectionTimeOut};
    std::shared_ptr<IWifiInterface> mWifiInterface{nullptr};
    std::shared_ptr<std::thread>    mWifiTimeoutThread{nullptr};
    /**
     * This timer checks if any data has been received from the connected to network, if not it will try to reconnect.
     */
    std::chrono::time_point<std::chrono::system_clock> mReadWatchdog{std::chrono::seconds(0)};
};
