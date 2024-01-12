/* Copyright (c) 2021 [Rick de Bondt] - WifiInterfaceWindows.cpp */

#include "WifiInterfaceWindows.h"

#include <codecvt>
#include <cstdio>
#include <cstdlib>
#include <locale>
#include <string>

#include <Winsock2.h>
#include <iphlpapi.h>
#include <objbase.h>
#include <wlanapi.h>
#include <wtypes.h>

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _MSC_VER
#define STDCALL __attribute__((stdcall))
#else
#define STDCALL __stdcall
#endif

// available network flags
#define WLAN_AVAILABLE_NETWORK_CONNECTED   0x00000001  // This network is currently connected
#define WLAN_AVAILABLE_NETWORK_HAS_PROFILE 0x00000002  // There is a profile for this network
#define WLAN_AVAILABLE_NETWORK_CONSOLE_USER_PROFILE                                                                    \
    0x00000004  // The profile is the active console user's per user profile

// Not sure why, but it couldn't find this on the mingw compiler
#define L2_NOTIFICATION_SOURCE_ALL 0X0000FFFF

// Context to pass along with callbacks
using WLAN_CALLBACK_INFO = struct _WLAN_CALLBACK_INFO
{
    GUID   interfaceGUID;
    HANDLE scanEvent;
    DWORD  callbackReason;
};

// Need to link with Wlanapi.lib and Ole32.lib
#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "IPHLPAPI.lib")

#include "Logger.h"
#include "NetConversionFunctions.h"

WifiInterface::WifiInterface(std::string_view aAdapter)
{
    std::string lAdapter{aAdapter};
    size_t      lIndex{lAdapter.find("NPF")};

    // Remove NPF part
    if (lIndex != std::string::npos) {
        lIndex = lAdapter.find_first_of('{', lIndex);
        if (lIndex != std::string::npos) {
            lAdapter = lAdapter.substr(lIndex);
        } else {
            lIndex = 0;
        }
    }

    // Uppercase the string
    std::transform(lAdapter.begin(), lAdapter.end(), lAdapter.begin(), ::toupper);
    mAdapterName = lAdapter;

    GUID lAdapterGUID{};

    if (CLSIDFromString(std::wstring(lAdapter.begin(), lAdapter.end()).c_str(), &lAdapterGUID) == NOERROR) {
        mGUID = lAdapterGUID;
    }
}

WifiInterface::~WifiInterface()
{
    if (mWifiHandle != nullptr) {
        WlanCloseHandle(mWifiHandle, nullptr);
    }
}

static std::wstring GenerateXML(const std::wstring& aSSID)
{
    std::wstring lFirst{L"<?xml version=\"1.0\"?>"
                        "<WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\">"
                        "<name>PSPAdhocNetwork</name>"
                        "<SSIDConfig>"
                        "<SSID>"
                        "<name>"};

    std::wstring lSecond{L"</name>"
                         "</SSID>"
                         "<nonBroadcast>false</nonBroadcast>"
                         "</SSIDConfig>"
                         "<connectionType>IBSS</connectionType>"
                         "<connectionMode>manual</connectionMode>"
                         "<MSM>"
                         "<security>"
                         "<authEncryption>"
                         "<authentication>open</authentication>"
                         "<encryption>none</encryption>"
                         "<useOneX>false</useOneX>"
                         "</authEncryption>"
                         "</security>"
                         "</MSM>"
                         "</WLANProfile>"};

    return lFirst + aSSID + lSecond;
}

static void STDCALL WlanCallback(WLAN_NOTIFICATION_DATA* aScanNotificationData, PVOID aContext)
{
    // Get the data from my struct. If it's null, nothing to do
    auto* lCallbackInfo = static_cast<WLAN_CALLBACK_INFO*>(aContext);
    if (lCallbackInfo != nullptr) {
        // Check the GUID in the struct against the GUID in the notification data, return if they don't match
        if (memcmp(&lCallbackInfo->interfaceGUID, &aScanNotificationData->InterfaceGuid, sizeof(GUID)) == 0) {
            // If the notification was for a scan complete or failure then we need to set the event
            if ((aScanNotificationData->NotificationCode == wlan_notification_acm_scan_complete) ||
                (aScanNotificationData->NotificationCode == wlan_notification_acm_scan_fail)) {
                // Set the notification code as the callbackReason
                lCallbackInfo->callbackReason = aScanNotificationData->NotificationCode;

                // Set the event
                SetEvent(lCallbackInfo->scanEvent);
            }
        }
    } else {
        Logger::GetInstance().Log("No callback info received", Logger::Level::ERROR);
    }
}

