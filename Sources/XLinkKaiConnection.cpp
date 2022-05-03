/* Copyright (c) 2020 [Rick de Bondt] - XLinkKaiConnection.cpp */

#include "XLinkKaiConnection.h"

#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <thread>

#include <boost/exception/diagnostic_information.hpp>

#include "IPCapDevice.h"
#include "Logger.h"
#include "MonitorDevice.h"
#include "NetConversionFunctions.h"
#include "UDPSocketWrapper.h"

using namespace std::chrono_literals;

XLinkKaiConnection::XLinkKaiConnection(std::shared_ptr<IUDPSocketWrapper> aSocketWrapper) :
    mSocketWrapper(aSocketWrapper)
{
    if (!mSocketWrapper) {
        mSocketWrapper = std::make_shared<UDPSocketWrapper>();
    }
}

XLinkKaiConnection::~XLinkKaiConnection()
{
    Close();
}

bool XLinkKaiConnection::Open(std::string_view aArgument)
{
    return Open(aArgument, cPort);
}

bool XLinkKaiConnection::Open(std::string_view aIp, unsigned int aPort)
{
    std::string  lIp{aIp};
    unsigned int lPort{aPort};

    // TODO: Do broadcast, but for now, use default stuff
    if (aIp.empty()) {
        lIp   = cIp;
        lPort = cPort;
    }

    return mSocketWrapper->Open(lIp, lPort);
}

bool XLinkKaiConnection::Connect()
{
    bool lReturn{true};

    if (Send(cConnectString, "")) {
        // Start the timer for receiving a confirmation from XLink Kai.
        mConnectInitiated = true;
        mConnectionTimerStart += (std::chrono::system_clock::now() - mConnectionTimerStart);
    } else {
        // Logging in send function
        lReturn = false;
    }
    return lReturn;
}

bool XLinkKaiConnection::Send(std::string_view aCommand, std::string_view aData)
{
    bool lReturn{true};

    // We only allow connection/disconnection requests to be sent, when XLink Kai has not confirmed the connection yet.
    if (mSocketWrapper->IsOpen()) {
        if ((mConnected || aCommand == cConnectString || aCommand == cDisconnectString)) {
            try {
                if (aCommand == cEthernetDataString) {
                    Logger::GetInstance().Log("Sent: " + std::string(aCommand) + PrettyHexString(aData),
                                              Logger::Level::TRACE);
                } else {
                    Logger::GetInstance().Log("Sent: " + std::string(aCommand) + std::string(aData),
                                              Logger::Level::DEBUG);
                }

                mSocketWrapper->SendTo(std::string(aCommand) + std::string(aData));
            } catch (const boost::system::system_error& lException) {
                Logger::GetInstance().Log(
                    "Could not send message! " + std::string(aData) + std::string(lException.what()),
                    Logger::Level::ERROR);
                lReturn = false;
            }
        } else {
            Logger::GetInstance().Log("No other messages before Xlink Kai has connected!", Logger::Level::DEBUG);
            lReturn = false;
        }
    } else {
        Logger::GetInstance().Log("Could not send message on closed socket.", Logger::Level::DEBUG);
        mConnected = false;
        lReturn    = false;
    }
    return lReturn;
}

bool XLinkKaiConnection::Send(std::string_view aData)
{
    return Send(cEthernetDataString, aData);
}

void XLinkKaiConnection::SendESSID(std::string_view aESSID)
{
    mLastESSID = aESSID;

    // Set this regardless of hosting, this is info for XLink Kai
    Send(std::string(XLinkKai_Constants::cInfoSetESSIDString),
         std::string(mLastESSID) + XLinkKai_Constants::cSeparator.data());

    if (mHosting) {
        Send(std::string(XLinkKai_Constants::cSetESSIDString), mLastESSID);
    }
}

void XLinkKaiConnection::SendTitleId(std::string_view aTitleId)
{
    mLastTitleId = aTitleId;

    Send(std::string(XLinkKai_Constants::cInfoSetTitleIdString),
         std::string(mLastTitleId) + XLinkKai_Constants::cSeparator.data());
}

