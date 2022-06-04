#pragma once

/* Copyright (c) 2022 [Rick de Bondt] - Timer.h
 *
 * This file contains a timer based on the system clock.
 *
 **/

/**
 * Timer based on system clock.
 */

#include <chrono>

#include "ITimer.h"

class Timer : public ITimer
{
public:
    virtual ~Timer() = default;
    bool                      Start(std::chrono::milliseconds aTimeOut) override;
    bool                      Start() override;
    void                      Stop() override;
    bool                      IsTimedOut() override;
    std::chrono::milliseconds GetTimeLeft() override;

private:
    std::chrono::time_point<std::chrono::system_clock> mStartTime{};
    std::chrono::milliseconds                          mTimeOut{};
    bool                                               mStarted{};
};