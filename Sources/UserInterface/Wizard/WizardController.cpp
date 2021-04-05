#include "../../../Includes/UserInterface/Wizard/WizardController.h"

#include <iostream>

#include <curses.h>
#include <pcap.h>

#include "../../../Includes/Logger.h"
#include "../../../Includes/UserInterface/Wizard/MonitorDeviceStep.h"
#include "../../../Includes/UserInterface/Wizard/PluginOptionsStep.h"
#include "../../../Includes/UserInterface/Wizard/WizardSelectorStep.h"
#include "../../../Includes/UserInterface/Wizard/XLinkOptionsStep.h"

#undef MOUSE_MOVED

/* Copyright (c) 2021 [Rick de Bondt] - WizardController.cpp */

using namespace WizardController_Constants;

Dimensions ScaleWizard()
{
    Dimensions lDimensions{};
    getmaxyx(stdscr, lDimensions.at(2), lDimensions.at(3));

    return lDimensions;
}

template<class WindowType>
void ReplaceWindow(std::vector<std::shared_ptr<IWindow>>& aWindows, WindowModel& aModel, std::string_view aTitle)
{
    if (!aWindows.empty()) {
        aWindows.pop_back();
    }

    aWindows.emplace_back(std::make_shared<WindowType>(aModel, aTitle, ScaleWizard));

    // Run setup on the window,
    aWindows.back()->SetUp();
}

WizardController::WizardController(WindowModel& aModel) : WindowControllerBase(aModel) {}

// Windows does not do well with the standard pcap approach
#if defined(_WIN32) || defined(_WIN64)
#include <Winsock2.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>

// Link with Iphlpapi.lib
#pragma comment(lib, "IPHLPAPI.lib")

static void FillWifiAdapters(std::vector<std::pair<std::string, std::string>>& aWifiAdapterList)
{
    ULONG lBuffer = sizeof(IP_ADAPTER_ADDRESSES);

    // For some reason this has to be called twice, Windows shenanigans
    GetAdaptersAddresses(0, 0, nullptr, nullptr, &lBuffer);
    std::vector<uint8_t>  lBytes(lBuffer, 0);
    PIP_ADAPTER_ADDRESSES lCurrentAddresses = (IP_ADAPTER_ADDRESSES*) lBytes.data();

    DWORD lReturn = GetAdaptersAddresses(0, 0, nullptr, lCurrentAddresses, &lBuffer);

    if (lReturn == NO_ERROR) {
        while (lCurrentAddresses != nullptr) {
            // Check if the device is a wifi (802.11) device
            if (lCurrentAddresses->IfType == IF_TYPE_IEEE80211) {
                std::string lFriendlyName{lCurrentAddresses->FriendlyName,
                                          lCurrentAddresses->FriendlyName + wcslen(lCurrentAddresses->FriendlyName)};

                // Try to skip virtual adapters
                if (lFriendlyName.rfind("Virtual", 0) != 0) {
                    std::pair<std::string, std::string> lWifiInformation{};
                    // Without \Device\NPF_ in front of this, it doesn't work
                    lWifiInformation.first  = "\\Device\\NPF_" + std::string(lCurrentAddresses->AdapterName,
                                                                          lCurrentAddresses->AdapterName +
                                                                              strlen(lCurrentAddresses->AdapterName));
                    lWifiInformation.second = lFriendlyName;
                    aWifiAdapterList.push_back(lWifiInformation);
                }
            }
            lCurrentAddresses = lCurrentAddresses->Next;
        }
    } else {
        // Lazy way to show an error, I know.
        std::pair<std::string, std::string> lWifiInformation{};
        lWifiInformation.first  = "wlan0";
        lWifiInformation.second = "ERROR: GetAdaptersAddresses: " + std::to_string(lReturn);
        aWifiAdapterList.push_back(lWifiInformation);
    }

    free(lCurrentAddresses);
    lCurrentAddresses = nullptr;
}
#else
static void FillWifiAdapters(std::vector<std::pair<std::string, std::string>>& aWifiAdapterList)
{
    pcap_if_t*                         lDevices{nullptr};
    std::array<char, PCAP_ERRBUF_SIZE> lErrorBuffer{};

    if (pcap_findalldevs(&lDevices, lErrorBuffer.data()) != -1) {
        aWifiAdapterList.clear();
        for (pcap_if_t* lDevice = lDevices; lDevice != nullptr; lDevice = lDevice->next) {
            // Only show wifi adapters that are wireless and up
            int lMask = PCAP_IF_WIRELESS;
            if ((lDevice->flags & lMask) == lMask) {
                pcap_t* lHandle{pcap_create(lDevice->name, lErrorBuffer.data())};
                if (lHandle != nullptr) {
                    // Device has to be 802.11 as well, so no bluetooth and the like
                    int lError    = pcap_activate(lHandle);
                    int lLinkType = pcap_datalink(lHandle);
                    // It seems to be EN10MB when the network is down on Linux :/
                    if (lError == 0 && lLinkType == DLT_IEEE802_11 || lLinkType == DLT_EN10MB) {
                        std::pair<std::string, std::string> lWifiInformation{};
                        lWifiInformation.first = lDevice->name;
                        if (lDevice->description) {
                            lWifiInformation.second = lDevice->description;
                        } else {
                            // We have no better information than this
                            lWifiInformation.second = lDevice->name;
                        }
                        aWifiAdapterList.push_back(lWifiInformation);
                    } else if (lError == PCAP_ERROR_PERM_DENIED) {
                        // Lazy way to show an error, I know.
                        std::pair<std::string, std::string> lWifiInformation{};
                        lWifiInformation.first  = "wlan0";
                        lWifiInformation.second = "ERROR: Permission denied, try running as root/administrator!";
                        aWifiAdapterList.push_back(lWifiInformation);
                        break;
                    }

                    pcap_close(lHandle);
                }
            }
        }
        pcap_freealldevs(lDevices);
    }
}

