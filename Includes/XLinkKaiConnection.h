#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - XLinkKaiConnection.h
 *
 * This file contains functions to talk to XLink Kai.
 *
 **/

#include <memory>
#include <string>
#include <thread>

#include "Handler8023.h"
#include "IConnector.h"
#include "ITimer.h"
#include "IUDPSocketWrapper.h"

namespace XLinkKai_Constants
{
    static constexpr int                  cMaxLength{4096};
    static constexpr std::string_view     cIp{"127.0.0.1"};
    static constexpr std::string_view     cSeparator{";"};
    static constexpr std::string_view     cInfoFormat{"info"};
    static constexpr std::string_view     cSubCommandTitleIdFormat{"titleid"};
    static constexpr std::string_view     cSubCommandSetESSIDFormat{"essid"};
    static constexpr std::string_view     cKeepAliveFormat{"keepalive"};
    static constexpr std::string_view     cConnectFormat{"connect"};
    static constexpr std::string_view     cConnectedFormat{"connected"};
    static constexpr std::string_view     cDisconnectFormat{"disconnect"};
    static constexpr std::string_view     cDisconnectedFormat{"disconnected"};
    static constexpr std::string_view     cSetESSIDFormat{"setessid"};
    static constexpr std::string_view     cEthernetDataFormat{"e"};
    static constexpr std::string_view     cEthernetDataMetaFormat{"d"};
    static constexpr std::string_view     cSettingFormat{"setting"};
    static constexpr std::string_view     cSettingDDSOnly{"ddsonly"};
    static constexpr std::string_view     cLocallyUniqueName{"XLHA_Device"};
    static constexpr std::string_view     cEmulatorName{"XLHA"};
    static constexpr unsigned int         cPort{34523};
    static constexpr std::chrono::seconds cConnectionTimeout{10};
    static constexpr std::chrono::seconds cKeepAliveTimeout{60};

    static const std::string cConnectString{std::string(cConnectFormat) + cSeparator.data() +
                                            cLocallyUniqueName.data() + cSeparator.data() + cEmulatorName.data() +
                                            cSeparator.data()};

    static const std::string cConnectedString{std::string(cConnectedFormat) + cSeparator.data() +
                                              cLocallyUniqueName.data()};

    static const std::string cDisconnectedString{std::string(cDisconnectedFormat) + cSeparator.data() +
                                                 cLocallyUniqueName.data()};

    static const std::string cDisconnectString{std::string(cDisconnectFormat) + cSeparator.data()};

    static const std::string cKeepAliveString{std::string(cKeepAliveFormat) + cSeparator.data()};

    static const std::string cEthernetDataString{std::string(cEthernetDataFormat) + cSeparator.data() +
                                                 cEthernetDataFormat.data() + cSeparator.data()};

    static const std::string cEthernetDataMetaString{std::string(cEthernetDataFormat) + cSeparator.data() +
                                                     cEthernetDataMetaFormat.data() + cSeparator.data()};

    static const std::string cSettingDDSOnlyString{std::string(cSettingFormat) + cSeparator.data() +
                                                   cSettingDDSOnly.data() + cSeparator.data() + "true" +
                                                   cSeparator.data()};

    static const std::string cInfoSetTitleIdString{std::string(cInfoFormat) + cSeparator.data() +
                                                   cSubCommandTitleIdFormat.data() + cSeparator.data()};

    static const std::string cInfoSetESSIDString(std::string(cInfoFormat) + cSeparator.data() +
                                                 cSubCommandSetESSIDFormat.data() + cSeparator.data());

    static const std::string cSetESSIDString(std::string(cEthernetDataMetaString.data()) + cSetESSIDFormat.data() +
                                             cSeparator.data());
}  // namespace XLinkKai_Constants

using namespace XLinkKai_Constants;

/**
 * Class that connects to XLink Kai and sends and receives data from and to XLink Kai.
 */
