/* Copyright (c) 2020 [Rick de Bondt] - HUDController.cpp **/

#include "UserInterface/HUDController.h"

#include "UserInterface/AboutWindow.h"
#include "UserInterface/HUDWindow.h"
#include "UserInterface/OptionsWindow.h"
#include "UserInterface/ThemeWindow.h"

Window::Dimensions ScaleHUD(const int& aHeight, const int& aWidth)
{
    Window::Dimensions lDimensions{};

    lDimensions.at(2) = aHeight;
    lDimensions.at(3) = aWidth;

    return lDimensions;
}

// TODO: put in ControllerElements file, or maybe even put in baseclass?
template<class WindowType> void ReplaceWindow(std::vector<std::shared_ptr<IWindow>>& aWindows,
                                              WindowModel&                           aModel,
                                              std::string_view                       aTitle,
                                              std::function<Window::Dimensions()>    aDimensions)
{
    if (!aWindows.empty()) {
        aWindows.pop_back();
    }

    aWindows.emplace_back(std::make_shared<WindowType>(aModel, aTitle, aDimensions));

    // Run setup on the window,
    aWindows.back()->SetUp();
}

HUDController::HUDController(WindowModel& aWindowModel) : WindowControllerBase(aWindowModel) {}

bool HUDController::SetUp()
{
    WindowControllerBase::SetUp();

    // Add the HUD
    ReplaceWindow<HUDWindow>(
        GetWindows(), GetWindowModel(), "HUD", [&] { return ScaleHUD(GetHeightReference(), GetWidthReference()); });

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
        ReplaceWindow<OptionsWindow>(GetWindows(), GetWindowModel(), "Options", [&] {
            return ScaleHUD(GetHeightReference(), GetWidthReference());
        });
    }

    if (GetWindowModel().mAboutSelected) {
        GetWindowModel().mAboutSelected = false;
        ReplaceWindow<AboutWindow>(GetWindows(), GetWindowModel(), "About", [&] {
            return ScaleHUD(GetHeightReference(), GetWidthReference());
        });
    }

    if (GetWindowModel().mThemeSelected) {
        GetWindowModel().mThemeSelected = false;
        ReplaceWindow<ThemeWindow>(GetWindows(), GetWindowModel(), "Theme", [&] {
            return ScaleHUD(GetHeightReference(), GetWidthReference());
        });
    }

    if (GetWindowModel().mWindowDone) {
        GetWindowModel().mWindowDone = false;

        // Save any changes that may have been made
        GetWindowModel().SaveToFile(GetWindowModel().mProgramPath + "config.txt");

        ReplaceWindow<HUDWindow>(
            GetWindows(), GetWindowModel(), "HUD", [&] { return ScaleHUD(GetHeightReference(), GetWidthReference()); });
    }

    if (GetWindowModel().mWizardSelected) {
        // Let the main windowcontroller handle this
        lReturn = false;
    }

    return lReturn;
}
