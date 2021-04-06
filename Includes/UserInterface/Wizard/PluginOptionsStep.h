
#pragma once

#include "../Window.h"

/* Copyright (c) 2021 [Rick de Bondt] - PluginOptionsStep.h
 *
 * This file contains an class for a userinterface plugin options window.
 *
 **/

namespace PluginOptionsStep_Constants
{}  // namespace PluginOptionsStep_Constants

using namespace PluginOptionsStep_Constants;

/**
 * Class that will setup and draw a plugin options window.
 **/
class PluginOptionsStep : public Window
{
public:
    PluginOptionsStep(WindowModel& aModel, std::string_view aTitle, std::function<Dimensions()> aCalculation);

    virtual void SetUp() final;
    void         Draw() override;
};