#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - WindowControllerBase.h
 *
 * This file contains the base functions for a windowcontroller.
 *
 **/

#include <memory>

#include "../WindowModel.h"
#include "IWindowController.h"

class WindowControllerBase : public IWindowController
{
public:
    WindowControllerBase(WindowModel& aWindowModel);
    bool KeyAction(unsigned int aAction) override;
    bool Process() override;
    void SetReleaseCallback(std::function<void()> aCallback) override;
    void SetSubController(std::unique_ptr<IWindowController> aController) override;
    void UnsetSubController() override;

protected:
    std::unique_ptr<IWindowController>&    GetSubController() override;
    std::vector<std::shared_ptr<IWindow>>& GetWindows() override;
    WindowModel&                           GetWindowModel() override;
    bool                                   HasSubControllerSet() override;

private:
    int                                   mHeight{};
    int                                   mWidth{};
    std::vector<std::shared_ptr<IWindow>> mWindows{};
    std::function<void()>                 mReleaseCallback{nullptr};
    std::unique_ptr<IWindowController>    mSubController{nullptr};
    WindowModel&                          mWindowModel;
};