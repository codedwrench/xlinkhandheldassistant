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

    MainWindowController(MainWindowController const& aController)     = delete;
    MainWindowController(MainWindowController&& aController) noexcept = delete;

    MainWindowController& operator=(MainWindowController aController) = delete;
    MainWindowController& operator=(MainWindowController&& aController) = delete;

    ~MainWindowController();

    bool SetUp() override;
    bool KeyAction(unsigned int aAction) override;
    bool Process() override;

private:
    bool mSkipWizard{};
};