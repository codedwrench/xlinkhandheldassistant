#include "../../Includes/UserInterface/RadioBoxGroup.h"

#include <functional>
#include <iostream>
#include <string>
#include <utility>

/* Copyright (c) 2020 [Rick de Bondt] - RadioBoxGroup.cpp */

RadioBoxGroup::RadioBoxGroup(IWindow&                    aWindow,
                             std::string_view            aName,
                             std::function<Dimensions()> aCalculation,
                             int&                        aSelectionReference,
                             bool                        aSelected,
                             bool                        aVisible,
                             bool                        aSelectable) :
    UIObject(aWindow, aName, std::move(aCalculation), aVisible, aSelectable),
    mSelected(aSelected), mSelectionReference(aSelectionReference)
{}

void RadioBoxGroup::AddRadioBox(std::string_view aName)
{
    Dimensions lDimensions{};
    lDimensions.at(0) = GetYCoord() + mRadioBoxes.size() + 1;
    lDimensions.at(1) = GetXCoord();
    std::function<Dimensions()> lCalculation{[&]() { return lDimensions; }};

    mRadioBoxes.emplace_back(GetWindow(), aName, lCalculation);

    if ((mRadioBoxes.size() > 1) && mSelectionIndex < static_cast<int>(mRadioBoxes.size())) {
        SetHasDownAction(true);
    }
}

void RadioBoxGroup::ClearRadioBoxes()
{
    mRadioBoxes.clear();
}

std::string_view RadioBoxGroup::GetRadioBoxName(int aRadioBox)
{
    return mRadioBoxes.at(aRadioBox).GetName();
}

void RadioBoxGroup::SetChecked(int aRadioBox)
{
    for (auto& lRadioBox : mRadioBoxes) {
        lRadioBox.SetChecked(false);
    }
    mRadioBoxes.at(aRadioBox).SetChecked(true);
}

void RadioBoxGroup::Draw()
{
    GetWindow().DrawString(GetYCoord(), GetXCoord(), 1, GetName());
    for (auto& lRadioBox : mRadioBoxes) {
        if (lRadioBox.IsVisible()) {
            lRadioBox.Draw();
        }
    }
}

bool RadioBoxGroup::HandleKey(unsigned int aKeyCode)
{
    bool lReturn{false};

    if (!mRadioBoxes.empty()) {
        if (aKeyCode == KEY_UP) {
            if (mSelectionIndex > 0) {
                mRadioBoxes.at(mSelectionIndex).SetSelected(false);
                mSelectionIndex--;
                mRadioBoxes.at(mSelectionIndex).SetSelected(true);

                if (mSelectionIndex < static_cast<int>(mRadioBoxes.size() - 1)) {
                    SetHasDownAction(true);
                }

                // Min reached
                if (mSelectionIndex == 0) {
                    SetHasUpAction(false);
                }
            }
        } else if (aKeyCode == KEY_DOWN) {
            if (mSelectionIndex < static_cast<int>(mRadioBoxes.size() - 1)) {
                // When first entered, this index is set to -1
                if (mSelectionIndex >= 0) {
                    mRadioBoxes.at(mSelectionIndex).SetSelected(false);
                }
                mSelectionIndex++;
                mRadioBoxes.at(mSelectionIndex).SetSelected(true);

                if (mSelectionIndex > 0) {
                    SetHasUpAction(true);
                }

                // Max reached
                if (mSelectionIndex == mRadioBoxes.size() - 1) {
                    SetHasDownAction(false);
                }
            }
        } else if (aKeyCode == ' ' || aKeyCode == '\n' || aKeyCode == '\r') {
            if (mSelectionIndex >= 0 && !mRadioBoxes.at(mSelectionIndex).IsChecked()) {
                for (auto& lRadioBox : mRadioBoxes) {
                    lRadioBox.SetChecked(false);
                }
                mRadioBoxes.at(mSelectionIndex).SetChecked(true);
                mSelectionReference = mSelectionIndex;
            }
            lReturn = true;
        }
    }

    return lReturn;
}

void RadioBoxGroup::SetSelected(bool aSelected)
{
    mSelected = aSelected;

    // If it's the first time we are hitting this selection box, it will be -1
    if (mSelectionIndex < 0) {
        mSelectionIndex = 0;
    }

    if (!mRadioBoxes.empty()) {
        if (mSelected) {
            mRadioBoxes.at(mSelectionIndex).SetSelected(true);
        } else {
            mRadioBoxes.at(mSelectionIndex).SetSelected(false);
        }
    }
}

bool RadioBoxGroup::IsSelected() const
{
    return mSelected;
}