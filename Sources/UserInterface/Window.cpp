#include "../../Includes/UserInterface/Window.h"

#include <codecvt>
#include <locale>
#include <iostream>

#include "../../Includes/Logger.h"

/* Copyright (c) 2020 [Rick de Bondt] - Window.cpp */

Window::Window(WindowModel&                aModel,
               std::string_view            aTitle,
               std::function<Dimensions()> aCalculation,
               bool                        aDrawBorder,
               bool                        aExclusive,
               bool                        aVisible) :
    mModel{aModel},
    mTitle{aTitle}, mScaleCalculation(aCalculation), mNCursesWindow{nullptr}, mHeight{0}, mWidth{0},
    mDrawBorder(aDrawBorder), mExclusive{aExclusive}, mVisible{aVisible}, mSelectedObject{-1}, mObjects{}
{
    Dimensions lWindowParameters{aCalculation()};
    mHeight        = lWindowParameters.at(2);
    mWidth         = lWindowParameters.at(3);
    mNCursesWindow = NCursesWindow(
        newwin(lWindowParameters.at(2), lWindowParameters.at(3), lWindowParameters.at(0), lWindowParameters.at(1)),
        [](WINDOW* aWin) { delwin(aWin); });
}

void Window::SetUp() {}

bool Window::HandleKey(unsigned int aKeyCode)
{
    bool lReturn{false};

    switch (aKeyCode) {
        case KEY_UP:
            lReturn = RecedeSelectionVertical();
            break;
        case KEY_DOWN:
            lReturn = AdvanceSelectionVertical();
            break;
        case KEY_LEFT:
            lReturn = RecedeSelectionHorizontal();
            break;
        case KEY_RIGHT:
            lReturn = AdvanceSelectionHorizontal();
            break;
        default:
            if (mSelectedObject >= 0 && mSelectedObject < mObjects.size()) {
                lReturn = mObjects.at(mSelectedObject)->HandleKey(aKeyCode);
            }
    }

    return lReturn;
}

void Window::Draw()
{
    if (mDrawBorder) {
        box(mNCursesWindow.get(), 0, 0);
        DrawString(0, 0, 7, mTitle);
    }

    for (auto& lObject : mObjects) {
        if (lObject->IsVisible()) {
            lObject->Draw();
        }
    }

    Refresh();
}

void Window::ClearLine(int aYCoord, int aXCoord, int aLength)
{
    std::string lEmptySpace;
    lEmptySpace.resize(aLength, ' ');
    DrawString(aYCoord, aXCoord, 1, lEmptySpace);
}

void Window::DrawString(int aYCoord, int aXCoord, int aColorPair, std::string_view aString)
{
    wattrset(mNCursesWindow.get(), COLOR_PAIR(aColorPair));
    std::stringstream lStream{aString.data()};
    std::string       lLineToDraw{};
    int               lYCoord{aYCoord};
    while (std::getline(lStream, lLineToDraw, '\n')) {
#if defined(_WIN32) || defined(_WIN64)
        // conversion
        std::wstring lWidenedLineToDraw = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(lLineToDraw.data());
        mvwaddwstr(mNCursesWindow.get(), lYCoord, aXCoord, lWidenedLineToDraw.data());
#else
        mvwaddstr(mNCursesWindow.get(), lYCoord, aXCoord, lLineToDraw.data());
#endif
        wattrset(mNCursesWindow.get(), COLOR_PAIR(1));
        lYCoord++;
    }
}

void Window::AddObject(std::shared_ptr<IUIObject> aObject)
{
    mObjects.push_back(std::move(aObject));
}

void Window::Refresh()
{
    wrefresh(mNCursesWindow.get());
}

bool Window::Move(int aYCoord, int aXCoord)
{
    bool lReturn{true};

    if (mvwin(mNCursesWindow.get(), aYCoord, aXCoord) == ERR) {
        Logger::GetInstance().Log("Could not move window to desired spot", Logger::Level::TRACE);
        lReturn = false;
    }

    return lReturn;
}

WindowModel& Window::GetModel()
{
    return mModel;
}