std::vector<IWifiInterface::WifiInformation>& WifiInterface::GetAdhocNetworks()
{
    // Declare and initialize variables.
    std::vector<std::string> lReturn{};
    unsigned long            lHighestClientVersion{2};
    unsigned long            lNegotiatedVersion{0};
    DWORD                    lResult{0};

    PWLAN_AVAILABLE_NETWORK_LIST lNetworkList{nullptr};
    PWLAN_AVAILABLE_NETWORK      lNetworkInformation{nullptr};

    if (mWifiHandle == nullptr) {
        lResult = WlanOpenHandle(lHighestClientVersion, nullptr, &lNegotiatedVersion, &mWifiHandle);
    } else {
        lResult = ERROR_SUCCESS;
    }

    // Declare the callback parameter struct
    WLAN_CALLBACK_INFO lCallbackInfo = {0};
    lCallbackInfo.interfaceGUID      = mGUID;

    // Create an event to be triggered in the scan case
    lCallbackInfo.scanEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    // Register for wlan scan notifications
    WlanRegisterNotification(mWifiHandle,
                             WLAN_NOTIFICATION_SOURCE_ALL,
                             1,
                             reinterpret_cast<WLAN_NOTIFICATION_CALLBACK>(WlanCallback),
                             reinterpret_cast<PVOID>(&lCallbackInfo),
                             nullptr,
                             nullptr);


    // Start a scan. If the WlanScan call fails, log the error
    lResult = WlanScan(mWifiHandle, &mGUID, nullptr, nullptr, nullptr);
    if (lResult == ERROR_SUCCESS) {
        // Clear last scan list
        mLastReceivedScanInformation.clear();

        // Scan request successfully sent
        Logger::GetInstance().Log("Scan request sent. Waiting for reply", Logger::Level::TRACE);

        // Wait for the event to be signaled, or an error to occur. Don't wait longer than 15 seconds.
        DWORD lWaitResult = WaitForSingleObject(lCallbackInfo.scanEvent, 15000);

        // Check how we got here, via callback or timeout
        if (lWaitResult == WAIT_OBJECT_0) {
            if (lCallbackInfo.callbackReason == wlan_notification_acm_scan_complete) {
                Logger::GetInstance().Log("The scan for networks is completed", Logger::Level::TRACE);
            } else if (lCallbackInfo.callbackReason == wlan_notification_acm_scan_fail) {
                Logger::GetInstance().Log("The scan for connectable networks failed", Logger::Level::ERROR);
            }
        } else if (lWaitResult == WAIT_TIMEOUT) {
            Logger::GetInstance().Log("No response was received after 15 seconds", Logger::Level::ERROR);
        } else {
            Logger::GetInstance().Log("Unknown error waiting for response. Error Code: " + std::to_string(lWaitResult),
                                      Logger::Level::ERROR);
        }


        if (lResult == ERROR_SUCCESS) {
            lResult = WlanGetAvailableNetworkList(mWifiHandle, &mGUID, 0, nullptr, &lNetworkList);
            if (lResult == ERROR_SUCCESS) {
                Logger::GetInstance().Log("Amount of Networks: " + std::to_string(lNetworkList->dwNumberOfItems),
                                          Logger::Level::TRACE);

                for (int lCount = 0; lCount < lNetworkList->dwNumberOfItems; lCount++) {
                    lNetworkInformation = &lNetworkList->Network[lCount];

                    if (lNetworkInformation->dot11Ssid.uSSIDLength != 0) {
                        std::string lSSID{reinterpret_cast<char*>(lNetworkInformation->dot11Ssid.ucSSID),
                                          lNetworkInformation->dot11Ssid.uSSIDLength};

                        WifiInformation lWifiInformation{};

                        Logger::GetInstance().Log("SSID: " + lSSID, Logger::Level::TRACE);

                        lWifiInformation.ssid = lSSID;
                        lWifiInformation.isconnected =
                            ((static_cast<unsigned int>(lNetworkInformation->dwFlags) &
                              static_cast<unsigned int>(WLAN_AVAILABLE_NETWORK_CONNECTED)) != 0u);

                        if (lNetworkInformation->dot11BssType == dot11_BSS_type_independent) {
                            Logger::GetInstance().Log("Is Ad-Hoc network!", Logger::Level::TRACE);
                            lWifiInformation.isadhoc = true;
                        }

                        Logger::GetInstance().Log(
                            "Amount of BSSIDs: " + std::to_string(lNetworkInformation->uNumberOfBssids),
                            Logger::Level::TRACE);

                        // Now get the BSSID belonging to the network, or atleast the first one
                        PWLAN_BSS_LIST lWlanBSSList = nullptr;
                        DWORD          lResult{WlanGetNetworkBssList(mWifiHandle,
                                                            &mGUID,
                                                            &lNetworkInformation->dot11Ssid,
                                                            lNetworkInformation->dot11BssType,
                                                            lNetworkInformation->bSecurityEnabled,
                                                            nullptr,
                                                            &lWlanBSSList)};
                        if (lResult == ERROR_SUCCESS && lWlanBSSList->dwNumberOfItems > 0) {
                            memcpy(lWifiInformation.bssid.data(), &lWlanBSSList->wlanBssEntries[0], 6);
                        }

                        mLastReceivedScanInformation.emplace_back(lWifiInformation);
                    }
                }
            } else {
                Logger::GetInstance().Log(
                    "Could not gather scan results:" + std::system_category().message(static_cast<int>(lResult)),
                    Logger::Level::ERROR);
            }
        } else {
            mWifiHandle = nullptr;
            Logger::GetInstance().Log(
                "Could not open WlanHandle" + std::system_category().message(static_cast<int>(lResult)),
                Logger::Level::ERROR);
        }

        if (lNetworkList != nullptr) {
            WlanFreeMemory(lNetworkList);
            lNetworkList = nullptr;
        }
    } else {
        Logger::GetInstance().Log(
            "Could not send Scan request" + std::system_category().message(static_cast<int>(lResult)),
            Logger::Level::ERROR);
    }
    return mLastReceivedScanInformation;
}

