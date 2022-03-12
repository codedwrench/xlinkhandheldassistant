
#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - WizardSelectorStep.h
 *
 * This file contains an class for a userinterface selector window.
 *
 **/

#include "../Window.h"

/**
 * Class that will setup and draw a wizard selector window.
 **/
class WizardSelectorStep : public Window
{
public:
    WizardSelectorStep(WindowModel& aModel, std::string_view aTitle, std::function<Window::Dimensions()> aCalculation);

    void SetUp() final;
    void Draw() override;
};