std::pair<int, int> Window::GetSize()
{
    return {mHeight, mWidth};
}

bool Window::Scale()
{
    bool       lReturn{false};
    Dimensions lParameters{mScaleCalculation()};

    mHeight = lParameters.at(2);
    mWidth  = lParameters.at(3);

    if (Resize(mHeight, mWidth)) {
        if (Move(lParameters.at(0), lParameters.at(1))) {
            ClearWindow();
            lReturn = true;
        }

        for (auto& lObject : mObjects) {
            lObject->Scale();
        }
    }
    return lReturn;
}

bool Window::Resize(int aLines, int aColumns)
{
    bool lReturn{true};

    if (wresize(mNCursesWindow.get(), aLines, aColumns) == ERR) {
        Logger::GetInstance().Log("Could not resize window to desired size", Logger::Level::TRACE);
        lReturn = false;
    }

    return lReturn;
}

bool Window::SetSelection(int aSelection)
{
    bool lReturn{false};

    if (aSelection >= 0 && aSelection < mObjects.size()) {
        mObjects.at(mSelectedObject)->SetSelected(false);
        mSelectedObject = aSelection;
        mObjects.at(mSelectedObject)->SetSelected(true);
        lReturn = true;
    }

    return lReturn;
}

bool Window::AdvanceSelectionVertical()
{
    bool lReturn{false};
    int  lCounter{0};
    if (!mObjects.empty()) {
        if (mSelectedObject >= 0) {
            if (!mObjects.at(mSelectedObject)->HasDownAction()) {
                int lSelectionIndex{mObjects.at(mSelectedObject)->GetYCoord()};
                int lLowestHigherSelection{std::numeric_limits<int>::max()};
                int lLowestHigherSelectionYCoord{std::numeric_limits<int>::max()};

                for (auto& lObject : mObjects) {
                    int lYCoord{lObject->GetYCoord()};
                    if (lObject->IsVisible() && lObject->IsSelectable() && lYCoord > lSelectionIndex &&
                        lYCoord < lLowestHigherSelectionYCoord) {
                        lLowestHigherSelection       = lCounter;
                        lLowestHigherSelectionYCoord = mObjects.at(lLowestHigherSelection)->GetYCoord();
                    }
                    lCounter++;
                }

                if (lLowestHigherSelection != std::numeric_limits<int>::max()) {
                    mObjects.at(mSelectedObject)->SetSelected(false);
                    mObjects.at(lLowestHigherSelection)->SetSelected(true);
                    mSelectedObject = lLowestHigherSelection;
                    lReturn         = true;
                }
            } else {
                mObjects.at(mSelectedObject)->HandleKey(KEY_DOWN);
            }
        } else {
            mSelectedObject = 0;
            mObjects.at(0)->SetSelected(true);
            lReturn = true;
        }
    } else {
        lReturn = true;
    }

    return lReturn;
}

bool Window::RecedeSelectionVertical()
{
    bool lReturn{false};
    int  lCounter{0};
    if (!mObjects.empty()) {
        if (mSelectedObject >= 0) {
            if (!mObjects.at(mSelectedObject)->HasUpAction()) {
                int lSelectionIndex{mObjects.at(mSelectedObject)->GetYCoord()};
                int lHighestLowerSelection{-1};
                int lHighestLowerSelectionYCoord{-1};

                for (auto& lObject : mObjects) {
                    int lYCoord{lObject->GetYCoord()};
                    if (lObject->IsVisible() && lObject->IsSelectable() && lYCoord < lSelectionIndex &&
                        lYCoord > lHighestLowerSelectionYCoord) {
                        lHighestLowerSelection       = lCounter;
                        lHighestLowerSelectionYCoord = mObjects.at(lHighestLowerSelection)->GetYCoord();
                    }
                    lCounter++;
                }

                if (lHighestLowerSelection != -1) {
                    mObjects.at(mSelectedObject)->SetSelected(false);
                    mObjects.at(lHighestLowerSelection)->SetSelected(true);
                    mSelectedObject = lHighestLowerSelection;
                    lReturn         = true;
                }
            } else {
                mObjects.at(mSelectedObject)->HandleKey(KEY_UP);
            }
        } else {
            mSelectedObject = static_cast<int>(mObjects.size()) - 1;
            mObjects.at(mSelectedObject)->SetSelected(true);
            lReturn = true;
        }
    } else {
        lReturn = true;
    }

    return lReturn;
}

