#pragma once

/* Copyright (c) 2022 [Rick de Bondt] - ITimer.h
 *
 * This file contains an interface for a timer.
 *
 **/

/**
 * Interface for a Timer.
 */

#include <chrono>

class ITimer
{
public:
    /**
     * Starts a timer.
     * @param aTimeOut - The timeout to set.
     * @return true if successful.
     */
    virtual bool Start(std::chrono::milliseconds aTimeOut) = 0;

    /**
     * Starts a timer based on the previous set timeout.
     * @return true if successful.
     */
    virtual bool Start() = 0;

    /**
     * Stops the timer, will reset the timed out flag.
     */
    virtual void Stop() = 0;

    /**
     * Checks if the timer has timed out.
     * Will reset the started flag if true is returned from this.
     *
     * @return true if timed out.
     */
    virtual bool IsTimedOut() = 0;

    /**
     * Gets the time that is left on the timer in milliseconds.
     *
     * @return The amount of milliseconds left on the timer.
     */
    virtual std::chrono::milliseconds GetTimeLeft() = 0;
};