#include <chrono>
#include <iostream>
#include <memory>
#include <string>

#include <boost/program_options.hpp>

#define PDC_WIDE
#include <curses.h>

#include "Includes/IPCapDevice.h"
#undef timeout

#include "Includes/Logger.h"
#include "Includes/MonitorDevice.h"
#include "Includes/NetConversionFunctions.h"
#include "Includes/UserInterface/KeyboardController.h"
#include "Includes/UserInterface/MainWindowController.h"
#include "Includes/WirelessPSPPluginDevice.h"
#include "Includes/WirelessPromiscuousDevice.h"
#include "Includes/XLinkKaiConnection.h"

namespace
{
    constexpr std::string_view cLogFileName{"log.txt"};
    constexpr bool             cLogToDisk{true};
    constexpr std::string_view cConfigFileName{"config.txt"};

    // Indicates if the program should be running or not, used to gracefully exit the program.
    bool gRunning{true};
}  // namespace

namespace po = boost::program_options;

// Add npcap to the dll path for windows
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
bool InitNPcapDLLPath()
{
    bool        lReturn{false};
    std::string lNPcapDirectory{};
    lNPcapDirectory.resize(MAX_PATH);
    unsigned int lLength = GetSystemDirectory(lNPcapDirectory.data(), MAX_PATH);
    lNPcapDirectory.resize(lLength);
    if (lLength > 0) {
        lNPcapDirectory.append("\\Npcap");
        if (SetDllDirectory(lNPcapDirectory.data()) != 0) {
            lReturn = true;
        }
    }
    return lReturn;
}
#endif

static void SignalHandler(const boost::system::error_code& aError, int aSignalNumber)
{
    if (!aError) {
        if (aSignalNumber == SIGINT || aSignalNumber == SIGTERM) {
            // Quit gracefully.
            gRunning = false;
        }
    }
}

