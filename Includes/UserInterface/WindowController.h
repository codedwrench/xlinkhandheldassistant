#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - MainWindow.h
 *
 * This file contains a class for a userinterface main window.
 *
 **/

#define PDC_WIDE
#define CHTYPE_32
#include <curses.h>

#include "IWindow.h"
#undef timeout

#include <cmath>
#include <functional>
#include <memory>
#include <vector>

#include "Window.h"

/**
 * This class will setup our window layout and basically handle everything
 * around the user interface.
 */
class WindowController
{
    using WindowList = std::vector<std::shared_ptr<IWindow>>;

public:
    /**
     * Constructor of WindowController, handles everything around the user
     * interface.
     * @param aModel - A model where the window can leave its data for the rest of
     * the program.
     */
    explicit WindowController(WindowModel& aModel);
    ~WindowController();

    WindowController(const WindowController& aWindowController)            = delete;
    WindowController& operator=(const WindowController& aWindowController) = delete;

    WindowController& operator=(WindowController&& aWindowController) = delete;
    WindowController(WindowController&& aWindowController)            = delete;

    /**
     * Sets up the layout and assigns the keybinds and such.
     * @return true if successful
     */
    bool SetUp();

    /**
     * Redraws and handles windows.
     * @return true if still processing
     */
    bool Process();

private:
    void AdvanceWindow();

    Window::NCursesWindow                    mMainCanvas;
    int                                      mHeight;
    int                                      mWidth;
    bool                                     mDimensionsChanged;
    WindowList                               mWindows;
    bool                                     mExclusiveWindow;
    std::pair<int, std::shared_ptr<IWindow>> mWindowSelector;
    WindowModel&                             mModel;
};
