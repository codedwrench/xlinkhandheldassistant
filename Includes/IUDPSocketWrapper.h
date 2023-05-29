#pragma once

/* Copyright (c) 2022 [Rick de Bondt] - IUDPSocketWrapper.h
 *
 * This file contains an interface for UDP socket wrapper.
 *
 **/

/**
 * Interface for UDP socket wrapper.
 */

#include <array>
#include <cstddef>
#include <functional>
#include <string_view>

class IUDPSocketWrapper
{
public:
    /**
     * Closes the UDP socket.
     */
    virtual void Close() = 0;

    /**
     * Opens the UDP socket.
     *
     * @param aIp - Ip address to use.
     * @param aPort - Port to use.
     *
     * @return true if successful.
     */
    virtual bool Open(std::string_view aIp, unsigned int aPort) = 0;

    /**
     * Checks if the UDP socket is open.
     *
     * @return true if socket is open.
     */
    virtual bool IsOpen() = 0;

    /**
     * Send data to socket.
     *
     * @param aData - Data to send.
     * @return The number of bytes sent.
     */
    virtual std::size_t SendTo(std::string_view aData) = 0;

    /**
     * Receives data from socket.
     *
     * @param aDataBuffer - The databuffer to put the result in.
     * @param aDataBufferSize  - The size of the data buffer.
     * @return Amount of bytes received.
     */
    virtual std::size_t ReceiveFrom(char* aDataBuffer, size_t aDataBufferSize) = 0;

    /**
     * Asynchronously receive data, function returns immediately, callback will be called when the data arrives.
     *
     * @param aDataBuffer - The databuffer to send the data to.
     * @param aDataBufferSize  - The size of the buffer to send the data to.
     * @param aCallBack - Callback to the function which will handle the callback.
     */
    virtual void AsyncReceiveFrom(char* aDataBuffer, size_t aDataBufferSize, std::function<void(size_t)> aCallBack) = 0;

    /**
     * Starts the thread for asynchronous data.
     */
    virtual void StartThread() = 0;

    /**
     * Stops the thread for asynchronous data.
     */
    virtual void StopThread() = 0;

    /**
     * Checks if the thread for asynchronous data is stopped.
     *
     * @return true if stopped.
     */
    virtual bool IsThreadStopped() = 0;

    /**
     * Polls the async thread
     */
    virtual void PollThread() = 0;
};