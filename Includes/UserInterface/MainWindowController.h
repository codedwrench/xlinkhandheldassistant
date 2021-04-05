#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - MainWindowController.h
 *
 * This file contains the MainWindowController.
 *
 **/

#include "WindowControllerBase.h"

class MainWindowController : public WindowControllerBase
{
public:
    MainWindowController(WindowModel& aWindowModel, bool aSkipWizard);
    ~MainWindowController();
    bool SetUp() override;
    bool KeyAction(unsigned int aAction) override;
    bool Process() override;

private:
    bool mSkipWizard{};
};