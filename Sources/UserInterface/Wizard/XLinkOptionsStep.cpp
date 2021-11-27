/* Copyright (c) 2020 [Rick de Bondt] - MonitorDeviceStep.cpp */

#include "../../../Includes/UserInterface/Wizard/XLinkOptionsStep.h"

#include <cmath>
#include <utility>

#include "../../../Includes/UserInterface/Button.h"
#include "../../../Includes/UserInterface/CheckBox.h"
#include "../../../Includes/UserInterface/DefaultElements.h"
#include "../../../Includes/UserInterface/String.h"
#include "../../../Includes/UserInterface/TextField.h"


namespace
{
    Dimensions ScaleIpAddressTextField(const int& /*aMaxHeight*/, const int& /*aMaxWidth*/) { return {2, 2, 0, 0}; }
    Dimensions ScalePortTextField(const int& /*aMaxHeight*/, const int& /*aMaxWidth*/) { return {3, 2, 0, 0}; }
}  // namespace

XLinkOptionsStep::XLinkOptionsStep(WindowModel&                aModel,
                                   std::string_view            aTitle,
                                   std::function<Dimensions()> aCalculation) :
    Window(aModel, aTitle, aCalculation)
{}

void XLinkOptionsStep::SetUp()
{
    Window::SetUp();

    // Get size of window so scaling works properly.
    GetSize();

    AddObject(std::make_shared<TextField>(
        *this,
        cIPAddressMessage,
        [&] { return ScaleIpAddressTextField(GetHeightReference(), GetWidthReference()); },
        GetModel().mXLinkIp,
        15,
        true,
        false,
        std::vector<char>{'.'}));

    AddObject(std::make_shared<TextField>(
        *this,
        cPortMessage,
        [&] { return ScalePortTextField(GetHeightReference(), GetWidthReference()); },
        GetModel().mXLinkPort,
        5,
        true,
        false,
        std::vector<char>{}));

    AddObject(CreateNextButton(*this, GetModel(), GetHeightReference(), GetWidthReference()));
    AddObject(CreateWizardStepText(*this, 3, 3, GetWidthReference()));
    AddObject(CreateQuitText(*this, GetHeightReference()));
}

void XLinkOptionsStep::Draw()
{
    // TODO: Add hiding checkbox object, which allows hiding groups of objects
    // Autodiscover checkbox
    //    if
    //    (std::dynamic_pointer_cast<CheckBox>(GetObjects().at(0))->IsChecked()) {
    //        // IP Field
    //        GetObjects().at(1)->SetVisible(false);
    //
    //        // Port Field
    //        GetObjects().at(2)->SetVisible(false);
    //    } else {
    GetObjects().at(1)->SetVisible(true);
    GetObjects().at(2)->SetVisible(true);
    //    }

    Window::Draw();
}