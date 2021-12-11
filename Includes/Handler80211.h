#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - Handler80211.h
 *
 * This file reads packets from a monitor format and converts to a promiscuous format.
 *
 **/

#include <memory>
#include <string>
#include <vector>

#include "IHandler.h"
#include "NetworkingHeaders.h"
#include "Parameter80211Reader.h"
#include "RadioTapReader.h"

/**
 * This class reads packets from a monitor format and converts to a promiscuous format.
 **/
class Handler80211 : public IHandler
{
public:
    /**
     * Constructs a handler object that converts packets from a wireless (radiotap + 802.11) format to an ethernet,
     * (802.3) format.
     * @param aType - Type of physical device header, usually radiotap is used, and for now this is the only supported
     * type.
     */
    explicit Handler80211(PhysicalDeviceHeaderType aType = PhysicalDeviceHeaderType::RadioTap);

    /**
     * This function converts a monitor mode packet to a promiscuous mode packet, stripping the radiotap and
     * 802.11 header and adding an 802.3 header. Only converts data packets!
     * @return converted packet data, empty string if failed.
     */
    std::string ConvertPacket();

    MACBlackList& GetBlackList() override;

    /**
     * Gets parameters for a control packet type, for example used for constructing acknowledgement frames.
     * @return a reference to PhysicalDeviceParameters object with the needed parameters.
     */
    const RadioTapReader::PhysicalDeviceParameters& GetControlPacketParameters();

    /**
     * Gets parameters for a data packet type, for example used for conversion to an 80211 packet.
     * @return a reference to PhysicalDeviceParameters object with the needed parameters.
     */
    const RadioTapReader::PhysicalDeviceParameters& GetDataPacketParameters();

    [[nodiscard]] uint64_t GetDestinationMAC() const override;
    [[nodiscard]] uint64_t GetSourceMAC() const override;

    /**
     * Gets locked onto BSSID.
     * @return the locked onto BSSID.
     */
    [[nodiscard]] uint64_t GetLockedBSSID() const;

    /**
     * Gets locked onto SSID.
     * @return the locked onto SSID.
     */
    [[nodiscard]] std::string GetLockedSSID() const;

    std::string_view GetPacket() override;

    /**
     * Checks if a packet is ackable.
     * @return true is packet is ackable.
     */
    [[nodiscard]] bool IsAckable() const;

    /**
     * Checks if the packet has been used by the handler.
     * @return true if unused.
     */
    [[nodiscard]] bool IsDropped() const;

    /**
     * Checks if this BSSID is locked onto.
     * @param aBSSID - BSSID to check
     * @return true if BSSID is locked onto.
     */
    [[nodiscard]] bool IsBSSIDAllowed(uint64_t aBSSID) const;

    /**
     * Sets a BSSID to listen to in case no beacon frames were sent out.
     * @param aBSSID - BSSID to listen to.
     */
    void SetBSSID(uint64_t aBSSID);

    /**
     * Checks if last received packet is convertible to 802.3 and should be sent.
     * @return true if should be sent.
     */
    [[nodiscard]] bool ShouldSend() const;

    /**
     * Checks if this SSID is whitelisted.
     * @param aSSID - SSID to check
     * @return true if SSID is whitelisted.
     */
    bool IsSSIDAllowed(std::string_view aSSID);

    /**
     *  Saves PhysicalDeviceParameters struct to reference.
     *  @param aParameters - reference to struct containing those parameters.
     */
    void SavePhysicalDeviceParameters(RadioTapReader::PhysicalDeviceParameters& aParameters);

    /**
     * Sets the SSID to filter on.
     */
    void SetSSIDFilterList(std::vector<std::string>& aSSIDList);

    void Update(std::string_view aPacket) override;

private:
    void UpdateBSSID();
    void UpdateControlPacketType();
    void UpdateDataPacketType();
    void UpdateMainPacketType();
    void UpdateManagementPacketType();
    void UpdateAckable();
    void UpdateRetry();
    void UpdateDestinationMac();
    void UpdateSourceMac();

    // Save last data in this class
    std::string mLastReceivedData{};

    std::vector<std::string> mSSIDList{};

    Main80211PacketType       mMainPacketType{Main80211PacketType::None};
    Control80211PacketType    mControlPacketType{Control80211PacketType::None};
    Data80211PacketType       mDataPacketType{Data80211PacketType::None};
    Management80211PacketType mManagementPacketType{Management80211PacketType::None};

    bool        mAckable{false};
    uint64_t    mBSSID{0};
    uint64_t    mDestinationMac{0};
    uint64_t    mLockedBSSID{0};
    std::string mLockedSSID{};
    bool        mRetry{false};
    bool        mShouldSend{false};
    uint64_t    mSourceMac{0};
    bool        mIsDropped{false};

    std::shared_ptr<Parameter80211Reader> mParameter80211Reader{nullptr};
    std::shared_ptr<RadioTapReader>       mPhysicalDeviceHeaderReader{nullptr};

    RadioTapReader::PhysicalDeviceParameters mPhysicalDeviceParametersControl{};
    RadioTapReader::PhysicalDeviceParameters mPhysicalDeviceParametersData{};
};
