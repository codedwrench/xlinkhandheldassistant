#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - WirelessPromiscuousBase.h
 *
 * This file contains functions to capture data from a wireless device in promiscuous mode.
 *
 **/

#include <chrono>
#include <memory>
#include <thread>

#include "HandlerPSPPlugin.h"
#include "IConnector.h"
#include "IWifiInterface.h"
#include "PCapDeviceBase.h"
#include "PCapWrapper.h"

#if defined(_WIN32) || defined(_WIN64)
#include "WifiInterfaceWindows.h"
#else
#include "WifiInterfaceLinuxBSD.h"
#endif

namespace WirelessPromiscuousBase_Constants
{
    static constexpr unsigned int         cSnapshotLength{65535};
    static constexpr unsigned int         cPCAPTimeoutMs{1};
    static constexpr std::chrono::seconds cReconnectionTimeOut{15};
}  // namespace WirelessPromiscuousBase_Constants

/**
 * Class which allows a wireless device in promiscuous mode (or using L2MOD) to capture data and send wireless frames.
 */
class WirelessPromiscuousBase : public PCapDeviceBase
{
public:
    explicit WirelessPromiscuousBase(bool                          aAutoConnect,
                                     std::chrono::seconds          aReConnectionTimeOut,
                                     std::string*                  aCurrentlyConnected,
                                     std::shared_ptr<IPCapWrapper> aPcapWrapper);

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
     * @param aInterface - The WifiInterface to use for connection/finding out the Mac address.
     * @return true if successful.
     */
    bool Open(std::string_view                aName,
              std::vector<std::string>&       aSSIDFilter,
              std::shared_ptr<IWifiInterface> aInterface);

    bool StartReceiverThread() override;

protected:
    uint64_t&                                           GetAdapterMacAddress();
    std::chrono::time_point<std::chrono::system_clock>& GetReadWatchdog();
    std::shared_ptr<IPCapWrapper>& GetWrapper();

private:
    bool                            mConnected{false};
    bool                            mSSIDFromHost{false};
    std::shared_ptr<IPCapWrapper>   mWrapper{nullptr};
    uint64_t                        mAdapterMacAddress{};
    bool                            mAutoConnect{};
    std::string*                    mCurrentlyConnected{nullptr};
    IWifiInterface::WifiInformation mCurrentlyConnectedInfo{};
    bool                            mPausedAutoConnect{false};
    std::shared_ptr<std::thread>    mReceiverThread{nullptr};
    bool                            mSendReceivedData{false};
    std::vector<std::string>        mSSIDFilter{};
    // Holds the ssids after a host ssid swap, so these can be restored
    std::vector<std::string>        mOldSSIDFilter{};
    std::chrono::seconds            mReConnectionTimeOut{WirelessPromiscuousBase_Constants::cReconnectionTimeOut};
    std::shared_ptr<IWifiInterface> mWifiInterface{nullptr};
    std::shared_ptr<std::thread>    mWifiTimeoutThread{nullptr};
    /**
     * This timer checks if any data has been received from the connected to network, if not it will try to reconnect.
     */
    std::chrono::time_point<std::chrono::system_clock> mReadWatchdog{std::chrono::seconds(0)};
};
