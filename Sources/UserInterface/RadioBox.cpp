/* Copyright (c) 2020 [Rick de Bondt] - RadioBox.cpp */

#include "UserInterface/RadioBox.h"

#include <string>
#include <utility>

#include "UserInterface/NCursesKeys.h"

RadioBox::RadioBox(IWindow&                            aWindow,
                   std::string_view                    aName,
                   std::function<Window::Dimensions()> aCalculation,
                   bool                                aSelected,
                   bool                                aVisible,
                   bool                                aSelectable) :
    UIObject(aWindow, aName, std::move(aCalculation), aVisible, aSelectable),
    mSelected(aSelected)
{}

void RadioBox::Draw()
{
    std::string lRadioBoxString{std::string("(") + (mChecked ? std::string("o") : std::string(" ")) + ")  " +
                                GetName().data()};
    auto        lColorPair{static_cast<unsigned int>(mSelected ? 7 : 1)};
    GetWindow().DrawString(GetYCoord(), GetXCoord(), lColorPair, lRadioBoxString);
}

bool RadioBox::HandleKey(unsigned int aKeyCode)
{
    bool lReturn{false};

    if (aKeyCode == ' ' || aKeyCode == '\n' || aKeyCode == '\r' || aKeyCode == cCombinedKeypadCenter) {
        mChecked = !mChecked;
        lReturn  = true;
    }

    return lReturn;
}

void RadioBox::SetChecked(bool aChecked)
{
    mChecked = aChecked;
}

bool RadioBox::IsChecked() const
{
    return mChecked;
}

void RadioBox::SetSelected(bool aSelected)
{
    mSelected = aSelected;
}

bool RadioBox::IsSelected() const
{
    return mSelected;
}
