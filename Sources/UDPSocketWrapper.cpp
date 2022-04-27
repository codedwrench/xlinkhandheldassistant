/* Copyright (c) 2022 [Rick de Bondt] - UDPSocketWrapper.cpp */

#include "UDPSocketWrapper.h"

#include "Logger.h"


bool UDPSocketWrapper::IsOpen()
{
    return mSocket.is_open();
}

bool UDPSocketWrapper::Open(std::string_view aIp, unsigned int aPort)
{
    if (!mSocket.is_open()) {
        mEndpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(aIp.data()), aPort);
        try {
            mSocket.open(boost::asio::ip::udp::v4());
        } catch (const boost::system::system_error& lException) {
            Logger::GetInstance().Log("Failed to open socket: " + std::string(lException.what()), Logger::Level::ERROR);
            return false;
        }
    }

    return true;
}

size_t UDPSocketWrapper::SendTo(std::string_view aData)
{
    return mSocket.send_to(boost::asio::buffer(aData, aData.size()), mEndpoint);
}

size_t UDPSocketWrapper::ReceiveFrom(char *aDataBuffer, size_t aDataBufferSize)
{
    return mSocket.receive_from(boost::asio::buffer(aDataBuffer, aDataBufferSize), mEndpoint);
}

void UDPSocketWrapper::AsyncReceiveFrom(char *aDataBuffer, size_t aDataBufferSize, 
std::function<void (const int &, size_t)> &aCallBack) 
{
    return mSocket.async_receive_from(boost::asio::buffer(aDataBuffer, aDataBufferSize), mEndpoint, aCallBack);
}

bool UDPSocketWrapper::IsThreadStopped()
{
    return mThread.stopped();
}

void UDPSocketWrapper::StartThread()
{
    mThread.restart();
}

void UDPSocketWrapper::StopThread()
{
    if(!mThread.stopped())
    {
        mThread.stop();
    }
}