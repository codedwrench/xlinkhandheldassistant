#include "../Includes/UserInterface/WindowControllerBase.h"

/* Copyright (c) 2020 [Rick de Bondt] - WindowControllerBase.cpp **/

WindowControllerBase::WindowControllerBase(WindowModel& aWindowModel) : mWindowModel(aWindowModel) {}

std::vector<std::shared_ptr<IWindow>>& WindowControllerBase::GetWindows()
{
    return mWindows;
}

WindowModel& WindowControllerBase::GetWindowModel()
{
    return mWindowModel;
}

std::unique_ptr<IWindowController>& WindowControllerBase::GetSubController()
{
    return mSubController;
}

bool WindowControllerBase::KeyAction(unsigned int aAction)
{
    bool lReturn{true};

    if (aAction != ERR) {
        if (aAction == 'q') {
            GetWindowModel().mStopProgram = true;
        } else if (mSubController != nullptr) {
            lReturn = mSubController->KeyAction(aAction);
        } else {
            // For now send the key action to all visible windows
            for (auto& lWindow : mWindows) {
                if (lWindow->IsVisible()) {
                    lWindow->HandleKey(aAction);
                }
            }
        }
    }

    return lReturn;
}

bool WindowControllerBase::Process()
{
    bool lReturn{true};

    if (!mWindowModel.mStopProgram) {
        bool lDimensionsChanged{false};
        int  lHeight{0};
        int  lWidth{0};
        getmaxyx(stdscr, lHeight, lWidth);

        if ((lHeight != mHeight) || (lWidth != mWidth)) {
            mHeight            = lHeight;
            mWidth             = lWidth;
            lDimensionsChanged = true;
        }

        for (auto& lWindow : mWindows) {
            if (lWindow->IsVisible()) {
                if (lDimensionsChanged) {
                    lWindow->Scale();
                }
                lWindow->Draw();
            }
        }

        if (GetSubController() != nullptr) {
            lReturn = GetSubController()->Process();
        }

        curs_set(0);
    } else {
        lReturn = false;
    }

    return lReturn;
}

void WindowControllerBase::SetReleaseCallback(std::function<void()> aCallback)
{
    mReleaseCallback = aCallback;
}

void WindowControllerBase::SetSubController(std::unique_ptr<IWindowController> aController)
{
    mSubController = std::move(aController);
}

void WindowControllerBase::UnsetSubController()
{
    mSubController = nullptr;
}

bool WindowControllerBase::HasSubControllerSet()
{
    return mSubController != nullptr;
}
