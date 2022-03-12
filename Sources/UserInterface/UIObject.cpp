/* Copyright (c) 2020 [Rick de Bondt] - UIObject.cpp */

#include "UserInterface/UIObject.h"

#include <string>
#include <utility>

UIObject::UIObject(IWindow&                            aWindow,
                   std::string_view                    aName,
                   std::function<Window::Dimensions()> aCalculation,
                   bool                                aVisible,
                   bool                                aSelectable) :
    mWindow(aWindow),
    mName(aName), mSelectable(aSelectable), mVisible(aVisible), mYCoord(0), mXCoord(0),
    mScaleCalculation(std::move(aCalculation))
{
    Window::Dimensions lParameters{mScaleCalculation()};
    mYCoord = lParameters.at(0);
    mXCoord = lParameters.at(1);
}

bool UIObject::HandleKey(unsigned int /*aKeyCode*/)
{
    // Base object does not handle keys
    return false;
}

bool UIObject::HasDownAction()
{
    return mHasDownAction;
}

bool UIObject::HasUpAction()
{
    return mHasUpAction;
}

void UIObject::SetHasDownAction(bool aHasAction)
{
    mHasDownAction = aHasAction;
}

void UIObject::SetHasUpAction(bool aHasAction)
{
    mHasUpAction = aHasAction;
}

void UIObject::Scale()
{
    Window::Dimensions lParameters{mScaleCalculation()};
    mYCoord = lParameters.at(0);
    mXCoord = lParameters.at(1);
}

bool UIObject::IsSelected() const
{
    // Non selectable, not selected;
    return false;
}

void UIObject::SetSelected(bool /*aSelected*/)
{
    // By default an object is not selectable so ignore.
}

void UIObject::SetVisible(bool aVisible)
{
    mVisible = aVisible;

    // Also clear the line that this is on.
    if (!mVisible) {
        mWindow.ClearLine(mYCoord, 0, mWindow.GetSize().second);
    }
}

bool UIObject::IsVisible() const
{
    return mVisible;
}

IWindow& UIObject::GetWindow()
{
    return mWindow;
}

std::string_view UIObject::GetName()
{
    return mName;
}

void UIObject::SetName(std::string_view aName)
{
    mName = aName;
}

bool UIObject::IsSelectable() const
{
    return mSelectable;
}

int UIObject::GetYCoord() const
{
    return mYCoord;
}

int UIObject::GetXCoord() const
{
    return mXCoord;
}