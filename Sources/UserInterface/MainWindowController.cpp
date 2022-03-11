/* Copyright (c) 2020 [Rick de Bondt] - MainWindowController.cpp **/

#include "UserInterface/MainWindowController.h"

#include <iostream>

#include "UserInterface/HUDController.h"
#include "UserInterface/Wizard/WizardController.h"

MainWindowController::MainWindowController(WindowModel& aWindowModel, bool aSkipWizard) :
    WindowControllerBase(aWindowModel), mSkipWizard(aSkipWizard)
{}

bool MainWindowController::SetUp()
{
    bool lReturn{false};

    initscr();
    keypad(stdscr, true);
    nonl();
    cbreak();
    noecho();
    timeout(500);
    resize_term(0, 0);

    int lHeight{0};
    int lWidth{0};
    getmaxyx(stdscr, lHeight, lWidth);

    if (lHeight >= 24 && lWidth >= 80) {
        if (has_colors()) {
            start_color();
            init_pair(1, COLOR_WHITE, COLOR_BLACK);
            init_pair(2, COLOR_WHITE, COLOR_BLUE);
            init_pair(3, COLOR_BLACK, COLOR_CYAN);
            init_pair(4, COLOR_BLUE, COLOR_BLACK);
            init_pair(5, COLOR_CYAN, COLOR_BLACK);
            init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(7, COLOR_BLACK, COLOR_WHITE);
        }

        WindowControllerBase::SetUp();

        if (!mSkipWizard) {
            // Setup subcontroller, we start with the wizard:
            SetSubController(std::make_unique<WizardController>(GetWindowModel()));
        } else {
            // Setup subcontroller, skip the wizard so go to the HUD:
            SetSubController(std::make_unique<HUDController>(GetWindowModel()));
        }

        if (GetSubController() != nullptr) {
            lReturn = GetSubController()->SetUp();
        }
    } else {
        endwin();
        Logger::GetInstance().SetLogToScreen(true);
        Logger::GetInstance().Log("Terminal size too small: " + std::to_string(lWidth) + "x" + std::to_string(lHeight) +
                                      ", make sure the terminal is at least 80x24",
                                  Logger::Level::ERROR);
        lReturn = false;
    }

    return lReturn;
}

bool MainWindowController::KeyAction(unsigned int aAction)
{
    bool lReturn{};
    lReturn = WindowControllerBase::KeyAction(aAction);

    return lReturn;
}

bool MainWindowController::Process()
{
    bool lReturn{false};

    lReturn = WindowControllerBase::Process();
    if (!lReturn) {
        // If we are dealing with a wizard controller, the wizard is done
        if (dynamic_cast<WizardController*>(GetSubController().get()) != nullptr) {
            // Save the config
            GetWindowModel().SaveToFile(GetWindowModel().mProgramPath + "config.txt");

            // Pull up HUD
            SetSubController(std::make_unique<HUDController>(GetWindowModel()));
            if (GetSubController() != nullptr) {
                lReturn = GetSubController()->SetUp();
            }
        }
        if (GetWindowModel().mWizardSelected) {
            GetWindowModel().mWizardSelected = false;
            SetSubController(std::make_unique<WizardController>(GetWindowModel()));
            // TODO: Add way to go to sub-step
            if (GetSubController() != nullptr) {
                lReturn = GetSubController()->SetUp();
            }
        }
    }

    return lReturn;
}

MainWindowController::~MainWindowController()
{
    endwin();
}
