#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - AboutWindow.h
 *
 * This file contains the AboutWindow class.
 *
 **/

#include "Window.h"

/**
 * Class that will setup and draw an about window.
 **/
class AboutWindow : public Window
{
public:
    AboutWindow(WindowModel& aModel, std::string_view aTitle, std::function<Window::Dimensions()> aCalculation);

    void SetUp() final;
    void Draw() override;

private:
};
