#include "../../Includes/UserInterface/HUDWindow.h"

#include <cmath>
#include <utility>

#include "../../Includes/UserInterface/Button.h"
#include "../../Includes/UserInterface/CheckBox.h"
#include "../../Includes/UserInterface/DefaultElements.h"
#include "../../Includes/UserInterface/TextField.h"

/* Copyright (c) 2021 [Rick de Bondt] - HUDWindow.cpp */

namespace
{
    Dimensions ScalePicture(const int& aMaxHeight, const int& aMaxWidth, const std::string& aPicture)
    {
        // Pictures need to have the right amount of spaces
        // For height \n characters get counted and divided by 2
        // For width, the space until the first \n gets counted and divided by 2

        auto lFirstReturn{aPicture.find('\n')};
        auto lPictureWidth{(lFirstReturn == std::string::npos) ? aPicture.length() : lFirstReturn};

        return {(aMaxHeight / 2) -
                    static_cast<int>(std::count_if(
                        aPicture.begin(), aPicture.end(), [](char aCharacter) { return (aCharacter == '\n'); })) /
                        2,
                static_cast<int>((aMaxWidth / 2) - (lPictureWidth / 2)),
                0,
                0};
    }

    Dimensions ScaleConnectedTo(const int& aMaxWidth, const std::string& aText)
    {
        return {1, aMaxWidth - static_cast<int>(std::string("Connected to: ").length() + aText.length() + 1), 0, 0};
    }

    Dimensions ScaleEngineStatus(const int& aMaxHeight, const int& aMaxWidth, std::string_view aText)
    {
        return {(aMaxHeight - 1),
                (aMaxWidth / 2) - static_cast<int>((std::string("Status: ").length() + aText.length()) / 2),
                0,
                0};
    }

    Dimensions ScaleOptionsButton(const int& aMaxHeight, const int& aMaxWidth)
    {
        return {(aMaxHeight - 2),
                (aMaxWidth - 2) - (static_cast<int>(std::string("[ Start Engine ]").length() + 2 +
                                                    std::string("[ Options ]").length())),
                0,
                0};
    }

    Dimensions ScaleStartEngineButton(const int& aMaxHeight, const int& aMaxWidth)
    {
        return {(aMaxHeight - 2), (aMaxWidth - 2) - static_cast<int>(std::string("[ Start Engine ]").length()), 0, 0};
    }

    Dimensions ScaleHostingButton(const int& aMaxHeight, const int& aMaxWidth)
    {
        return {(aMaxHeight - 4), (aMaxWidth - 2) - static_cast<int>(std::string("[ ]  Hosting").length()), 0, 0};
    }

}  // namespace

static inline std::string LoadPicture(std::string_view aFile)
{
    std::string   lReturn{};
    std::ifstream lFile;
    lFile.open(aFile.data());
    std::string lLine{};

    if (lFile.is_open()) {
        lReturn = "";
        while (std::getline(lFile, lLine)) {
            lReturn += lLine + '\n';
        }
        lFile.close();
    }
    return lReturn;
}

Dimensions HUDWindow::ScaleReConnectionButton()
{
    // TODO: REALLY FUCKING FIX THIS ARRAY ACCESSING YOU'VE BEEN DOING
    return {GetHeightReference() - 2,
            GetObjects().at(3)->GetXCoord() - static_cast<int>(std::string("[ Re-Connect ]").length()) - 2,
            0,
            0};
}

HUDWindow::HUDWindow(WindowModel& aModel, std::string_view aTitle, std::function<Dimensions()> aCalculation) :
    Window(aModel, aTitle, aCalculation)
{}

