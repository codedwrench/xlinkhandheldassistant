/* Copyright (c) 2021 [Rick de Bondt] - ThemeWindow.cpp */

#include "UserInterface/ThemeWindow.h"

#include "UserInterface/Button.h"
#include "UserInterface/DefaultElements.h"

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

#ifdef __APPLE__
#include "Availability.h"
#if (__MAC_OS_X_VERSION_MIN_REQUIRED <= 101500)
#define HAS_NO_FILESYSTEM
#endif
#endif

// 2 Implementations to also support older macOS versions
#ifdef HAS_NO_FILESYSTEM
#include <dirent.h>
#include <stdio.h>

void ThemeWindow::GetThemes(std::string_view aPath, std::shared_ptr<RadioBoxGroup> aThemeSelector)
{
    int lCount{0};

    DIR* lDirectory = opendir(aPath.data());

    if (lDirectory) {
        dirent* lEntry = readdir(lDirectory);

        while (lEntry != nullptr) {
            if (lEntry->d_type == DT_DIR && std::string(lEntry->d_name) != "." && std::string(lEntry->d_name) != "..") {
                if (lCount == 0) {
                    // Only make the theme selector at all if there are themes
                    aThemeSelector = std::make_shared<RadioBoxGroup>(
                        *this, "Select Theme:", ScaleThemeSelector, reinterpret_cast<int&>(mThemeSelector));
                }


                aThemeSelector->AddRadioBox(lEntry->d_name);
                if (GetModel().mTheme == lEntry->d_name) {
                    aThemeSelector->SetChecked(lCount);
                }
                lCount++;
            }

            lEntry = readdir(lDirectory);
        }
        if (aThemeSelector != nullptr) {
            AddObject(aThemeSelector);
        }
        closedir(lDirectory);
    }
}
#else
#include <filesystem>

void ThemeWindow::GetThemes(std::string_view aPath, std::shared_ptr<RadioBoxGroup> aThemeSelector)
{
    try {
        int lCount{0};
        for (const auto& lEntry : std::filesystem::directory_iterator(aPath)) {
            if (lEntry.is_directory()) {
                if (lCount == 0) {
                    // Only make the theme selector at all if there are themes
                    aThemeSelector = std::make_shared<RadioBoxGroup>(
                        *this, "Select Theme:", ScaleThemeSelector, reinterpret_cast<int&>(mThemeSelector));
                }
                aThemeSelector->AddRadioBox(lEntry.path().stem().string());
                if (GetModel().mTheme == lEntry.path().stem().string()) {
                    aThemeSelector->SetChecked(lCount);
                }
                lCount++;
            }
        }

        if (aThemeSelector != nullptr) {
            AddObject(aThemeSelector);
        }
    } catch (std::filesystem::filesystem_error aException) {
        Logger::GetInstance().Log(std::string("Could not open themes directory: ") + aException.what(),
                                  Logger::Level::WARNING);
    }
}
#endif

void ThemeWindow::SetUp()
{
    Window::SetUp();

    // Get size of window so scaling works properly.
    GetSize();

    std::string                    lPath = GetModel().mProgramPath + "Themes";
    std::shared_ptr<RadioBoxGroup> lThemeSelector{nullptr};

    GetThemes(lPath, lThemeSelector);

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
