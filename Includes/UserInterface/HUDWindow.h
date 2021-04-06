#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - HUDWindow.h
 *
 * This file contains the HUDWindow.
 *
 **/

#include "Window.h"

/**
 * Class that will setup and draw a HUD window.
 **/
class HUDWindow : public Window
{
public:
    HUDWindow(WindowModel& aModel, std::string_view aTitle, std::function<Dimensions()> aCalculation);

    virtual void SetUp() final;
    void         Draw() override;

private:
    // If you want nice ascii art, add on/off txt files
    std::string mOffPicture{"O" + '\n'};
    std::string mOnPicture{"( ( O ) )" + '\n'};
    std::string mOldConnected{""};

    Dimensions ScaleReConnectionButton();
};
