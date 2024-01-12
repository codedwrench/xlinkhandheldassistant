/* Copyright (c) 2021 [Rick de Bondt] - AboutWindow.cpp */
#include "UserInterface/AboutWindow.h"

#include <algorithm>

#include "UserInterface/Button.h"
#include "UserInterface/DefaultElements.h"

namespace
{
    constexpr std::string_view cAboutText{R"(___   ___  __       __    __       ___
\  \ /  / |  |     |  |  |  |     /   \
 \  V  /  |  |     |  |__|  |    /  ^  \
  >   <   |  |     |   __   |   /  /_\  \
 /  .  \  |  `----.|  |  |  |  /  _____  \
/__/ \__\ |_______||__|  |__| /__/     \__\

          XLink Handheld Assistant
              Version: )"};

    constexpr std::string_view cVersion{GIT_VERSION};

    constexpr std::string_view cAboutFinisher{R"(

               by CodedWrench

         Made for use with XLink Kai
        <https://www.teamxlink.co.uk>)"};

    Window::Dimensions ScaleAboutText(const int& aMaxHeight, const int& aMaxWidth)
    {
        // About texts need to have the right amount of spaces
        // For height \n characters get counted and divided by 2
        // For width, the space until the first \n gets counted and divided by 2

        // In this case for height: (AboutText + 1 for version + AboutFinisher) / 2
        return {
            (aMaxHeight / 2) - static_cast<int>(std::count_if(cAboutText.begin(),
                                                              cAboutText.end(),
                                                              [](char aCharacter) { return (aCharacter == '\n'); }) +
                                                1 +
                                                std::count_if(cAboutFinisher.begin(),
                                                              cAboutFinisher.end(),
                                                              [](char aCharacter) { return (aCharacter == '\n'); })) /
                                   2,
            // TODO: Widest line is actually 5 wider, find a proper way to do this
            static_cast<int>((aMaxWidth / 2) - ((cAboutText.find('\n') + 5) / 2)),
            0,
            0};
    }

    Window::Dimensions ScaleReturnButton(const int& aMaxHeight, const int& aMaxWidth)
    {
        return {aMaxHeight - 2, static_cast<int>(aMaxWidth - 2 - std::string("[ Return to HUD ]").length()), 0, 0};
    }

}  // namespace

AboutWindow::AboutWindow(WindowModel&                        aModel,
                         std::string_view                    aTitle,
                         std::function<Window::Dimensions()> aCalculation) :
    Window(aModel, aTitle, aCalculation)
{}

void AboutWindow::SetUp()
{
    Window::SetUp();

    // Get size of window so scaling works properly.
    GetSize();


    AddObject({std::make_shared<String>(*this,
                                        std::string(cAboutText) + std::string(cVersion) + std::string(cAboutFinisher),
                                        [&] { return ScaleAboutText(GetHeightReference(), GetWidthReference()); })});

    AddObject({std::make_shared<Button>(
        *this,
        "Return to HUD",
        [&] { return ScaleReturnButton(GetHeightReference(), GetWidthReference()); },
        [&] {
            GetModel().mWindowDone = true;
            return true;
        })});


    AddObject(CreateQuitText(*this, GetHeightReference()));
}

void AboutWindow::Draw()
{
    Window::Draw();
}