class XLinkKaiConnection : public IConnector
{
public:
    /**
     * Creates an XLink Kai Connection.
     *
     * @param aSocketWrapper - The implementation to use for UDP sockets.
     * @param aConnectionTimer - The implementation to use for the connection timer.
     * @param aKeepAliveTimer - The implementation to use for the keepalive timer.
     */
    explicit XLinkKaiConnection(std::shared_ptr<IUDPSocketWrapper> aSocketWrapper   = nullptr,
                                std::shared_ptr<ITimer>            aConnectionTimer = nullptr,
                                std::shared_ptr<ITimer>            aKeepAliveTimer  = nullptr);
    ~XLinkKaiConnection();
    XLinkKaiConnection(const XLinkKaiConnection& aXLinkKaiConnection) = delete;
    XLinkKaiConnection& operator=(const XLinkKaiConnection& aXLinkKaiConnection) = delete;

    bool Open(std::string_view aArgument) override;

    /**
     * Creates a connection.
     * @param aIp - IP Address of the XLink Kai engine.
     * @param aPort - Port of the XLink Kai engine.
     * @return True if successful.
     */
    bool Open(std::string_view aIp, unsigned int aPort);

    /**
     * Connects to XLink Kai.
     * @return True if successful.
     */
    bool Connect();

    /*
     * Sends a titleId over to the connector, it's up to the connector what to do with it.
     * @param aTitleId - The ID to send over to the connector.
     */
    void SendTitleId(std::string_view aTitleId) override;

    /*
     * Sends an ESSID over to the connector, it's up to the connector what to do with it.
     * @param aESSID - The ESSID to send over to the connector.
     */
    void SendESSID(std::string_view aESSID) override;

    /**
     * Synchronous receive of network messages from XLink Kai, may hang if nothing received!.
     * @return True if successful.
     */
    bool ReadNextData() override;

    bool StartReceiverThread() override;

    bool Send(std::string_view aCommand, std::string_view aData) override;

    bool Send(std::string_view aData) override;

    void Close() final;

    /**
     * Closes the connection.
     * @param aKillThread - set true if the receiver thread needs to be killed as well.
     */
    void Close(bool aKillThread);

    /**
     * Whether we are hosting a game or not.
     * @param aHosting - Set to true if hosting.
     */
    void SetHosting(bool aHosting);

    /**
     * Whether or not the SSID from the host should be used in the rest of the program.
     * @param aUseHostSSID - Set to true if host SSID should be used.
     */
    void SetUseHostSSID(bool aUseHostSSID);

    /**
     * Sets port to XLink Kai interface.
     * @param aPort - Port to connect to.
     */
    void SetPort(unsigned int aPort);

    void SetIncomingConnection(std::shared_ptr<IPCapDevice> aDevice) override;

private:
    /**
     * Handles traffic from XLink Kai.
     */
    void ReceiveCallback(size_t aBytesReceived);

    /**
     * Sends a keepalive back to the XLink Kai engine, call this function when a keepalive is received.
     * @return True if all bytes have been sent over successfully.
     */
    bool HandleKeepAlive();

    bool                    mStopCommand{false};
    bool                    mConnected{false};
    bool                    mConnectInitiated{false};
    bool                    mSettingsSent{false};
    std::shared_ptr<ITimer> mConnectionTimer{nullptr};
    std::shared_ptr<ITimer> mKeepAliveTimer{nullptr};

    std::array<char, cMaxLength> mData{};
    // Raw ethernet data received from XLink Kai
    std::string                        mEthernetData{};
    std::string                        mLastESSID{};
    std::string                        mLastTitleId{};
    std::shared_ptr<IPCapDevice>       mIncomingConnection{nullptr};
    std::string                        mIp{cIp};
    Handler8023                        mPacketHandler{};
    unsigned int                       mPort{cPort};
    bool                               mHosting{};
    bool                               mUseHostSSID{};
    std::shared_ptr<std::thread>       mReceiverThread{nullptr};
    std::shared_ptr<IUDPSocketWrapper> mSocketWrapper{nullptr};
};
