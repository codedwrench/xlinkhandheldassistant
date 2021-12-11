/* Copyright (c) 2020 [Rick de Bondt] - CheckBox.cpp */

#include "../../Includes/UserInterface/CheckBox.h"

#include <string>
#include <utility>

#include "../../Includes/UserInterface/NCursesKeys.h"

CheckBox::CheckBox(IWindow&                    aWindow,
                   std::string_view            aName,
                   std::function<Dimensions()> aCalculation,
                   bool&                       aModelCheckBox,
                   bool                        aSelected,
                   bool                        aVisible,
                   bool                        aSelectable) :
    UIObject(aWindow, aName, std::move(aCalculation), aVisible, aSelectable),
    mSelected(aSelected), mModelCheckBox{aModelCheckBox}
{}

void CheckBox::Draw()
{
    std::string lCheckBoxString{std::string("[") + (mModelCheckBox ? std::string("X") : std::string(" ")) + "]  " +
                                GetName().data()};
    int         lColorPair{mSelected ? 7 : 1};
    GetWindow().DrawString(GetYCoord(), GetXCoord(), lColorPair, lCheckBoxString);
}

bool CheckBox::HandleKey(unsigned int aKeyCode)
{
    bool lReturn{false};
    if (aKeyCode == ' ' || aKeyCode == '\n' || aKeyCode == '\r' || aKeyCode == cCombinedKeypadCenter) {
        mModelCheckBox = !mModelCheckBox;
        lReturn        = true;
    }

    return lReturn;
}

void CheckBox::SetChecked(bool aChecked)
{
    mModelCheckBox = aChecked;
}

bool CheckBox::IsChecked() const
{
    return mModelCheckBox;
}

void CheckBox::SetSelected(bool aSelected)
{
    mSelected = aSelected;
}

bool CheckBox::IsSelected() const
{
    return mSelected;
}