#endif

void WizardController::HandleConnectionMethod()
{
    switch (GetWindowModel().mConnectionMethod) {
        case WindowModel_Constants::ConnectionMethod::Plugin:
            mWizardStep = PluginOptions;

            // We are going to fill the WiFi-adapter list now, since we need it here
            FillWifiAdapters(GetWindowModel().mWifiAdapterList);

            // Add the Plugin wizard step
            ReplaceWindow<PluginOptionsStep>(GetWindows(), GetWindowModel(), "Plugin options");

            break;
#if not defined(_WIN32) && not defined(_WIN64)
        case WindowModel_Constants::ConnectionMethod::Monitor:
            mWizardStep = MonitorOptions;

            // We are going to fill the WiFi-adapter list now, since we need it here
            FillWifiAdapters(GetWindowModel().mWifiAdapterList);

            // Add the Monitor wizard step
            ReplaceWindow<MonitorDeviceStep>(GetWindows(), GetWindowModel(), "Monitor options");

            break;
#endif
        case WindowModel_Constants::ConnectionMethod::Simulation:
            mWizardStep = SimulationOptions;

            // Add the Simulation wizard step
            ReplaceWindow<WizardSelectorStep>(GetWindows(), GetWindowModel(), "Simulation options");

            break;
        case WindowModel_Constants::ConnectionMethod::USB:
            mWizardStep = USBOptions;

            // Add the USB wizard step
            ReplaceWindow<WizardSelectorStep>(GetWindows(), GetWindowModel(), "USB options");

            break;
        default:
            // Do nothing
            break;
    }
}

bool WizardController::Process()
{
    bool lReturn{WindowControllerBase::Process()};

    if (GetWindowModel().mWindowDone) {
        // Clear the command
        GetWindowModel().mWindowDone = false;

        // If we are in the selection menu, we should check what the next step is we should show
        switch (mWizardStep) {
            case Selector:
                HandleConnectionMethod();
                break;
            case MonitorOptions:
                // Convert the wifi adapter selection to a string
                GetWindowModel().mWifiAdapter =
                    GetWindowModel().mWifiAdapterList.at(GetWindowModel().mWifiAdapterSelection).first;

                mWizardStep = XLinkKaiOptions;
                ReplaceWindow<XLinkOptionsStep>(GetWindows(), GetWindowModel(), "XLink Kai options");
                break;
            case PluginOptions:
                // Convert the wifi adapter selection to a string
                GetWindowModel().mWifiAdapter =
                    GetWindowModel().mWifiAdapterList.at(GetWindowModel().mWifiAdapterSelection).first;

                mWizardStep = XLinkKaiOptions;
                ReplaceWindow<XLinkOptionsStep>(GetWindows(), GetWindowModel(), "XLink Kai options");
                break;
            case SimulationOptions:
                break;
            case USBOptions:
                break;
            case XLinkKaiOptions:
                // Done processing
                lReturn = false;
                break;
            default:
                break;
        }
    }

    return lReturn;
}

bool WizardController::SetUp()
{
    // Add the setup wizard
    ReplaceWindow<WizardSelectorStep>(GetWindows(), GetWindowModel(), "Set-Up Wizard");
    return true;
}
