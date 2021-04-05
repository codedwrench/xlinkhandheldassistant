#include "../../Includes/UserInterface/KeyboardController.h"

/* Copyright (c) 2021 [Rick de Bondt] - KeyboardController.cpp */

#include <chrono>
#include <thread>
#include <utility>

#include <curses.h>

KeyboardController::KeyboardController(std::function<void(unsigned int)> aCallback) : mCallback(std::move(aCallback)) {}

unsigned int KeyboardController::Process()
{
    unsigned int lKey;
    lKey = getch();
    return lKey;
}

void KeyboardController::StartThread()
{
    mDone    = false;
    mRunning = true;
    mThread  = std::make_unique<std::thread>([&] {
        while (mRunning) {
            mCallback(Process());
        }
        mDone = true;
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