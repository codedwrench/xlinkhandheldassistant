#include "../Includes/WindowModel.h"

/* Copyright (c) 2020 [Rick de Bondt] - WindowModel.cpp */

using namespace WindowModel_Constants;
#include <iostream>
std::string BoolToString(bool aBool)
{
    return aBool ? "true" : "false";
}

static ConnectionMethod ConvertConnectionMethodText(std::string_view aLevel)
{
    ConnectionMethod lReturn{Plugin};

    for (std::size_t lCount = 0; lCount < cConnectionMethodTexts.size(); lCount++) {
        if (cConnectionMethodTexts.at(lCount).find(aLevel) != std::string::npos) {
            lReturn = static_cast<ConnectionMethod>(lCount);
        }
    }

    return lReturn;
}

static bool StringToBool(std::string_view aString)
{
    bool lReturn{false};

    if (aString == "true") {
        lReturn = true;
    }

    return lReturn;
}

bool WindowModel::SaveToFile(std::string_view aPath) const
{
    bool          lReturn{false};
    std::ofstream lFile;
    lFile.open(aPath.data());

    if (lFile.is_open() && lFile.good()) {
        lFile << cSaveAcknowledgeDataFrames << ": " << BoolToString(mAcknowledgeDataFrames) << std::endl;
        lFile << cSaveAutoDiscoverPSPVita << ": " << BoolToString(mAutoDiscoverPSPVitaNetworks) << std::endl;
        lFile << cSaveAutoDiscoverXLinkKai << ": " << BoolToString(mAutoDiscoverXLinkKaiInstance) << std::endl;
        lFile << cSaveChannel << ": \"" << mChannel << "\"" << std::endl;
        lFile << cSaveConnectionMethod << ": \"" << cConnectionMethodTexts.at(mConnectionMethod) << "\"" << std::endl;
        lFile << cSaveLogLevel << ": \"" << Logger::ConvertLogLevelToString(mLogLevel) << "\"" << std::endl;
        lFile << cSaveOnlyAcceptFromMac << ": \"" << mOnlyAcceptFromMac << "\"" << std::endl;
        lFile << cSaveReConnectionTimeOutS << ": \"" << mReConnectionTimeOutS << "\"" << std::endl;
        lFile << cSaveTheme << ": \"" << mTheme << "\"" << std::endl;
        lFile << cSaveUseSSIDFromXLinkKai << ": " << BoolToString(mUseSSIDFromXLinkKai) << std::endl;
        lFile << cSaveUseXLinkKaiHints << ": " << BoolToString(mUseXLinkKaiHints) << std::endl;
        lFile << cSaveWifiAdapter << ": \"" << mWifiAdapter << "\"" << std::endl;
        lFile << cSaveXLinkIp << ": \"" << mXLinkIp << "\"" << std::endl;
        lFile << cSaveXLinkPort << ": \"" << mXLinkPort << "\"" << std::endl;
        lFile.close();

        if (lFile.good()) {
            lFile.close();
            lReturn = true;
        } else {
            Logger::GetInstance().Log("Could not save config", Logger::Level::ERROR);
        }
    } else {
        Logger::GetInstance().Log(std::string("Could not open/create config file: ") + aPath.data(),
                                  Logger::Level::ERROR);
    }

    return lReturn;
}

bool WindowModel::LoadFromFile(std::string_view aPath)
{
    bool          lReturn{false};
    std::ifstream lFile;
    lFile.open(aPath.data());

    if (lFile.is_open() && lFile.good()) {
        bool        lContinue{true};
        std::string lLine;

        while (lContinue && !lFile.eof() && lFile.good()) {
            getline(lFile, lLine);
            if (!lFile.eof() && lFile.good()) {
                lLine.erase(remove_if(lLine.begin(), lLine.end(), isspace), lLine.end());
                size_t      lUntilDelimiter = lLine.find(':');
                std::string lOption{lLine.substr(0, lUntilDelimiter)};
                std::string lResult{lLine.substr(lUntilDelimiter + 1, lLine.size() - lUntilDelimiter - 1)};
                try {
                    if (!lResult.empty()) {
                        if (lOption == cSaveAcknowledgeDataFrames) {
                            mAcknowledgeDataFrames = StringToBool(lResult);
                        } else if (lOption == cSaveAutoDiscoverPSPVita) {
                            mAutoDiscoverPSPVitaNetworks = StringToBool(lResult);
                        } else if (lOption == cSaveAutoDiscoverXLinkKai) {
                            mAutoDiscoverXLinkKaiInstance = StringToBool(lResult);
                        } else if (lOption == cSaveChannel) {
                            mChannel = lResult.substr(1, lResult.size() - 2);
                        } else if (lOption == cSaveConnectionMethod) {
                            mConnectionMethod = ConvertConnectionMethodText(lResult.substr(1, lResult.size() - 2));
                        } else if (lOption == cSaveLogLevel) {
                            mLogLevel = Logger::ConvertLogLevelStringToLevel(lResult.substr(1, lResult.size() - 2));
                        } else if (lOption == cSaveOnlyAcceptFromMac) {
                            mOnlyAcceptFromMac = lResult.substr(1, lResult.size() - 2);
                        } else if (lOption == cSaveReConnectionTimeOutS) {
                            mReConnectionTimeOutS = lResult.substr(1, lResult.size() - 2);
                        } else if (lOption == cSaveTheme) {
                            mTheme = lResult.substr(1, lResult.size() - 2);
                        } else if (lOption == cSaveUseSSIDFromXLinkKai) {
                            mUseSSIDFromXLinkKai = StringToBool(lResult.substr(1, lResult.size() - 2));
                        } else if (lOption == cSaveUseXLinkKaiHints) {
                            mUseXLinkKaiHints = StringToBool(lResult);
                        } else if (lOption == cSaveWifiAdapter) {
                            mWifiAdapter = lResult.substr(1, lResult.size() - 2);
                        } else if (lOption == cSaveXLinkIp) {
                            mXLinkIp = lResult.substr(1, lResult.size() - 2);
                        } else if (lOption == cSaveXLinkPort) {
                            mXLinkPort = lResult.substr(1, lResult.size() - 2);
                        } else {
                            Logger::GetInstance().Log(std::string("Option:") + lOption + " unknown",
                                                      Logger::Level::DEBUG);
                        }
                    } else {
                        Logger::GetInstance().Log(std::string("Option:") + lOption + " has no parameter set",
                                                  Logger::Level::ERROR);
                    }
                } catch (std::exception& aException) {
                    Logger::GetInstance().Log(std::string("Option could not be read: ") + aException.what(),
                                              Logger::Level::ERROR);
                }
            } else {
                lContinue = false;
            }
        }
        lFile.close();

        if (lFile.eof()) {
            lReturn = true;
        } else {
            Logger::GetInstance().Log("Could not save config", Logger::Level::ERROR);
        }
    } else {
        Logger::GetInstance().Log(std::string("Could not open/create config file: ") + aPath.data(),
                                  Logger::Level::ERROR);
    }

    return lReturn;
}
