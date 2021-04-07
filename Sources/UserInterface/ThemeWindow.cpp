#include "../../Includes/UserInterface/ThemeWindow.h"

#include <cmath>
#include <filesystem>
#include <utility>

#include "../../Includes/UserInterface/Button.h"
#include "../../Includes/UserInterface/DefaultElements.h"
#include "../../Includes/UserInterface/RadioBoxGroup.h"

/* Copyright (c) 2021 [Rick de Bondt] - ThemeWindow.cpp */

namespace
{
    Dimensions ScaleThemeSelector() { return {2, 4, 0, 0}; }
    Dimensions ScaleDoneButton(const int& aMaxHeight, const int& aMaxWidth)
    {
        return {(aMaxHeight - 2), (aMaxWidth - 2) - static_cast<int>(std::string("[ Save selection ]").length()), 0, 0};
    }
}  // namespace

ThemeWindow::ThemeWindow(WindowModel& aModel, std::string_view aTitle, std::function<Dimensions()> aCalculation) :
    Window(aModel, aTitle, aCalculation)
{}

void ThemeWindow::SetUp()
{
    Window::SetUp();

    // Get size of window so scaling works properly.
    GetSize();

    std::string                    lPath = GetModel().mProgramPath + "Themes";
    std::shared_ptr<RadioBoxGroup> lThemeSelector{nullptr};

    try {
        int lCount{0};
        for (const auto& lEntry : std::filesystem::directory_iterator(lPath)) {
            if (lEntry.is_directory()) {
                if (lCount == 0) {
                    // Only make the theme selector at all if there are themes
                    lThemeSelector = std::make_shared<RadioBoxGroup>(
                        *this, "Select Theme:", ScaleThemeSelector, reinterpret_cast<int&>(mThemeSelector));
                }
                lThemeSelector->AddRadioBox(lEntry.path().stem().string());
                if (GetModel().mTheme == lEntry.path().stem().string()) {
                    lThemeSelector->SetChecked(lCount);
                }
                lCount++;
            }
        }

        AddObject(lThemeSelector);
    } catch (std::filesystem::filesystem_error aException) {
        Logger::GetInstance().Log(std::string("Could not open themes directory: ") + aException.what(),
                                  Logger::Level::WARNING);
    }

    AddObject({std::make_shared<Button>(
        *this,
        "Save selection",
        [&] { return ScaleDoneButton(GetHeightReference(), GetWidthReference()); },
        [&] {
            auto lRadioBoxGroup = std::dynamic_pointer_cast<RadioBoxGroup>(GetObjects().at(0));
            if (lRadioBoxGroup != nullptr) {
                GetModel().mTheme = lRadioBoxGroup->GetRadioBoxName(mThemeSelector);
            }
            GetModel().mWindowDone = true;
            return true;
        })});

    AddObject(CreateQuitText(*this, GetHeightReference()));
}

void ThemeWindow::Draw()
{
    Window::Draw();
}
