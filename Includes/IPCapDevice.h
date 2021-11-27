#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - IPCapDevice.h
 *
 * This file contains an interface for pcap devices, either file based or device based.
 *
 **/

#include <memory>
#include <string>
#include <vector>

#include "NetworkingHeaders.h"

namespace IPCapDevice_Constants
{
    struct WiFiBeaconInformation
    {
        uint64_t    BSSID{};
        std::string SSID{};
        uint8_t     MaxRate{RadioTap_Constants::cRateFlags};
        uint16_t    Frequency{RadioTap_Constants::cChannel};
    };
}  // namespace IPCapDevice_Constants

class IConnector;
class pcap_pkthdr;

/**
 * Interface for pcap devices, either file based or device based.
 */
class IPCapDevice
{
public:
    /**
     * Adds a Mac address to the blacklist.
     * @param aMac - Mac address to blacklist.
     */
    virtual void BlackList(uint64_t aMac) = 0;

    /**
     * Closes the PCAP device.
     */
    virtual void Close() = 0;

    /**
     * Connects to a network in whatever method is appropriate for the connection.
     */
    virtual bool Connect(std::string_view aESSID) = 0;

    /**
     * Opens the PCAP device so it can be used for capture.
     * @param aName - Name of the interface or file to use.
     * @todo SSIDFilter is monitor mode specific. Do not add here.
     * @param aSSIDFilter - The SSIDS to listen to.
     * @return true if successful.
     */
    virtual bool Open(std::string_view aName, std::vector<std::string>& aSSIDFilter) = 0;

    /**
     * Returns data as string.
     * @param aData - Data from pcap functions.
     * @param aHeader - Header from pcap functions.
     * @return Data as string.
     */
    virtual std::string DataToString(const unsigned char* aData, const pcap_pkthdr* aHeader) = 0;

    /**
     * Gets data from last read packet.
     * @return pointer to data as an unsigned char array.
     */
    virtual const unsigned char* GetData() = 0;

    /**
     * Gets header from last read packet.
     * @return pointer to header as pcap_pkthdr type.
     */
    virtual const pcap_pkthdr* GetHeader() = 0;

    /**
     * Sends data over device/file if supported.
     * @param aData - Data to send.
     * @return true if successful, false on failure or unsupported.
     */
    virtual bool Send(std::string_view aData) = 0;

    /**
     * Sets outgoing connection.
     * @param aDevice - Device to use as the outgoing connection.
     */
    virtual void SetConnector(std::shared_ptr<IConnector> aDevice) = 0;

    /**
     * Sets whether user is hosting or not.
     * @param aHosting - If the user is hosting.
     */
    virtual void SetHosting(bool aHosting) = 0;

    /**
     * Prints some fancy statistics about a packet.
     * @param aHeader - Header of the packet to show statistics of.
     */
    virtual void ShowPacketStatistics(const pcap_pkthdr* aHeader) const = 0;

    /**
     * Starts receiving on device.
     * @return true if successful (for a file based device this will start replaying the capture).
     */
    virtual bool StartReceiverThread() = 0;
};
