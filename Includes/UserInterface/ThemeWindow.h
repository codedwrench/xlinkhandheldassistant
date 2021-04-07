#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - ThemeWindow.h
 *
 * This file contains the ThemeWindow class.
 *
 **/

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
    int mThemeSelector{0};
};