int main(int argc, char* argv[])
{
    std::string lProgramPath{"./"};

#if not defined(_WIN32) && not defined(_WIN64)
    setlocale(LC_ALL, "");
    // Make robust against sudo path change.
    std::array<char, PATH_MAX> lResolvedPath{};
    if (realpath(argv[0], lResolvedPath.data()) != nullptr) {
        lProgramPath = std::string(lResolvedPath.begin(), lResolvedPath.end());

        // Remove excecutable name from path
        size_t lExcecutableNameIndex{lProgramPath.rfind('/')};
        if (lExcecutableNameIndex != std::string::npos) {
            lProgramPath.erase(lExcecutableNameIndex + 1, lProgramPath.length() - lExcecutableNameIndex - 1);
        }
    }
#else
    // Npcap needs this
    if (!InitNPcapDLLPath()) {
        // Quit the application almost immediately
        gRunning = false;
    }
#endif
    po::options_description lDescription("Options");
    // clang-format off
    lDescription.add_options()
        ("help,h", "Shows this help message.")
        ("verbose,v", "Disables HUD and shows log directly on screen.");
    // clang-format on
    po::variables_map lVariableMap;
    po::store(po::command_line_parser(argc, argv).options(lDescription).run(), lVariableMap);

    if ((lVariableMap.count("help") != 0U) || (lVariableMap.count("h") != 0U)) {
        std::cout << lDescription << std::endl;
    } else {
        po::notify(lVariableMap);

        bool                                  lContinue{true};
        std::vector<std::string>              lSSIDFilters{};
        std::shared_ptr<MainWindowController> lWindowController{nullptr};
        std::shared_ptr<KeyboardController>   lKeyboardController{nullptr};

        // Handle quit signals gracefully.
        boost::asio::io_service lSignalIoService{};
        boost::asio::signal_set lSignals(lSignalIoService, SIGINT, SIGTERM);
        lSignals.async_wait(&SignalHandler);
        std::thread lThread{[lIoService = &lSignalIoService] { lIoService->run(); }};

        WindowModel mWindowModel{};

        // Check if wizard can be skipped
        bool lSkipWizard{false};
        if (mWindowModel.LoadFromFile(lProgramPath + cConfigFileName.data())) {
            lSkipWizard = true;
        }

        mWindowModel.mProgramPath = lProgramPath;

        Logger::GetInstance().Init(mWindowModel.mLogLevel, cLogToDisk, lProgramPath + cLogFileName.data());

        if ((lVariableMap.count("verbose") != 0U) || (lVariableMap.count("v") != 0U)) {
            Logger::GetInstance().SetLogToScreen(true);
            if (lSkipWizard) {
                // Start the engine immediately
                mWindowModel.mCommand = WindowModel_Constants::Command::StartEngine;
            } else {
                Logger::GetInstance().Log("No config file found! First run the wizard before running in verbose mode",
                                          Logger::Level::ERROR);
                lContinue = false;
            }
        } else {
            lWindowController   = std::make_shared<MainWindowController>(mWindowModel, lSkipWizard);
            lKeyboardController = std::make_shared<KeyboardController>(
                [&](unsigned int aAction) { lWindowController->KeyAction(aAction); });

            if (lWindowController->SetUp()) {
                lKeyboardController->StartThread();

            } else {
                Logger::GetInstance().Log("Error initializing the TUI", Logger::Level::ERROR);
                // Stop the app, we have no TUI
                lContinue = false;
            }
        }

        if (lContinue) {
            std::shared_ptr<IPCapDevice>        lDevice{nullptr};
            std::shared_ptr<XLinkKaiConnection> lXLinkKaiConnection{std::make_shared<XLinkKaiConnection>()};

            bool lSuccess{false};

            // If we need more entry methods, make an actual state machine
            bool                                               lWaitEntry{true};
            std::chrono::time_point<std::chrono::system_clock> lWaitStart{std::chrono::seconds{0}};

            while (gRunning) {
                if (lWindowController == nullptr || lWindowController->Process()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    switch (mWindowModel.mCommand) {
                        case WindowModel_Constants::Command::StartEngine:
                            if (mWindowModel.mLogLevel != Logger::GetInstance().GetLogLevel()) {
                                Logger::GetInstance().SetLogLevel(mWindowModel.mLogLevel);
                            }

                            switch (mWindowModel.mConnectionMethod) {
                                case WindowModel_Constants::ConnectionMethod::Plugin:
                                    if (std::dynamic_pointer_cast<WirelessPSPPluginDevice>(lDevice) == nullptr) {
                                        std::chrono::seconds lTimeOut =
                                            std::chrono::seconds(std::stoi(mWindowModel.mReConnectionTimeOutS));
                                        lDevice = std::make_shared<WirelessPSPPluginDevice>(
                                            mWindowModel.mAutoDiscoverPSPVitaNetworks,
                                            lTimeOut,
                                            &mWindowModel.mCurrentlyConnectedNetwork);

                                        Logger::GetInstance().Log("Plugin Device created!", Logger::Level::INFO);
                                    }
                                    break;
                                case WindowModel_Constants::ConnectionMethod::Promiscuous:
                                    if (std::dynamic_pointer_cast<WirelessPromiscuousDevice>(lDevice) == nullptr) {
                                        std::chrono::seconds lTimeOut =
                                            std::chrono::seconds(std::stoi(mWindowModel.mReConnectionTimeOutS));
                                        lDevice = std::make_shared<WirelessPromiscuousDevice>(
                                            mWindowModel.mAutoDiscoverPSPVitaNetworks,
                                            lTimeOut,
                                            &mWindowModel.mCurrentlyConnectedNetwork);

                                        Logger::GetInstance().Log("Promiscuous Device created!", Logger::Level::INFO);
                                    }
                                    break;
                                case WindowModel_Constants::ConnectionMethod::Monitor:
                                    if (std::dynamic_pointer_cast<MonitorDevice>(lDevice) == nullptr) {
                                        lDevice = std::make_shared<MonitorDevice>(MacToInt(mWindowModel.mOnlyAcceptFromMac),
                                                                                  mWindowModel.mAcknowledgeDataFrames,
                                                                                  &mWindowModel.mCurrentlyConnectedNetwork);
                                        Logger::GetInstance().Log("Monitor Device created!", Logger::Level::INFO);
                                    }
                                default:
                                    Logger::GetInstance().Log("Unknown method!", Logger::Level::ERROR);
                                    gRunning = false;
                                    break;
                            }

                            lXLinkKaiConnection->SetIncomingConnection(lDevice);
                            lXLinkKaiConnection->SetUseHostSSID(mWindowModel.mUseSSIDFromHost);

                            lDevice->SetConnector(lXLinkKaiConnection);
                            lDevice->SetHosting(mWindowModel.mHosting);

                            // If we are auto discovering PSP/VITA networks add those to the filter list
                            if (mWindowModel.mAutoDiscoverPSPVitaNetworks) {
                                lSSIDFilters.emplace_back(Net_Constants::cPSPSSIDFilterName.data());
                                lSSIDFilters.emplace_back(Net_Constants::cVitaSSIDFilterName.data());
                            }

                            // Set the XLink Kai connection up, if we are autodiscovering we don't need to provide an IP
                            if (!mWindowModel.mAutoDiscoverXLinkKaiInstance) {
                                lSuccess = lXLinkKaiConnection->Open(mWindowModel.mXLinkIp,
                                                                     std::stoi(mWindowModel.mXLinkPort));
                            } else {
                                lSuccess = lXLinkKaiConnection->Open("");
                            }

                            // Now set up the wifi interface
                            if (lSuccess) {
                                if (lDevice->Open(mWindowModel.mWifiAdapter, lSSIDFilters)) {
                                    if (lDevice->StartReceiverThread() && lXLinkKaiConnection->StartReceiverThread()) {
                                        mWindowModel.mEngineStatus = WindowModel_Constants::EngineStatus::Running;
                                        mWindowModel.mCommand      = WindowModel_Constants::Command::NoCommand;
                                    } else {
                                        Logger::GetInstance().Log("Failed to start receiver threads",
                                                                  Logger::Level::ERROR);
                                        mWindowModel.mEngineStatus     = WindowModel_Constants::EngineStatus::Error;
                                        mWindowModel.mCommand          = WindowModel_Constants::Command::WaitForTime;
                                        mWindowModel.mTimeToWait       = std::chrono::seconds(5);
                                        mWindowModel.mCommandAfterWait = WindowModel_Constants::Command::StopEngine;
                                    }
                                } else {
                                    Logger::GetInstance().Log("Failed to activate monitor interface",
                                                              Logger::Level::ERROR);
                                    mWindowModel.mEngineStatus     = WindowModel_Constants::EngineStatus::Error;
                                    mWindowModel.mCommand          = WindowModel_Constants::Command::WaitForTime;
                                    mWindowModel.mTimeToWait       = std::chrono::seconds(5);
                                    mWindowModel.mCommandAfterWait = WindowModel_Constants::Command::StopEngine;
                                }
                            } else {
                                Logger::GetInstance().Log(
                                    "Failed to open connection to XLink Kai, retrying in 10 seconds!",
                                    Logger::Level::ERROR);
                                // Have it take some time between tries
                                mWindowModel.mCommand          = WindowModel_Constants::Command::WaitForTime;
                                mWindowModel.mTimeToWait       = std::chrono::seconds(10);
                                mWindowModel.mCommandAfterWait = WindowModel_Constants::Command::NoCommand;
                            }
                            break;
                        case WindowModel_Constants::Command::WaitForTime:
                            // Wait state, use this to add a delay without making the UI unresponsive.
                            if (lWaitEntry) {
                                lWaitStart = std::chrono::system_clock::now();
                                lWaitEntry = false;
                            }

                            if (std::chrono::system_clock::now() > lWaitStart + mWindowModel.mTimeToWait) {
                                mWindowModel.mCommand = mWindowModel.mCommandAfterWait;
                                lWaitEntry            = true;
                            }
                            break;
                        case WindowModel_Constants::Command::StopEngine:
                            lXLinkKaiConnection->Close();
                            lDevice->Close();
                            lSSIDFilters.clear();

                            // Let's actually just remove the device, easier this way
                            lDevice = nullptr;

                            mWindowModel.mEngineStatus = WindowModel_Constants::EngineStatus::Idle;
                            mWindowModel.mCommand      = WindowModel_Constants::Command::NoCommand;
                            break;
                        case WindowModel_Constants::Command::StartSearchNetworks:
                        case WindowModel_Constants::Command::StopSearchNetworks:
                            // TODO: implement.
                            break;
                        case WindowModel_Constants::Command::ReConnect:
                            if (lDevice != nullptr) {
                                lDevice->Connect("");
                            }

                            mWindowModel.mCommand = WindowModel_Constants::Command::NoCommand;
                            break;
                        case WindowModel_Constants::Command::SetHosting:
                            if (lDevice != nullptr) {
                                lDevice->SetHosting(mWindowModel.mHosting);
                            }
                            if (lXLinkKaiConnection != nullptr) {
                                lXLinkKaiConnection->SetHosting(mWindowModel.mHosting);
                            }
                            break;
                        case WindowModel_Constants::Command::NoCommand:
                            break;
                    }
                } else {
                    gRunning = false;
                }
            }

            mWindowModel.mEngineStatus = WindowModel_Constants::EngineStatus::Idle;
            mWindowModel.mCommand      = WindowModel_Constants::Command::NoCommand;

            if (lDevice != nullptr) {
                lDevice->Close();
            }

            lXLinkKaiConnection->Close();
            lSSIDFilters.clear();

            lDevice             = nullptr;
            lXLinkKaiConnection = nullptr;
        } else {
            gRunning = false;
        }

        if (lKeyboardController != nullptr) {
            lKeyboardController->StopThread();
            lKeyboardController = nullptr;
        }

        lWindowController = nullptr;

        lSignalIoService.stop();
        if (lThread.joinable()) {
            lThread.join();
        }
    }
}
