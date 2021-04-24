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
#include "IPCapDevice.h"

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
class WirelessPSPPluginDevice : public IPCapDevice
{
public:
    WirelessPSPPluginDevice(
        bool                 aAutoConnect         = false,
        std::chrono::seconds aReConnectionTimeOut = WirelessPSPPluginDevice_Constants::cReconnectionTimeOut,
        std::string*         aCurrentlyConnected  = nullptr);
    void               BlackList(uint64_t aMAC) override;
    void               ClearMACBlackList();
    [[nodiscard]] bool IsMACBlackListed(uint64_t aMAC) const;

    void Close() override;

    /**
     * Connects to PSP AdHoc network.
     * @return true if successful.
     */
    bool ConnectToAdHoc();

    std::string          DataToString(const unsigned char* aData, const pcap_pkthdr* aHeader) override;
    const unsigned char* GetData() override;
    const pcap_pkthdr*   GetHeader() override;

    bool Open(std::string_view aName, std::vector<std::string>& aSSIDFilter) override;
    /**
     * Opens the device so it can be used for capture.
     * @param aName - Name of the interface or file to use.
     * @param aSSIDFilter - The SSIDS to listen to.
     * @param aInterface - The WifiInterface to use for connection/finding out the MAC address.
     * @return true if successful.
     */
    bool Open(std::string_view aName, std::vector<std::string>& aSSIDFilter, std::shared_ptr<IWifiInterface> aInterface);

    // Public for easier testing
    bool ReadCallback(const unsigned char* aData, const pcap_pkthdr* aHeader);

    // Allow this to be overridden
    virtual bool Send(std::string_view aData, bool aModifyData);

    bool Send(std::string_view aData) override;
    void SetConnector(std::shared_ptr<IConnector> aDevice) override;
    void SetHosting(bool aHosting) override;
    bool StartReceiverThread() override;

private:
    void ShowPacketStatistics(const pcap_pkthdr* aHeader) const;

    std::vector<uint64_t>           mBlackList{};
    bool                            mConnected{false};
    std::shared_ptr<IConnector>     mConnector{nullptr};
    const unsigned char*            mData{nullptr};
    pcap_t*                         mHandler{nullptr};
    bool                            mHosting{false};
    uint64_t                        mAdapterMACAddress{};
    bool                            mAutoConnect{};
    const pcap_pkthdr*              mHeader{nullptr};
    std::string*                    mCurrentlyConnected{nullptr};
    unsigned int                    mPacketCount{0};
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
