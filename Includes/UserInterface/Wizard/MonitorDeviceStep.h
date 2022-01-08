#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - MonitorDeviceStep.h
 *
 * This file contains an class for a userinterface monitor window.
 *
 **/

#include "../Window.h"

namespace MonitorDeviceStep_Constants
{}  // namespace MonitorDeviceStep_Constants

using namespace MonitorDeviceStep_Constants;

/**
 * Class that will setup and draw a monitor device step window.
 **/
class MonitorDeviceStep : public Window
{
public:
    MonitorDeviceStep(WindowModel& aModel, std::string_view aTitle, std::function<Dimensions()> aCalculation);

    void SetUp() override;
    void Draw() override;
};