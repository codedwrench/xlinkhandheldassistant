/* Copyright (c) 2022 [Rick de Bondt] - ITimerMock.h
 * This file contains a mock for ITimer.
 **/

#include <chrono>
#include <gmock/gmock.h>

#include "ITimer.h"

class ITimerMock : public ITimer
{
public:
  MOCK_METHOD(bool, Start, (std::chrono::milliseconds aTimeout), ());
  MOCK_METHOD(bool, Start, (), ());
  MOCK_METHOD(void, Stop, (), ());
  MOCK_METHOD(bool, IsTimedOut, (), ());
  MOCK_METHOD(std::chrono::milliseconds, GetTimeLeft, (), ());
};
