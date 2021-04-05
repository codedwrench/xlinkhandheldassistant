#pragma once

#include "../Window.h"

/* Copyright (c) 2020 [Rick de Bondt] - MonitorDeviceStep.h
 *
 * This file contains an class for a userinterface xlink window.
 *
 **/

namespace XLinkOptionsStep_Constants
{
    static constexpr std::string_view cAutoSearchXLinkInstancesMessage{"Automatically search for XLink Kai instances"};
    static constexpr std::string_view cIPAddressMessage{"XLink IP address"};
    static constexpr std::string_view cPortMessage{"XLink Port"};
}  // namespace XLinkOptionsStep_Constants

using namespace XLinkOptionsStep_Constants;

/**
 * Class that will setup and draw an XLink window.
 **/
class XLinkOptionsStep : public Window
{
public:
    XLinkOptionsStep(WindowModel& aModel, std::string_view aTitle, const std::function<Dimensions()>& aCalculation);

    void SetUp() override;
    void Draw() override;
};