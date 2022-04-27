#pragma once

/* Copyright (c) 2022 [Rick de Bondt] - UDPSocketWrapper.h
 *
 * This file contains wrapper functions for communication with a UDP socket.
 *
 **/

#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>

#include "IUDPSocketWrapper.h"

class UDPSocketWrapper : public IUDPSocketWrapper
{
    public:
        void Close() override;
        bool Open(std::string_view aIp, unsigned int aPort) override;
        bool IsOpen() override;
        std::size_t SendTo(std::string_view aData) override;
        std::size_t ReceiveFrom(char* aDataBuffer, size_t aDataBufferSize) override;
        void AsyncReceiveFrom(char* aDataBuffer, size_t aDataBufferSize, 
        std::function<void(size_t)> aCallBack) override; 
        void StartThread() override;
        void StopThread() override;
        bool IsThreadStopped() override;
        void PollThread() override;
    
    private:
        void ReceiveCallBack(const boost::system::error_code& aError, size_t aBufferSize);

        std::function<void(size_t)> mCallBack;

        boost::asio::io_service mThread{};
        boost::asio::ip::udp::endpoint mEndpoint{};
        boost::asio::ip::udp::socket mSocket{mThread};
};