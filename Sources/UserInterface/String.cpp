/* Copyright (c) 2020 [Rick de Bondt] - String.cpp */

#include "UserInterface/String.h"

String::String(IWindow&                    aWindow,
               std::string_view            aName,
               std::function<Dimensions()> aCalculation,
               bool                        aVisible,
               bool                        aSelectable,
               int                         aColorPair) :
    UIObject(aWindow, aName, std::move(aCalculation), aVisible, aSelectable),
    mColorPair{aColorPair}
{}

void String::Draw()
{
    GetWindow().DrawString(GetYCoord(), GetXCoord(), mColorPair, GetName());
}

void String::SetColorPair(int aPair)
{
    mColorPair = aPair;
}