void HUDWindow::SetUp()
{
    Window::SetUp();

    // Lets grab our OFF/ON picture
    std::string lOffPicture =
        LoadPicture(GetModel().mProgramPath + "Themes" + "/" + GetModel().mTheme + "/" + "off.txt");
    if (!lOffPicture.empty()) {
        mOffPicture = lOffPicture;
    }

    std::string lOnPicture = LoadPicture(GetModel().mProgramPath + "Themes" + "/" + GetModel().mTheme + "/" + "on.txt");
    if (!lOnPicture.empty()) {
        mOnPicture = lOnPicture;
    }

    // Get size of window so scaling works properly.
    GetSize();

    AddObject({std::make_shared<String>(*this, *mActivePicture, [&] {
        return ScalePicture(GetHeightReference(), GetWidthReference(), *mActivePicture);
    })});

    AddObject({std::make_shared<String>(
        *this,
        "Connected to: " + GetModel().mCurrentlyConnectedNetwork,
        [&] { return ScaleConnectedTo(GetWidthReference(), GetModel().mCurrentlyConnectedNetwork); },
        !GetModel().mCurrentlyConnectedNetwork.empty())});

    AddObject({std::make_shared<String>(
        *this, "Status: " + std::string(WindowModel_Constants::cEngineStatusTexts.at(GetModel().mEngineStatus)), [&] {
            return ScaleEngineStatus(GetHeightReference(),
                                     GetWidthReference(),
                                     WindowModel_Constants::cEngineStatusTexts.at(GetModel().mEngineStatus));
        })});

    AddObject({std::make_shared<Button>(
        *this,
        "Options",
        [&] { return ScaleOptionsButton(GetHeightReference(), GetWidthReference()); },
        [&] {
            GetModel().mOptionsSelected = true;
            return true;
        })});

    AddObject({std::make_shared<Button>(
        *this,
        "Re-Connect",
        [&] { return ScaleReConnectionButton(); },
        [&] {
            GetModel().mCommand = WindowModel_Constants::Command::ReConnect;
            return true;
        },
        false,
        // Only show if we're using the plugin method
        GetModel().mConnectionMethod == WindowModel_Constants::ConnectionMethod::Plugin)});

    AddObject({std::make_shared<Button>(
        *this,
        "Start Engine",
        [&] { return ScaleStartEngineButton(GetHeightReference(), GetWidthReference()); },
        [&] {
            if (GetModel().mEngineStatus == WindowModel_Constants::EngineStatus::Idle) {
                GetModel().mCommand = WindowModel_Constants::Command::StartEngine;
            } else if (GetModel().mEngineStatus == WindowModel_Constants::EngineStatus::Running) {
                GetModel().mCommand = WindowModel_Constants::Command::StopEngine;
            }
            return true;
        })});

    AddObject({std::make_shared<CheckBox>(
        *this,
        "Hosting",
        [&] { return ScaleHostingButton(GetHeightReference(), GetWidthReference()); },
        GetModel().mHosting
        )});

    AddObject(CreateQuitText(*this, GetHeightReference()));
}

void HUDWindow::Draw()
{
    if (GetModel().mEngineStatus == WindowModel_Constants::EngineStatus::Idle) {
        // TODO: This still needs a better method
        mActivePicture = &mOffPicture;
        GetObjects().at(0)->SetName(*mActivePicture);
        GetObjects().at(0)->Scale();
        GetObjects().at(5)->SetName("Start Engine");
    } else if (GetModel().mEngineStatus == WindowModel_Constants::EngineStatus::Running) {
        mActivePicture = &mOnPicture;
        GetObjects().at(0)->SetName(*mActivePicture);
        GetObjects().at(0)->Scale();

        auto lStartStopButton = std::dynamic_pointer_cast<Button>(GetObjects().at(5));

        // Clear line so you won't get double text
        ClearLine(
            lStartStopButton->GetYCoord(), lStartStopButton->GetXCoord(), std::string("[ Start Engine ]").length());

        lStartStopButton->SetName("Stop Engine");
    }

    if (mOldConnected != GetModel().mCurrentlyConnectedNetwork) {
        // The Connected to: will otherwise show up multiple times
        ClearLine(1, 1, GetWidthReference() - 1);
        mOldConnected = GetModel().mCurrentlyConnectedNetwork;
        GetObjects().at(1)->SetVisible(!GetModel().mCurrentlyConnectedNetwork.empty());
        GetObjects().at(1)->SetName("Connected to: " + GetModel().mCurrentlyConnectedNetwork);
        GetObjects().at(1)->Scale();
    }

    GetObjects().at(2)->SetName(std::string("Status: ") +
                                std::string(WindowModel_Constants::cEngineStatusTexts.at(GetModel().mEngineStatus)));

    Window::Draw();
}
