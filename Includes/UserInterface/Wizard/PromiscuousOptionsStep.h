
#pragma once

#include "../Window.h"

/* Copyright (c) 2021 [Rick de Bondt] - PromiscuousOptionsStep.h
 *
 * This file contains an class for a userinterface plugin options window.
 *
 **/

/**
 * Class that will setup and draw a plugin options window.
 **/
class PromiscuousOptionsStep : public Window
{
public:
    PromiscuousOptionsStep(WindowModel&                        aModel,
                           std::string_view                    aTitle,
                           std::function<Window::Dimensions()> aCalculation);

    void SetUp() final;
    void Draw() override;
};