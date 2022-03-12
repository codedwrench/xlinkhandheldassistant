#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - MonitorDeviceStep.h
 *
 * This file contains an class for a userinterface monitor window.
 *
 **/

#include "../Window.h"

/**
 * Class that will setup and draw a monitor device step window.
 **/
class MonitorDeviceStep : public Window
{
public:
    MonitorDeviceStep(WindowModel& aModel, std::string_view aTitle, std::function<Window::Dimensions()> aCalculation);

    void SetUp() override;
    void Draw() override;
};