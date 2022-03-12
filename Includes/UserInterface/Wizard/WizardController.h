#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - WizardController.h
 *
 * This file contains an class for Wizard controller.
 *
 **/

#include "../WindowControllerBase.h"

namespace WizardController_Constants
{
    enum WizardStep
    {
        Selector = 0,
        MonitorOptions,
        PromiscuousOptions,
        PluginOptions,
        SimulationOptions,
        USBOptions,
        XLinkKaiOptions
    };
}  // namespace WizardController_Constants

class WizardController : public WindowControllerBase
{
public:
    explicit WizardController(WindowModel& aWindowModel);

    bool SetUp() override;
    bool Process() override;

private:
    void                                   HandleConnectionMethod();
    WizardController_Constants::WizardStep mWizardStep{WizardController_Constants::Selector};
};
