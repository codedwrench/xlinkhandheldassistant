#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - KeyboardController.h
 *
 * This file contains the interface for a KeyboardController.
 * The keyboard controller will send the actions taken within the WindowController
 *
 **/

#include <functional>
#include <thread>

class KeyboardController
{
public:
    /**
     * Constructor of the keyboard controller.
     * @param aCallback - Where to send the obtained key events to.
     */
    KeyboardController(std::function<void(unsigned int)> aCallback);

    ~KeyboardController();

    /**
     * Waits for a keyboard event.
     * @return keyboard key pressed.
     */
    static unsigned int Process();

    /**
     * Starts the input event thread.
     * This will makes sure every keyboard event will make it to the UI.
     */
    void StartThread();

    /**
     * Stops the input event thread.
     * If this function is called the keyboard controller will stop processing events.
     */
    void StopThread();

private:
    std::function<void(unsigned int)> mCallback;
    bool                              mDone{true};
    bool                              mRunning{false};
    std::unique_ptr<std::thread>      mThread{nullptr};
};
