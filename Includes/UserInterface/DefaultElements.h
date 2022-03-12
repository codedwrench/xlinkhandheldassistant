#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - DefaultElements.h
 *
 * This file contains some reusable elements for different windows.
 *
 **/

#include "Button.h"
#include "String.h"

static Window::Dimensions ScaleQuitText(const int& aMaxHeight)
{
    return {aMaxHeight - 1, 1, 0, 0};
}

static Window::Dimensions ScaleWizardStepText(const int& aMaxWidth)
{
    return {0, static_cast<int>(aMaxWidth - 1 - std::string("Step: */*").length()), 0, 0};
}

static Window::Dimensions ScaleNextButton(const int& aMaxHeight, const int& aMaxWidth)
{
    return {aMaxHeight - 2, static_cast<int>(aMaxWidth - 6 - std::string("Next").length()), 0, 0};
}

/**
 * Gives a next button in the wizard.
 * @param aWindow - Window to put this prompt onto.
 * @param aModel - Model to use to show the next button has been pressed.
 * @param aMaxHeight - The height of the window (will be used for scaling).
 * @param aMaxWidth - The width of the window (will be used for scaling).
 * @return Object that can be added to the window.
 */
static inline std::shared_ptr<Button> CreateNextButton(IWindow&     aWindow,
                                                       WindowModel& aModel,
                                                       const int&   aMaxHeight,
                                                       const int&   aMaxWidth)
{
    auto lNextButton{std::make_shared<Button>(
        aWindow,
        "Next",
        [&] { return ScaleNextButton(aMaxHeight, aMaxWidth); },
        [&] {
            aModel.mWindowDone = true;
            return true;
        })};

    return lNextButton;
}

/**
 * Gives a prompt in the right-top corner that looks like "Step: X/X".
 * @param aWindow - Window to put this prompt onto.
 * @param aStep - The step that will be put on the left side.
 * @param aAmountOfSteps - The amount of steps that are there.
 * @param aMaxWidth - The width of the window (will be used for scaling).
 * @return Object that can be added to the window.
 */
static inline std::shared_ptr<String> CreateWizardStepText(IWindow&   aWindow,
                                                           int        aStep,
                                                           int        aAmountOfSteps,
                                                           const int& aMaxWidth)
{
    auto lWizardStepText{
        std::make_shared<String>(aWindow, "Step: " + std::to_string(aStep) + "/" + std::to_string(aAmountOfSteps), [&] {
            return ScaleWizardStepText(aMaxWidth);
        })};

    return lWizardStepText;
}

/**
 * Gives a prompt in the lower-left corner that looks like "Press 'q' to quit".
 * @param aWindow - Window to put this prompt onto.
 * @param aMaxHeight - The height of the window (will be used for scaling).
 * @return Object that can be added to the window.
 */
static inline std::shared_ptr<String> CreateQuitText(IWindow& aWindow, const int& aMaxHeight)
{
    auto lQuitText{std::make_shared<String>(aWindow, "Press 'q' to quit", [&] { return ScaleQuitText(aMaxHeight); })};

    return lQuitText;
}
