/* Copyright (c) 2022 [Rick de Bondt] - Timer.cpp */

#include "Timer.h"

#include <chrono>

bool Timer::Start()
{
    if (mTimeOut > std::chrono::milliseconds(0)) {
        mStartTime = std::chrono::system_clock::now();
        mStarted   = true;
        return true;
    }

    return false;
}

bool Timer::Start(std::chrono::milliseconds aTimeout)
{
    mTimeOut = aTimeout;
    return Start();
}

void Timer::Stop()
{
    mStarted = false;
}

bool Timer::IsTimedOut()
{
    if (mStarted && (std::chrono::system_clock::now() - mTimeOut) > mStartTime) {
        mStarted = false;
        return true;
    }
    return false;
}

std::chrono::milliseconds Timer::GetTimeLeft()
{
    if (mStarted) {
        auto lDelta = std::chrono::system_clock::now() - mTimeOut - mStartTime;
        return (lDelta > std::chrono::milliseconds(0)) ? std::chrono::duration_cast<std::chrono::milliseconds>(lDelta) :
                                                         std::chrono::milliseconds(0);
    }

    return std::chrono::milliseconds(0);
}