bool XLinkKaiConnection::HandleKeepAlive()
{
    bool lReturn{true};
    if (!Send(cKeepAliveString, "")) {
        // Logging in send function.
        lReturn = false;
    }
    return lReturn;
}

bool XLinkKaiConnection::ReadNextData()
{
    bool lReturn{true};

    size_t lBytesReceived{mSocketWrapper->ReceiveFrom(mData.data(), cMaxLength)};

    if (lBytesReceived > 0) {
        ReceiveCallback(lBytesReceived);
    }

    return lReturn;
}

void XLinkKaiConnection::ReceiveCallback(size_t aBytesReceived)
{
    std::string lData{mData.begin(), mData.begin() + aBytesReceived};

    // If we actually received anything useful, react.
    if (!lData.empty()) {
        // Make sure the keepalive timer gets tickled so it doesn't bite.
        mKeepAliveTimerStart += (std::chrono::system_clock::now() - mKeepAliveTimerStart);
        std::size_t lFirstSeparator{lData.find(cSeparator)};
        std::string lCommand{lData.substr(0, lFirstSeparator + 1)};

        if (lCommand != std::string(cEthernetDataFormat) + cSeparator.data()) {
            Logger::GetInstance().Log("Received: " + lCommand + lData, Logger::Level::TRACE);
        }

        if (!mConnected && (lCommand == std::string(cConnectedFormat) + cSeparator.data())) {
            lCommand = lData.substr(0, cConnectedString.size());
            if (lCommand == cConnectedString) {
                Logger::GetInstance().Log("XLink Kai succesfully connected: " + lCommand, Logger::Level::INFO);
                mConnectInitiated = false;
                mConnected        = true;
            }
        }

        // If no connection confirmation has been sent on XLink Kai's side, Don't care about any other message yet
        if (mConnected) {
            if (lCommand == cKeepAliveString) {
                HandleKeepAlive();
            } else if (lCommand == std::string(cEthernetDataFormat) + cSeparator.data()) {
                // For data XLink Kai uses e;e; which doesn't filter all that well, so if we find e; just check if this
                // is e;e;
                lCommand = lData.substr(0, cEthernetDataString.size());

                Logger::GetInstance().Log("Received: " + PrettyHexString(lData.substr(cEthernetDataString.length())),
                                          Logger::Level::TRACE);

                if (lCommand == cEthernetDataString) {
                    if (mIncomingConnection != nullptr) {
                        // Strip e;e;
                        mEthernetData =
                            lData.substr(cEthernetDataString.length(), lData.length() - cEthernetDataString.length());

                        mPacketHandler.Update(mEthernetData);

                        std::shared_ptr<MonitorDevice> lMonitorDevice =
                            std::dynamic_pointer_cast<MonitorDevice>(mIncomingConnection);

                        // If it is actually a monitor device, do convert.
                        if (lMonitorDevice != nullptr) {
                            mEthernetData = mPacketHandler.ConvertPacketOut(lMonitorDevice->GetLockedBSSID(),
                                                                            lMonitorDevice->GetDataPacketParameters());
                        }

                        // Data from XLink Kai should never be caught in the receiver thread
                        mIncomingConnection->BlackList(mPacketHandler.GetSourceMac());
                        mIncomingConnection->Send(mEthernetData);
                    }
                } else if (lCommand == cEthernetDataMetaString) {
                    if (lData.substr(cEthernetDataMetaString.length(), cSetESSIDFormat.length()) == cSetESSIDFormat) {
                        Logger::GetInstance().Log(
                            "XLink Kai gave us the following ESSID: " + lData.substr(cSetESSIDString.length()),
                            Logger::Level::DEBUG);

                        if (!mHosting && mUseHostSSID) {
                            mIncomingConnection->Connect(lData.substr(cSetESSIDString.length()));
                        }
                    } else {
                        Logger::GetInstance().Log(std::string("Unrecognized e;d message from XLink Kai: ") + lData,
                                                  Logger::Level::DEBUG);
                    }
                }
            } else if (lCommand == std::string(cDisconnectedFormat) + cSeparator.data()) {
                lCommand = lData.substr(0, cDisconnectedString.size());
                if (lCommand == cDisconnectedString) {
                    Logger::GetInstance().Log("Xlink Kai has disconnected us! " + lCommand, Logger::Level::ERROR);
                    mConnected = false;
                }
            }
        }
    }
}

