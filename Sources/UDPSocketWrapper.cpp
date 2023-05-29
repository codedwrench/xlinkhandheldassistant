/* Copyright (c) 2022 [Rick de Bondt] - UDPSocketWrapper.cpp */

#include "UDPSocketWrapper.h"

#include <boost/bind/bind.hpp>
#include <boost/exception/exception.hpp>
#include <boost/system/error_code.hpp>

#include "Logger.h"

using namespace boost::placeholders;

bool UDPSocketWrapper::IsOpen()
{
    return mSocket.is_open();
}

void UDPSocketWrapper::Close()
{
    if (mSocket.is_open()) {
        mSocket.close();
    }
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

size_t UDPSocketWrapper::ReceiveFrom(char* aDataBuffer, size_t aDataBufferSize)
{
    return mSocket.receive_from(boost::asio::buffer(aDataBuffer, aDataBufferSize), mEndpoint);
}

void UDPSocketWrapper::ReceiveCallBack(const boost::system::error_code& /*aError*/, size_t aBufferSize)
{
    mCallBack(aBufferSize);
}

void UDPSocketWrapper::AsyncReceiveFrom(char*                       aDataBuffer,
                                        size_t                      aDataBufferSize,
                                        std::function<void(size_t)> aCallBack)
{
    mCallBack = std::move(aCallBack);
    return mSocket.async_receive_from(boost::asio::buffer(aDataBuffer, aDataBufferSize),
                                      mEndpoint,
                                      boost::bind(&UDPSocketWrapper::ReceiveCallBack,
                                                  this,
                                                  boost::asio::placeholders::error,
                                                  boost::asio::placeholders::bytes_transferred));
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
    if (!mThread.stopped()) {
        mThread.stop();
    }
}

void UDPSocketWrapper::PollThread()
{
    mThread.poll();
}