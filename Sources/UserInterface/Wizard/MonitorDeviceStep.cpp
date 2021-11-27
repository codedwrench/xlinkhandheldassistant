#include "../../../Includes/UserInterface/Wizard/MonitorDeviceStep.h"

#include <cmath>
#include <utility>

#include "../../../Includes/UserInterface/Button.h"
#include "../../../Includes/UserInterface/CheckBox.h"
#include "../../../Includes/UserInterface/DefaultElements.h"
#include "../../../Includes/UserInterface/RadioBoxGroup.h"
#include "../../../Includes/UserInterface/String.h"
#include "../../../Includes/UserInterface/TextField.h"

/* Copyright (c) 2021 [Rick de Bondt] - MonitorDeviceStep.cpp */

namespace
{
    Dimensions ScaleAutoConnectCheckBox() { return {1, 4, 0, 0}; }
    Dimensions ScaleSearchPSPNetworks() { return {2, 4, 0, 0}; }
    Dimensions ScaleAcknowledgeDataFrames() { return {3, 4, 0, 0}; }
    Dimensions ScaleOnlyAcceptFromMac() { return {4, 4, 0, 0}; }
    Dimensions ScaleSetChannel() { return {5, 4, 0, 0}; }
    Dimensions ScaleUseWifiAdapterRadioBoxGroup() { return {6, 4, 0, 0}; }
}  // namespace

MonitorDeviceStep::MonitorDeviceStep(WindowModel&                aModel,
                                     std::string_view            aTitle,
                                     std::function<Dimensions()> aCalculation) :
    Window(aModel, aTitle, aCalculation)
{}

void MonitorDeviceStep::SetUp()
{
    Window::SetUp();

    // Get size of window so scaling works properly.
    GetSize();

    auto lAutoConnectCheckbox{std::make_shared<CheckBox>(*this,
                                                         "Automatically connect to PSP networks",
                                                         ScaleAutoConnectCheckBox,
                                                         GetModel().mAutoDiscoverPSPVitaNetworks)};

    AddObject(std::make_shared<CheckBox>(*this,
                                         "Automatically connect to PSP/Vita networks",
                                         ScaleSearchPSPNetworks,
                                         GetModel().mAutoDiscoverPSPVitaNetworks));

    AddObject(std::make_shared<CheckBox>(*this,
                                         "Acknowledge data packets (may break on some WiFi-cards)",
                                         ScaleAcknowledgeDataFrames,
                                         GetModel().mAcknowledgeDataFrames));

    AddObject(std::make_shared<TextField>(*this,
                                          "Only allow packets from the following Mac-address",
                                          ScaleOnlyAcceptFromMac,
                                          GetModel().mOnlyAcceptFromMac,
                                          17,
                                          true,
                                          true,
                                          std::vector<char>{':'}));

    AddObject(std::make_shared<TextField>(
        *this, "The channel to use", ScaleSetChannel, GetModel().mChannel, 2, true, true, std::vector<char>{}));

    auto lAdapterRadioBoxGroup{std::make_shared<RadioBoxGroup>(
        *this, "Use the following adapter:", ScaleUseWifiAdapterRadioBoxGroup, GetModel().mWifiAdapterSelection)};

    for (auto& lAdapter : GetModel().mWifiAdapterList) {
        lAdapterRadioBoxGroup->AddRadioBox(lAdapter.second);
    }

    AddObject(lAdapterRadioBoxGroup);
    AddObject(CreateNextButton(*this, GetModel(), GetHeightReference(), GetWidthReference()));
    AddObject(CreateWizardStepText(*this, 2, 3, GetWidthReference()));
    AddObject(CreateQuitText(*this, GetHeightReference()));
}

void MonitorDeviceStep::Draw()
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