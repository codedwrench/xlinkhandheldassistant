#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - RadioBoxGroup.h
 *
 * This file contains an class for a userinterface RadioBoxGroup.
 * This holds a group of radioboxes which can only be activated one at a time.
 *
 **/

#include <vector>

#include "RadioBox.h"
#include "UIObject.h"
#include "Window.h"

/**
 * Class for a userinterface RadioBoxGroup.
 */
class RadioBoxGroup : public UIObject
{
public:
    RadioBoxGroup(IWindow&                    aWindow,
                  std::string_view            aName,
                  std::function<Dimensions()> aCalculation,
                  int&                        aSelectionReference,
                  bool                        aSelected   = false,
                  bool                        aVisible    = true,
                  bool                        aSelectable = true);

    void Draw() override;
    bool HandleKey(unsigned int aKeyCode) override;

    void               SetSelected(bool aSelected) override;
    [[nodiscard]] bool IsSelected() const override;

    /**
     * Adds a radiobox to the radiobox group.
     * @param aName - Name of the radiobox that needs to be added.
     */
    void AddRadioBox(std::string_view aName);

    /**
     * Gets a radiobox name.
     * @param aRadioBox - The index of the radiobox to check.
     */
    std::string_view GetRadioBoxName(int aRadioBox);

    /**
     * Checks a radiobox without user interaction.
     * @param aRadioBox - The index of the radiobox to check.
     */
    void SetChecked(int aRadioBox);

    /**
     * Removes all radioboxes from this radio button group.
     */
    void ClearRadioBoxes();

private:
    std::vector<RadioBox> mRadioBoxes;
    bool                  mSelected;
    int                   mSelectionIndex{-1};
    int&                  mSelectionReference;
};
