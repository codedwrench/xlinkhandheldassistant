/* Copyright (c) 2021 [Rick de Bondt] - WizardSelectorStep.cpp */

#include "../../../Includes/UserInterface/Wizard/WizardSelectorStep.h"

#include <cmath>
#include <utility>

#include "../../../Includes/UserInterface/Button.h"
#include "../../../Includes/UserInterface/DefaultElements.h"
#include "../../../Includes/UserInterface/RadioBoxGroup.h"
#include "../../../Includes/UserInterface/String.h"


namespace
{
    Dimensions ScaleSelectorBoxes() { return {2, 4, 0, 0}; }
}  // namespace

WizardSelectorStep::WizardSelectorStep(WindowModel&                aModel,
                                       std::string_view            aTitle,
                                       std::function<Dimensions()> aCalculation) :
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
#if not defined(_WIN32) && not defined(_WIN64)
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
