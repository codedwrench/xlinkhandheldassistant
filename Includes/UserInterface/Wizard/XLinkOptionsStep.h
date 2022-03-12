#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - MonitorDeviceStep.h
 *
 * This file contains an class for a userinterface xlink window.
 *
 **/


#include "../Window.h"

/**
 * Class that will setup and draw an XLink window.
 **/
class XLinkOptionsStep : public Window
{
public:
    XLinkOptionsStep(WindowModel& aModel, std::string_view aTitle, std::function<Window::Dimensions()> aCalculation);

    void SetUp() override;
    void Draw() override;
};