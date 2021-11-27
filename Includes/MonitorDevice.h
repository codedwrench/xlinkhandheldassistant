#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - MonitorDevice.h
 *
 * This file contains functions to capture data from a wireless device in monitor mode.
 *
 **/

#include <memory>
#include <thread>

#include "Handler80211.h"
#include "IConnector.h"
#include "PCapDeviceBase.h"
#include "PCapWrapper.h"

namespace WirelessMonitorDevice_Constants
{
    static constexpr unsigned int cSnapshotLength{65535};
    static constexpr unsigned int cTimeout{1};
}  // namespace WirelessMonitorDevice_Constants

using namespace WirelessMonitorDevice_Constants;

/**
 * Class which allows a wireless device in monitor mode to capture data and send wireless frames.
 */
class MonitorDevice : public PCapDeviceBase
{
public:
    /**
     * Constructs the monitor device
     * @param aSourceMacToFilter - If a specific Mac needs to be filtered out.
     * @param aAcknowledgeDataFrames - If Data frames should be acknowledged.
     * @param aCurrentlyConnectedNetwork - For use in the user interface, shows SSID.
     * @param aPcapWrapper - The libpcap wrapper to use, allows for mocks to be shoved in.
     */
    MonitorDevice(uint64_t                      aSourceMacToFilter         = std::uint64_t(0),
                  bool                          aAcknowledgeDataFrames     = false,
                  std::string*                  aCurrentlyConnectedNetwork = nullptr,
                  std::shared_ptr<IPCapWrapper> aPcapWrapper               = std::make_shared<PCapWrapper>());

    void BlackList(uint64_t aMac) override;
    void Close() override;
    bool Connect(std::string_view aESSID) override;

    /**
     * Gets parameters for a data packet type, for example used for conversion to an 80211 packet.
     * @return a reference to PhysicalDeviceParameters object with the needed parameters.
     */
    const RadioTapReader::PhysicalDeviceParameters& GetDataPacketParameters();

    /**
     * Gets locked onto BSSID, this is the BSSID found when searching for beacon frames with the filtered SSID.
     * @return Locked onto BSSID/
     */
    uint64_t GetLockedBSSID();

    bool Open(std::string_view aName, std::vector<std::string>& aSSIDFilter) override;
    bool Send(std::string_view aData) override;
    void SetAcknowledgePackets(bool aAcknowledge);
    void SetSourceMacToFilter(uint64_t aMac);
    bool StartReceiverThread() override;

private:
    bool ReadCallback(const unsigned char* aData, const pcap_pkthdr* aHeader);

    bool                          mAcknowledgePackets{false};
    bool                          mConnected{false};
    std::string*                  mCurrentlyConnectedNetwork{nullptr};
    std::shared_ptr<IPCapWrapper> mPcapWrapper;
    Handler80211                  mPacketHandler{PhysicalDeviceHeaderType::RadioTap};
    std::shared_ptr<std::thread>  mReceiverThread{nullptr};
    bool                          mSendReceivedData{false};
};
