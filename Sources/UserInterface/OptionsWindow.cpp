/* Copyright (c) 2021 [Rick de Bondt] - OptionsWindow.cpp */

#include "../../Includes/UserInterface/OptionsWindow.h"

#include "../../Includes/UserInterface/Button.h"
#include "../../Includes/UserInterface/DefaultElements.h"
#include "../../Includes/UserInterface/RadioBoxGroup.h"

namespace
{
    Dimensions ScaleStartWizardButton() { return {2, 4, 0, 0}; }
    Dimensions ScaleAboutButton() { return {3, 4, 0, 0}; }
    Dimensions ScaleThemeButton() { return {4, 4, 0, 0}; }
    Dimensions ScaleLogLevelSelector() { return {6, 4, 0, 0}; }
    Dimensions ScaleDoneButton() { return {13, 4, 0, 0}; }
    Dimensions ScaleExitButton() { return {14, 4, 0, 0}; }
}  // namespace

OptionsWindow::OptionsWindow(WindowModel& aModel, std::string_view aTitle, std::function<Dimensions()> aCalculation) :
    Window(aModel, aTitle, aCalculation)
{}

void OptionsWindow::SetUp()
{
    Window::SetUp();

    // Get size of window so scaling works properly.
    GetSize();

    AddObject({std::make_shared<Button>(*this, "Reconfigure the application", ScaleStartWizardButton, [&] {
        GetModel().mWizardSelected = true;
        return true;
    })});

    AddObject({std::make_shared<Button>(*this, "Select a theme", ScaleThemeButton, [&] {
        GetModel().mThemeSelected = true;
        return true;
    })});

    AddObject({std::make_shared<Button>(*this, "About the application", ScaleAboutButton, [&] {
        GetModel().mAboutSelected = true;
        return true;
    })});

    auto lLogLevelSelector{std::make_shared<RadioBoxGroup>(
        *this, "Configure log level:", ScaleLogLevelSelector, reinterpret_cast<int&>(GetModel().mLogLevel))};

    for (std::string_view lLogLevel : Logger::cLevelTexts) {
        lLogLevelSelector->AddRadioBox(lLogLevel);
    }

    lLogLevelSelector->SetChecked(static_cast<int>(GetModel().mLogLevel));

    AddObject(lLogLevelSelector);

    AddObject({std::make_shared<Button>(*this, "Return to the HUD", ScaleDoneButton, [&] {
        GetModel().mWindowDone = true;
        return true;
    })});

    AddObject({std::make_shared<Button>(*this, "Exit the application", ScaleExitButton, [&] {
        GetModel().mStopProgram = true;
        return true;
    })});

    AddObject(CreateQuitText(*this, GetHeightReference()));
}

void OptionsWindow::Draw()
{
    Window::Draw();
}
