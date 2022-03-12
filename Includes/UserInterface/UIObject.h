#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - UIObject.h
 *
 * This file contains an abstract class for a userinterface object.
 *
 **/

#include <string>
#include <string_view>

#include "IUIObject.h"
#include "Window.h"

class UIObject : public IUIObject
{
public:
    UIObject(IWindow&                            aWindow,
             std::string_view                    aName,
             std::function<Window::Dimensions()> aCalculation,
             bool                                aVisible    = true,
             bool                                aSelectable = false);

    void Scale() override;

    void               SetVisible(bool aVisible) override;
    [[nodiscard]] bool IsVisible() const override;

    bool               HasDownAction() override;
    bool               HasUpAction() override;
    void               SetHasDownAction(bool aHasAction) override;
    void               SetHasUpAction(bool aHasAction) override;
    void               SetSelected(bool aSelected) override;
    [[nodiscard]] bool IsSelectable() const override;
    [[nodiscard]] bool IsSelected() const override;
    [[nodiscard]] int  GetYCoord() const override;
    [[nodiscard]] int  GetXCoord() const override;
    std::string_view   GetName() override;
    void               SetName(std::string_view aName) override;
    bool               HandleKey(unsigned int aKeyCode) override;

protected:
    IWindow& GetWindow();

private:
    IWindow&                            mWindow;
    bool                                mHasUpAction{false};
    bool                                mHasDownAction{false};
    std::string                         mName;
    bool                                mSelectable;
    bool                                mVisible;
    int                                 mYCoord;
    int                                 mXCoord;
    std::function<Window::Dimensions()> mScaleCalculation;
};