bool Window::AdvanceSelectionHorizontal()
{
    bool lReturn{false};
    int  lCounter{0};
    if (!mObjects.empty()) {
        if (mSelectedObject >= 0) {
            int lSelectionIndex{mObjects.at(mSelectedObject)->GetXCoord()};
            int lLowestHigherSelection{std::numeric_limits<int>::max()};
            int lLowestHigherSelectionXCoord{std::numeric_limits<int>::max()};
            int lCount = 0;

            for (auto& lObject : mObjects) {
                int lXCoord{lObject->GetXCoord()};
                if (lObject->IsVisible() && lObject->IsSelectable() && lXCoord > lSelectionIndex &&
                    lXCoord < lLowestHigherSelectionXCoord) {
                    lLowestHigherSelection       = lCounter;
                    lLowestHigherSelectionXCoord = mObjects.at(lLowestHigherSelection)->GetXCoord();
                    lCount++;
                }
                lCounter++;
            }

            if (lLowestHigherSelection != std::numeric_limits<int>::max()) {
                mObjects.at(mSelectedObject)->SetSelected(false);
                mObjects.at(lLowestHigherSelection)->SetSelected(true);
                mSelectedObject = lLowestHigherSelection;
                lReturn         = true;
            }
        } else {
            mSelectedObject = 0;
            mObjects.at(0)->SetSelected(true);
            lReturn = true;
        }
    }

    return lReturn;
}

bool Window::RecedeSelectionHorizontal()
{
    bool lReturn{false};
    int  lCounter{0};
    if (mSelectedObject >= 0) {
        int lSelectionIndex{mObjects.at(mSelectedObject)->GetXCoord()};
        int lHighestLowerSelection{-1};
        int lHighestLowerSelectionXCoord{-1};

        for (auto& lObject : mObjects) {
            int lXCoord{lObject->GetXCoord()};
            if (lObject->IsVisible() && lObject->IsSelectable() && lXCoord < lSelectionIndex &&
                lXCoord > lHighestLowerSelectionXCoord) {
                lHighestLowerSelection       = lCounter;
                lHighestLowerSelectionXCoord = mObjects.at(lHighestLowerSelection)->GetXCoord();
            }
            lCounter++;
        }

        if (lHighestLowerSelection != -1) {
            mObjects.at(mSelectedObject)->SetSelected(false);
            mObjects.at(lHighestLowerSelection)->SetSelected(true);
            mSelectedObject = lHighestLowerSelection;
            lReturn         = true;
        }
    } else {
        mSelectedObject = 0;
        mObjects.at(0)->SetSelected(true);
        lReturn = true;
    }

    return lReturn;
}

void Window::DeSelect()
{
    mSelectedObject = -1;
    for (auto& lObject : mObjects) {
        lObject->SetSelected(false);
    }
}

bool Window::IsExclusive()
{
    return mExclusive;
}

void Window::SetExclusive(bool aExclusive)
{
    mExclusive = aExclusive;
}

bool Window::IsVisible()
{
    return mVisible;
}

void Window::SetVisible(bool aVisible)
{
    mVisible = aVisible;
}

void Window::ClearWindow()
{
    std::pair<int, int> lSize{GetSize()};

    for (int lCount = 0; lCount < lSize.first; lCount++) {
        ClearLine(lCount, 0, lSize.second);
    }
}

const int& Window::GetHeightReference() const
{
    return mHeight;
}

const int& Window::GetWidthReference() const
{
    return mWidth;
}

ObjectList& Window::GetObjects()
{
    return mObjects;
}

int Window::GetSelectedObject() const
{
    return mSelectedObject;
}
