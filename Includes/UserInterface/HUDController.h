#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - HUDController.h
 *
 * This file contains the HUDController.
 *
 **/

#include "WindowControllerBase.h"

class HUDController : public WindowControllerBase
{
public:
    HUDController(WindowModel& aWindowModel);
    bool SetUp() override;
    bool KeyAction(unsigned int aAction) override;
    bool Process() override;

private:
    int mHeight{};
    int mWidth{};
};