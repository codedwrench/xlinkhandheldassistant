/* Copyright (c) 2020 [Rick de Bondt] - WindowControllerBase.cpp **/

#include "UserInterface/WindowControllerBase.h"

WindowControllerBase::WindowControllerBase(WindowModel& aWindowModel) : mWindowModel(aWindowModel) {}

const int& WindowControllerBase::GetHeightReference()
{
    return mHeight;
}

const int& WindowControllerBase::GetWidthReference()
{
    return mWidth;
}

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

bool WindowControllerBase::SetUp()
{
    int lHeight{0};
    int lWidth{0};
    getmaxyx(stdscr, lHeight, lWidth);
    mHeight = lHeight;
    mWidth  = lWidth;

    return true;
}

bool WindowControllerBase::KeyAction(unsigned int aAction)
{
    bool lReturn{true};

    if (aAction != ERR && mSubController == nullptr) {
        if (aAction == 'q') {
            GetWindowModel().mStopProgram = true;
        } else if (aAction == KEY_RESIZE) {
            mDimensionsChanged = true;
        } else {
            // For now send the key action to all visible windows
            for (auto& lWindow : mWindows) {
                if (lWindow->IsVisible()) {
                    lWindow->HandleKey(aAction);
                }
            }
        }
    } else if (mSubController != nullptr) {
        lReturn = mSubController->KeyAction(aAction);
    }

    return lReturn;
}

bool WindowControllerBase::Process()
{
    bool lReturn{true};

    if (!mWindowModel.mStopProgram) {
        if (mSubController == nullptr) {
            int lHeight{0};
            int lWidth{0};
            getmaxyx(stdscr, lHeight, lWidth);

            if ((lHeight != mHeight) || (lWidth != mWidth)) {
                mDimensionsChanged = true;
                mHeight            = lHeight;
                mWidth             = lWidth;
            }

#if defined(_WIN32) || defined(_WIN64)
            if (is_termresized()) {
                mDimensionsChanged = true;
            }

            if (mDimensionsChanged) {
                resize_term(0, 0);
            }
#endif

            for (auto& lWindow : mWindows) {
                if (lWindow->IsVisible()) {
                    if (mDimensionsChanged) {
                        lWindow->Scale();
                    } else {
                        lWindow->Draw();
                    }
                }
            }

            refresh();
            curs_set(0);
        }

        if (GetSubController() != nullptr) {
            lReturn = GetSubController()->Process();
        }

        mDimensionsChanged = false;
    } else {
        // Save any configuration that may have changed
        GetWindowModel().SaveToFile(GetWindowModel().mProgramPath + "config.txt");
        lReturn = false;
    }

    return lReturn;
}

void WindowControllerBase::SetHeight(int aHeight)
{
    mHeight = aHeight;
}

void WindowControllerBase::SetWidth(int aWidth)
{
    mWidth = aWidth;
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
