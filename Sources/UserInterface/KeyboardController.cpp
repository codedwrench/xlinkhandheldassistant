/* Copyright (c) 2021 [Rick de Bondt] - KeyboardController.cpp */

#include "UserInterface/KeyboardController.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <utility>

#define PDC_WIDE
#define CHTYPE_32
#include <curses.h>

#include "Logger.h"
#include "UserInterface/NCursesKeys.h"

KeyboardController::KeyboardController(std::function<void(unsigned int)> aCallback) : mCallback(std::move(aCallback)) {}

unsigned int KeyboardController::Process()
{
    unsigned int lKey = 0;
    lKey = getch();
    return lKey;
}

void KeyboardController::StartThread()
{
    mDone    = false;
    mRunning = true;
    mThread  = std::make_unique<std::thread>([&] {
        unsigned int lNumCyclesSinceComboKeyPressed{0};
        bool         lComboKeyPressed{false};
        bool         lKeyIsKeypad{false};

        while (mRunning) {
            unsigned int lKey = Process();

            // Combo key pressed so check for a combo
            // For some reason this key is 'ESC' in ncurses
            if (lKey == cKeyEsc) {
                lNumCyclesSinceComboKeyPressed++;
                lComboKeyPressed = true;
                // I should probably differentiate between these two, but whatever
            } else if ((lComboKeyPressed && (lKey == cKeypadArrowCombo)) ||
                       (lComboKeyPressed && (lKey == cKeypadCenterCombo))) {
                lNumCyclesSinceComboKeyPressed++;
                lKeyIsKeypad = true;
            } else if (lComboKeyPressed) {
                lNumCyclesSinceComboKeyPressed++;

                // Wait for 4 cycles, after that we assume it was ESC
                if (lNumCyclesSinceComboKeyPressed >= 4) {
                    lNumCyclesSinceComboKeyPressed = 0;
                    lComboKeyPressed               = false;
                    lKeyIsKeypad                   = false;

                    mCallback(cKeyEsc);
                } else if (lKeyIsKeypad) {
                    lNumCyclesSinceComboKeyPressed = 0;
                    lComboKeyPressed               = false;
                    lKeyIsKeypad                   = false;

                    switch (lKey) {
                        case cKeypadUp:
                            lKey = cCombinedKeypadUp;
                            break;
                        case cKeypadDown:
                            lKey = cCombinedKeypadDown;
                            break;
                        case cKeypadLeft:
                            lKey = cCombinedKeypadLeft;
                            break;
                        case cKeypadRight:
                            lKey = cCombinedKeypadRight;
                            break;
                        case cKeypadCenter:
                            lKey = cCombinedKeypadCenter;
                            break;
                        default:
                            Logger::GetInstance().Log("Got unknown keycode: " + std::to_string(lKey),
                                                      Logger::Level::DEBUG);

                            lKey = -1;
                    }
                    mCallback(lKey);
                }
            } else {
                mCallback(lKey);
            }
            mDone = true;
        }
    });
}

void KeyboardController::StopThread()
{
    mRunning = false;

    while (!mDone || (mThread != nullptr && !mThread->joinable())) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if (mThread != nullptr) {
        mThread->join();
    }

    mThread = nullptr;
}

KeyboardController::~KeyboardController()
{
    StopThread();
}
