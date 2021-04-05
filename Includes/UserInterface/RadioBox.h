#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - RadioBox.h
 *
 * This file contains an class for a userinterface radiobox.
 *
 **/

#include "UIObject.h"
#include "Window.h"

/**
 * Class for a userinterface radiobox.
 */
class RadioBox : public UIObject
{
public:
    RadioBox(IWindow&                    aWindow,
             std::string_view            aName,
             std::function<Dimensions()> aCalculation,
             bool                        aSelected   = false,
             bool                        aVisible    = true,
             bool                        aSelectable = true);

    void Draw() override;
    bool HandleKey(unsigned int aKeyCode) override;

    void               SetChecked(bool aChecked);
    [[nodiscard]] bool IsChecked() const;

    void               SetSelected(bool aSelected) override;
    [[nodiscard]] bool IsSelected() const override;

private:
    bool mChecked{false};
    bool mSelected;
};