uint64_t WifiInterface::GetAdapterMacAddress()
{
    uint64_t lReturn{0};

    ULONG lBuffer = sizeof(IP_ADAPTER_ADDRESSES);

    GetAdaptersAddresses(0, 0, nullptr, nullptr, &lBuffer);
    std::vector<uint8_t> lBytes(lBuffer, 0);
    auto*                lCurrentAddresses = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(lBytes.data());
    DWORD                lResult           = GetAdaptersAddresses(0, 0, nullptr, lCurrentAddresses, &lBuffer);
    if (lResult == NO_ERROR) {
        while (lCurrentAddresses != nullptr) {
            if (lCurrentAddresses->AdapterName == mAdapterName) {
                memcpy(&lReturn, &lCurrentAddresses->PhysicalAddress[0], Net_8023_Constants::cSourceAddressLength);
            }
            lCurrentAddresses = lCurrentAddresses->Next;
        }
    }
    return lReturn;
}

bool WifiInterface::Connect(const IWifiInterface::WifiInformation& aConnection)
{
    Logger::GetInstance().Log("Connecting to: " + aConnection.ssid, Logger::Level::TRACE);
    DWORD lReturn{};

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> lConverter;
    std::wstring                                           lProf = GenerateXML(lConverter.from_bytes(aConnection.ssid));
    Logger::GetInstance().Log("Generated XML: " + lConverter.to_bytes(lProf), Logger::Level::TRACE);
    DWORD dwReason = 0;
    // set profile
    DWORD dwError = WlanSetProfile(mWifiHandle,
                                   &mGUID,
                                   0,  // no flags for the profile
                                   lProf.c_str(),
                                   nullptr,  // use the default ACL
                                   TRUE,     // overwrite a profile if it already exists
                                   nullptr,  // reserved
                                   &dwReason);

    std::string lReasonString;

    // Obtain reason
    if (dwError != ERROR_SUCCESS) {
        std::array<WCHAR, 256> lReasonWchar{};
        if (WlanReasonCodeToString(dwReason, lReasonWchar.size(), lReasonWchar.data(), NULL) == ERROR_SUCCESS) {
            std::wstring lReasonWString = std::wstring(lReasonWchar.data());
            lReasonString               = lConverter.to_bytes(lReasonWString);
        }
    }

    switch (dwError) {
        case ERROR_SUCCESS:
            // Everything is good, no error
            break;
        case ERROR_INVALID_PARAMETER:
            Logger::GetInstance().Log(std::string("The profile has invalid params: ") + lReasonString,
                                      Logger::Level::ERROR);
            break;
        case ERROR_BAD_PROFILE:
            Logger::GetInstance().Log(std::string("The profile is bad: ") + lReasonString, Logger::Level::ERROR);
            break;
        case ERROR_ALREADY_EXISTS:
            Logger::GetInstance().Log(std::string("The profile is already exists: ") + lReasonString,
                                      Logger::Level::ERROR);
            break;
        default:
            Logger::GetInstance().Log(
                std::string("Unknown error setting profile: ") + std::to_string(dwError) + " : " + lReasonString,
                Logger::Level::ERROR);
    }

    WLAN_CONNECTION_PARAMETERS mParameters{};

    if (aConnection.bssid.at(0) != 0 && aConnection.bssid.at(1) != 0) {
        mParameters.dwFlags = WLAN_CONNECTION_ADHOC_JOIN_ONLY;
    }

    mParameters.wlanConnectionMode = wlan_connection_mode_profile;
    mParameters.strProfile         = L"PSPAdhocNetwork";

    // Setup SSID
    DOT11_SSID lSSID{};
    memcpy(&lSSID.ucSSID[0], aConnection.ssid.data(), aConnection.ssid.size());
    lSSID.uSSIDLength = aConnection.ssid.size();

    mParameters.dot11BssType      = dot11_BSS_type_independent;
    mParameters.pDesiredBssidList = nullptr;
    mParameters.pDot11Ssid        = &lSSID;

    // Can't set a channel on windows ???!

    lReturn = WlanConnect(mWifiHandle, &mGUID, &mParameters, nullptr);
    return lReturn == ERROR_SUCCESS;
}


bool WifiInterface::LeaveIBSS()
{
    return WlanDisconnect(mWifiHandle, &mGUID, nullptr) == ERROR_SUCCESS;
}
