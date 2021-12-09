#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - ThemeWindow.h
 *
 * This file contains the ThemeWindow class.
 *
 **/

#include "RadioBoxGroup.h"
#include "Window.h"

/**
 * Class that will setup and draw an theme selection window.
 **/
class ThemeWindow : public Window
{
public:
    ThemeWindow(WindowModel& aModel, std::string_view aTitle, std::function<Dimensions()> aCalculation);

    virtual void SetUp() final;
    void         Draw() override;

private:
    void GetThemes(std::string_view aPath, std::shared_ptr<RadioBoxGroup> aThemeSelector);

    int mThemeSelector{0};
};
