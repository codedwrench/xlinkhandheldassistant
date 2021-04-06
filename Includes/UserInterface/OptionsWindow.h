#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - OptionsWindow.h
 *
 * This file contains the OptionsWindow classes.
 *
 **/

#include "Window.h"

/**
 * Class that will setup and draw an options window.
 **/
class OptionsWindow : public Window
{
public:
    OptionsWindow(WindowModel& aModel, std::string_view aTitle, std::function<Dimensions()> aCalculation);

    virtual void SetUp() final;
    void         Draw() override;

private:
};
