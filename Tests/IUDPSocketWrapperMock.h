/* Copyright (c) 2022 [Rick de Bondt] - IUDPSocketMock.cpp
 * This file contains a mock for IUDPSocket.
 **/

#include <string_view>

#include <gmock/gmock.h>

#include "IUDPSocketWrapper.h"

class IUDPSocketWrapperMock : public IUDPSocketWrapper
{
public:
    MOCK_METHOD(void, Close, ());
    MOCK_METHOD(bool, Open, (std::string_view aIp, unsigned int aPort));
    MOCK_METHOD(bool, IsOpen, ());
    MOCK_METHOD(std::size_t, SendTo, (std::string_view aData));
    MOCK_METHOD(std::size_t, ReceiveFrom, (char* aDataBuffer, size_t aDataBufferSize));
    MOCK_METHOD(void,
                AsyncReceiveFrom,
                (char* aDataBuffer, size_t aDataBufferSize, std::function<void(size_t)> aCallBack));
    MOCK_METHOD(void, StartThread, ());
    MOCK_METHOD(void, StopThread, ());
    MOCK_METHOD(bool, IsThreadStopped, ());
    MOCK_METHOD(void, PollThread, ());
};