bool XLinkKaiConnection::StartReceiverThread()
{
    bool lReturn{true};

    if (mSocketWrapper->IsOpen()) {
        mSocketWrapper->AsyncReceiveFrom(
            mData.data(), cMaxLength, [&](size_t aBufferSize) { ReceiveCallback(aBufferSize); });

        // Run
        if (mReceiverThread == nullptr) {
            mReceiverThread = std::make_shared<std::thread>([&] {
                mSocketWrapper->StartThread();

                // Try to connect to XLink Kai for the first time before going into the while loop.
                Connect();

                while (!mSocketWrapper->IsThreadStopped()) {
                    if ((!mConnected && !mConnectInitiated)) {
                        Close(false);
                        Open(mIp, mPort);
                        Connect();
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                    } else if ((!mConnected) && mConnectInitiated &&
                               (std::chrono::system_clock::now() > (mConnectionTimerStart + cConnectionTimeout))) {
                        Logger::GetInstance().Log("Timeout waiting for XLink Kai to connect", Logger::Level::ERROR);
                        mConnectInitiated = false;
                        mConnected        = false;
                        mSettingsSent     = false;
                        // Retry in 10 seconds
                        std::this_thread::sleep_for(10s);
                    } else if (mConnected && !mConnectInitiated &&
                               (std::chrono::system_clock::now() > (mKeepAliveTimerStart + cKeepAliveTimeout))) {
                        // KaiEngine stopped sending keepalive messages, must've died.
                        Logger::GetInstance().Log("It seems KaiEngine has stopped responding, resetting connection ...",
                                                  Logger::Level::ERROR);
                        mConnected        = false;
                        mConnectInitiated = false;
                        mSettingsSent     = false;
                    } else if (mConnected && !mConnectInitiated && !mSettingsSent) {
                        Send(cSettingDDSOnlyString, "");

                        // Cache the title ID and ESSID because they get destroyed in the devices.
                        std::string lTitleId = mIncomingConnection->GetTitleId().empty() ?
                                                   mLastTitleId :
                                                   mIncomingConnection->GetTitleId();

                        std::string lESSID =
                            mIncomingConnection->GetESSID().empty() ? mLastESSID : mIncomingConnection->GetESSID();

                        if (!lTitleId.empty()) {
                            SendTitleId(lTitleId);
                        }

                        if (!lESSID.empty()) {
                            SendESSID(lESSID);
                        }

                        mSettingsSent = true;
                    } else {
                        mSocketWrapper->PollThread();
                        
                        // Make sure the thread doesn't stop after poll
                        mSocketWrapper->StartThread();
                        // Very small delay to make the computer happy
                        std::this_thread::sleep_for(100us);
                    }
                }
            });
        }
    } else {
        Logger::GetInstance().Log("Can't start receiving without an opened socket!", Logger::Level::ERROR);
        lReturn = false;
    }

    return lReturn;
}


void XLinkKaiConnection::Close()
{
    Close(true);
}

void XLinkKaiConnection::Close(bool aKillThread)
{
    try {
        if (mConnected || mConnectInitiated) {
            Send(cDisconnectString, "");
        }

        if (aKillThread && mReceiverThread != nullptr) {
            mSocketWrapper->StopThread();

            mReceiverThread->join();
            mReceiverThread = nullptr;
        }

        mConnected        = false;
        mConnectInitiated = false;

        mSocketWrapper->Close();

        // Settings need to be resent
        mSettingsSent = false;
    } catch (...) {
        std::cout << "Failed to disconnect :( " + boost::current_exception_diagnostic_information() << std::endl;
    }
}

void XLinkKaiConnection::SetHosting(bool aHosting)
{
    mHosting = aHosting;
}

void XLinkKaiConnection::SetUseHostSSID(bool aUseHostSSID)
{
    mUseHostSSID = aUseHostSSID;
}

void XLinkKaiConnection::SetPort(unsigned int aPort)
{
    mPort = aPort;
}

void XLinkKaiConnection::SetIncomingConnection(std::shared_ptr<IPCapDevice> aDevice)
{
    mIncomingConnection = aDevice;
}
