/* Copyright (c) 2021 [Rick de Bondt] - WizardSelectorStep.cpp */

#include "UserInterface/Wizard/WizardSelectorStep.h"

#include "UserInterface/Button.h"
#include "UserInterface/DefaultElements.h"
#include "UserInterface/RadioBoxGroup.h"

namespace
{
    Window::Dimensions ScaleSelectorBoxes()
    {
        return {2, 4, 0, 0};
    }
}  // namespace

WizardSelectorStep::WizardSelectorStep(WindowModel&                        aModel,
                                       std::string_view                    aTitle,
                                       std::function<Window::Dimensions()> aCalculation) :
    Window(aModel, aTitle, aCalculation)
{}

void WizardSelectorStep::SetUp()
{
    Window::SetUp();

    auto lSelector{std::make_shared<RadioBoxGroup>(*this,
                                                   "Choose your connection method:",
                                                   ScaleSelectorBoxes,
                                                   reinterpret_cast<int&>(GetModel().mConnectionMethod))};

    lSelector->AddRadioBox("Plugin Device");
    lSelector->AddRadioBox("Vita Device");
#if not defined(_WIN32) && not defined(_WIN64) && not defined(__APPLE__)
    lSelector->AddRadioBox("Monitor Device");
#endif

    // TODO: Add
    // lSelector->AddRadioBox("USB Device");
    // lSelector->AddRadioBox("Simulated Device");

    // Objects need to be added for them to be drawn
    AddObject(lSelector);
    AddObject(CreateNextButton(*this, GetModel(), GetHeightReference(), GetWidthReference()));
    AddObject(CreateWizardStepText(*this, 1, 3, GetWidthReference()));
    AddObject(CreateQuitText(*this, GetHeightReference()));
}

void WizardSelectorStep::Draw()
{
    Window::Draw();
}
