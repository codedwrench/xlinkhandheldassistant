/* Copyright (c) 2020 [Rick de Bondt] - HUDController.cpp **/

#include "../Includes/UserInterface/HUDController.h"

#include <iostream>

#include "../Includes/UserInterface/AboutWindow.h"
#include "../Includes/UserInterface/HUDWindow.h"
#include "../Includes/UserInterface/OptionsWindow.h"

Dimensions ScaleHUD()
{
    Dimensions lDimensions{};
    getmaxyx(stdscr, lDimensions.at(2), lDimensions.at(3));

    return lDimensions;
}

// TODO: put in ControllerElements file, or maybe even put in baseclass?
template<class WindowType>
void ReplaceWindow(std::vector<std::shared_ptr<IWindow>>& aWindows, WindowModel& aModel, std::string_view aTitle)
{
    if (!aWindows.empty()) {
        aWindows.pop_back();
    }

    aWindows.emplace_back(std::make_shared<WindowType>(aModel, aTitle, ScaleHUD));

    // Run setup on the window,
    aWindows.back()->SetUp();
}

HUDController::HUDController(WindowModel& aWindowModel) : WindowControllerBase(aWindowModel) {}

bool HUDController::SetUp()
{
    // Add the HUD
    ReplaceWindow<HUDWindow>(GetWindows(), GetWindowModel(), "HUD");
    return true;
}

bool HUDController::KeyAction(unsigned int aAction)
{
    bool lReturn{};
    lReturn = WindowControllerBase::KeyAction(aAction);

    return lReturn;
}

bool HUDController::Process()
{
    bool lReturn{false};

    lReturn = WindowControllerBase::Process();

    if (GetWindowModel().mOptionsSelected) {
        GetWindowModel().mOptionsSelected = false;
        ReplaceWindow<OptionsWindow>(GetWindows(), GetWindowModel(), "Options");
    }

    if (GetWindowModel().mAboutSelected) {
        GetWindowModel().mAboutSelected = false;
        ReplaceWindow<AboutWindow>(GetWindows(), GetWindowModel(), "About");
    }

    if (GetWindowModel().mWindowDone) {
        GetWindowModel().mWindowDone = false;
        ReplaceWindow<HUDWindow>(GetWindows(), GetWindowModel(), "HUD");
    }

    if (GetWindowModel().mWizardSelected) {
        // Let the main windowcontroller handle this
        lReturn = false;
    }

    return lReturn;
}
