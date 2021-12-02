#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - IWindowController.h
 *
 * This file contains the interface for a WindowController.
 *
 **/

#include <functional>
#include <memory>

#include "WindowController.h"

class WindowModel;
class IWindowController
{
public:
    virtual ~IWindowController() = default;

    /**
     * Sets up the layout and sets up curses in case of the mainn controller.
     * @return true if successful
     */
    virtual bool SetUp() = 0;

    /**
     * Catches action given to the controller and handles it.
     * @param aAction - Action to handle, uses Curses keycodes.
     * @return true - If action successfully performed.
     */
    virtual bool KeyAction(unsigned int aAction) = 0;

    /**
     * Redraws and handles windows.
     * @return true if processing succeeded.
     */
    virtual bool Process() = 0;

    /**
     * Tells the parent controller to release the subcontroller.
     * @param aCallback - Function pointer to the UnsetSubController function of
     * the parent controller.
     */
    virtual void SetReleaseCallback(std::function<void()> aCallback) = 0;

    /**
     * Passes control to a subcontroller.
     * @param aController - Controller to set.
     */
    virtual void SetSubController(std::unique_ptr<IWindowController> aController) = 0;

    /**
     * Releases control to the subcontroller.
     */
    virtual void UnsetSubController() = 0;

protected:
    /**
     * Gets the subcontroller.
     * @return
     */
    virtual std::unique_ptr<IWindowController>& GetSubController() = 0;

    /**
     * Gets the NCurses windows attached to this controller.
     * @return reference to windows vector, can be empty.
     */
    virtual std::vector<std::shared_ptr<IWindow>>& GetWindows() = 0;

    /**
     * Checks if there is a subcontroller set, used in KeyAction.
     * @return true if there is a subcontroller set.
     */
    virtual WindowModel& GetWindowModel() = 0;

    /**
     * Gets height of the entire window as a reference.
     * @return height of the window.
     */
    virtual const int& GetHeightReference() = 0;

    /**
     * Gets width of the entire window as a reference.
     * @return width of the window.
     */
    virtual const int& GetWidthReference() = 0;

    /**
     * Checks if there is a subcontroller set, used in KeyAction.
     * @return true if there is a subcontroller set.
     */
    virtual bool HasSubControllerSet() = 0;

    /**
     * Sets height of the entire window. Use when obtaining size from for example the setup.
     * @param aHeigth - Height to set.
     */
    virtual void SetHeight(int aHeigth) = 0;

    /**
     * Sets width of the entire window. Use when obtaining size from for example the setup.
     * @param aWidth - Width to set.
     */
    virtual void SetWidth(int aWidth) = 0;
};
