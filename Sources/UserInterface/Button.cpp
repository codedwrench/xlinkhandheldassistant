/* Copyright (c) 2020 [Rick de Bondt] - Button.cpp */

#include "UserInterface/Button.h"

#include <string>
#include <utility>

#include "UserInterface/NCursesKeys.h"

Button::Button(IWindow&                            aWindow,
               std::string_view                    aName,
               std::function<Window::Dimensions()> aCalculation,
               std::function<bool()>               aAction,
               bool                                aSelected,
               bool                                aVisible,
               bool                                aSelectable) :
    UIObject(aWindow, aName, std::move(aCalculation), aVisible, aSelectable),
    mSelected(aSelected), mAction{std::move(aAction)}
{}

void Button::Draw()
{
    if (IsVisible()) {
        std::string lButtonString{std::string("[ ") + GetName().data() + " ]"};
        auto        lColorPair{static_cast<unsigned int>(mSelected ? 7 : 1)};
        GetWindow().DrawString(GetYCoord(), GetXCoord(), lColorPair, lButtonString);
    }
}

bool Button::HandleKey(unsigned int aKeyCode)
{
    bool lReturn{false};

    if (aKeyCode == ' ' || aKeyCode == '\n' || aKeyCode == '\r' || aKeyCode == cCombinedKeypadCenter) {
        lReturn = mAction();
    }
    return lReturn;
}

void Button::SetSelected(bool aSelected)
{
    mSelected = aSelected;
}

bool Button::IsSelected() const
{
    return mSelected;
}
