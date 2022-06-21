#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - IConnector.h
 *
 * This file contains an interface for outside connections, either file based or device based.
 *
 **/

#include <memory>
#include <string>

class IPCapDevice;

/**
 * Interface for outside connections.
 */
class IConnector
{
public:
    /**
     * Opens the device/file.
     * @param aArgument - File/Device/IP to use.
     * @return true if successful
     */
    virtual bool Open(std::string_view aArgument) = 0;

    /**
     * Closes the device/file.
     */
    virtual void Close() = 0;

    /**
     * Reads the next data from the device or file.
     * @return true on success.
     */
    virtual bool ReadNextData() = 0;

    /**
     * Sends data over device/file if supported.
     * @param aData - Data to send.
     * @return true if successful, false on failure or unsupported.
     */
    virtual bool Send(std::string_view aData) = 0;

    /**
     * Sends data over device/file if supported.
     * Also accepts commands in case the device supports those.
     * @param aCommand - Command to send.
     * @param aData - Data to send.
     * @return true if successful, false on failure or unsupported.
     */
    virtual bool Send(std::string_view aCommand, std::string_view aData) = 0;

    /*
     * Sends a titleId over to the connector, it's up to the connector what to do with it.
     * @param aTitleId - The ID to send over to the connector.
     */
    virtual void SendTitleId(std::string_view aTitleId) = 0;

    /*
     * Sends an ESSID over to the connector, it's up to the connector what to do with it.
     * @param aESSID - The ESSID to send over to the connector.
     */
    virtual void SendESSID(std::string_view aESSID) = 0;

    /**
     * Allows sending over different device.
     * @param aDevice - Device to use.
     */
    virtual void SetIncomingConnection(std::shared_ptr<IPCapDevice> aDevice) = 0;

    /**
     * Starts receiving network messages from Connector.
     * @return true if successful.
     */
    virtual bool StartReceiverThread() = 0;
};
