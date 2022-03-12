#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - String.h
 *
 * This file contains an class for a userinterface string.
 *
 **/

#include "IWindow.h"
#include "UIObject.h"

class String : public UIObject
{
public:
    String(IWindow&                            aWindow,
           std::string_view                    aName,
           std::function<Window::Dimensions()> aCalculation,
           bool                                aVisible    = true,
           bool                                aSelectable = false,
           unsigned int                        aColorPair  = 1);
    void Draw() override;
    void SetColorPair(int aPair);

private:
    unsigned int mColorPair{1};
};