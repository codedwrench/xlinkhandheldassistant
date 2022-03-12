/* Copyright (c) 2020 [Rick de Bondt] - Handler80211.cpp */

#include "Handler80211.h"

#include "Logger.h"
#include "NetConversionFunctions.h"

Handler80211::Handler80211(PhysicalDeviceHeaderType aType)
{
    if (aType == PhysicalDeviceHeaderType::RadioTap) {
        mPhysicalDeviceHeaderReader = std::make_shared<RadioTapReader>();
    }

    mParameter80211Reader = std::make_shared<Parameter80211Reader>(mPhysicalDeviceHeaderReader);
}

std::string Handler80211::ConvertPacketOut()
{
    std::string lConvertedPacket{};

    // Only important if Data type
    if ((mPhysicalDeviceHeaderReader != nullptr) && (mMainPacketType == Main80211PacketType::Data)) {
        unsigned int lFCSLength =
            ((mPhysicalDeviceHeaderReader->GetFlags() & RadioTap_Constants::cFCSAvailableFlag) != 0) ? 4 : 0;

        unsigned int lSourceAddressIndex =
            Net_80211_Constants::cSourceAddressIndex + mPhysicalDeviceHeaderReader->GetLength();
        unsigned int lDestinationAddressIndex =
            Net_80211_Constants::cDestinationAddressIndex + mPhysicalDeviceHeaderReader->GetLength();
        unsigned int lTypeIndex = Net_80211_Constants::cEtherTypeIndex + mPhysicalDeviceHeaderReader->GetLength();
        unsigned int lDataIndex = Net_80211_Constants::cDataIndex + mPhysicalDeviceHeaderReader->GetLength();

        // If there is QOS data added to the 80211 header, we need to skip past that as well
        switch (mDataPacketType) {
            case Data80211PacketType::QoSData:
                lTypeIndex += sizeof(uint8_t) * Net_80211_Constants::cDataQOSLength;
                lDataIndex += sizeof(uint8_t) * Net_80211_Constants::cDataQOSLength;
                break;
            default:
                break;
        }

        switch (mDataPacketType) {
            case Data80211PacketType::QoSNull:
            case Data80211PacketType::Null:
                break;
            case Data80211PacketType::QoSData:
            case Data80211PacketType::Data:
                // The header should have its complete size for the packet to be valid.
                if (mLastReceivedData.size() >
                    Net_80211_Constants::cDataHeaderLength + mPhysicalDeviceHeaderReader->GetLength()) {
                    // Strip framecheck sequence as well.
                    lConvertedPacket.reserve(mLastReceivedData.size() - Net_80211_Constants::cDataIndex -
                                             mPhysicalDeviceHeaderReader->GetLength() - lFCSLength);

                    lConvertedPacket.append(mLastReceivedData.substr(lDestinationAddressIndex,
                                                                     Net_80211_Constants::cDestinationAddressLength));

                    lConvertedPacket.append(
                        mLastReceivedData.substr(lSourceAddressIndex, Net_80211_Constants::cSourceAddressLength));

                    lConvertedPacket.append(
                        mLastReceivedData.substr(lTypeIndex, Net_80211_Constants::cEtherTypeLength));

                    lConvertedPacket.append(
                        mLastReceivedData.substr(lDataIndex, mLastReceivedData.size() - lDataIndex - lFCSLength));
                } else {
                    Logger::GetInstance().Log("The header has an invalid length, cannot convert the packet",
                                              Logger::Level::WARNING);
                }
            default:
                break;
        }
    }

    // [ Destination Mac | Source Mac | EtherType ] [ Payload ]
    return lConvertedPacket;
}

MacBlackList& Handler80211::GetBlackList()
{
    return mBlackList;
}

const RadioTapReader::PhysicalDeviceParameters& Handler80211::GetControlPacketParameters()
{
    return mPhysicalDeviceParametersControl;
}

const RadioTapReader::PhysicalDeviceParameters& Handler80211::GetDataPacketParameters()
{
    return mPhysicalDeviceParametersData;
}

std::string_view Handler80211::GetPacket()
{
    return mLastReceivedData;
}

uint64_t Handler80211::GetDestinationMac() const
{
    return mDestinationMac;
}

[[nodiscard]] uint16_t Handler80211::GetEtherType() const
{
    return mEtherType;
}


uint64_t Handler80211::GetLockedBSSID() const
{
    return mLockedBSSID;
}

std::string Handler80211::GetLockedSSID() const
{
    return mLockedSSID;
}

uint64_t Handler80211::GetSourceMac() const
{
    return mSourceMac;
}

bool Handler80211::IsAckable() const
{
    return mAckable;
}

bool Handler80211::IsBroadcastPacket() const
{
    return mIsBroadcastPacket;
}

bool Handler80211::IsBSSIDAllowed(uint64_t aBSSID) const
{
    return mLockedBSSID == aBSSID;
}

bool Handler80211::ShouldSend() const
{
    return mShouldSend;
}

bool Handler80211::IsSSIDAllowed(std::string_view aSSID)
{
    bool lReturn{false};

    if (mSSIDList.empty()) {
        lReturn = true;
    } else {
        for (auto& lSSID : mSSIDList) {
            if (aSSID.find(lSSID) != std::string::npos) {
                lReturn = true;
            }
        }
    }

    return lReturn;
}

bool Handler80211::IsDropped() const
{
    return mIsDropped;
}

void Handler80211::SavePhysicalDeviceParameters(RadioTapReader::PhysicalDeviceParameters& aParameters)
{
    if (mPhysicalDeviceHeaderReader != nullptr) {
        aParameters = mPhysicalDeviceHeaderReader->ExportRadioTapParameters();
    }
}

void Handler80211::SetBSSID(uint64_t aBSSID)
{
    mLockedBSSID = aBSSID;
}

void Handler80211::SetSSIDFilterList(std::vector<std::string>& aSSIDList)
{
    mSSIDList = std::move(aSSIDList);
}

void Handler80211::Update(std::string_view aPacket)
{
    // Save data in object and fill RadioTap parameters.
    mLastReceivedData = aPacket;

    mAckable           = false;
    mIsDropped         = true;
    mShouldSend        = false;
    mEtherType         = 0;
    mIsBroadcastPacket = false;

    if (mPhysicalDeviceHeaderReader != nullptr) {
        mPhysicalDeviceHeaderReader->FillRadioTapParameters(aPacket);
    }

    UpdateMainPacketType();

    switch (mMainPacketType) {
        case Main80211PacketType::Control:
            UpdateDestinationMac();

            // Blacklisted Macs will have a destination Mac in XLink Kai, so only copy info about these packets
            if (GetBlackList().IsMacBlackListed(mDestinationMac)) {
                UpdateControlPacketType();

                if (mControlPacketType == Control80211PacketType::ACK) {
                    Logger::GetInstance().Log("Saving parameters for a Control packet type", Logger::Level::TRACE);
                    SavePhysicalDeviceParameters(mPhysicalDeviceParametersControl);
                    mIsDropped = false;
                }
            }
            break;
        case Main80211PacketType::Data:
            // Only do something with the data frame if we care about this network
            UpdateSourceMac();
            UpdateBSSID();
            if (GetBlackList().IsMacAllowed(mSourceMac) && IsBSSIDAllowed(mBSSID)) {
                UpdateDestinationMac();

                // Put above ackable because ackable needs this flag to be up-to-date
                mIsBroadcastPacket = mDestinationMac == Net_Constants::cBroadcastMac;

                UpdateAckable();
                UpdateDataPacketType();
                UpdateRetry();

                mEtherType = GetRawData<uint16_t>(mLastReceivedData, Net_8023_Constants::cEtherTypeIndex);

                // Only save parameters on normal data types.
                if (!mRetry) {
                    switch (mDataPacketType) {
                        case Data80211PacketType::Data:
                            Logger::GetInstance().Log("Saving parameters for a Data packet type", Logger::Level::TRACE);
                            SavePhysicalDeviceParameters(mPhysicalDeviceParametersData);
                            mShouldSend = true;
                            break;
                        case Data80211PacketType::QoSData:
                            mShouldSend = true;
                            break;
                        default:
                            break;
                    }
                    mIsDropped = false;
                } else {
                    Logger::GetInstance().Log("Packet Retry blocked", Logger::Level::TRACE);
                }
            }
            break;
        case Main80211PacketType::Management:
            UpdateSourceMac();

            if (GetBlackList().IsMacAllowed(mSourceMac)) {
                UpdateManagementPacketType();

                if (mManagementPacketType == Management80211PacketType::Beacon) {
                    mParameter80211Reader->Update(mLastReceivedData);
                    if (IsSSIDAllowed(mParameter80211Reader->GetSSID())) {
                        UpdateBSSID();
                        if (mBSSID != mLockedBSSID) {
                            mLockedBSSID = mBSSID;
                            mLockedSSID  = mParameter80211Reader->GetSSID().data();

                            Logger::GetInstance().Log(
                                std::string("SSID switched:") + mLockedSSID + ", BSSID: " + IntToMac(mBSSID),
                                Logger::Level::DEBUG);

                            mIsDropped = false;
                        }
                    }
                }
            }
            break;
        default:
            Logger::GetInstance().Log("Could not determine main packet type", Logger::Level::DEBUG);
    }
}

void Handler80211::UpdateAckable()
{
    // TODO: Filter multicast
    // Not a broadcast
    if (!mIsBroadcastPacket) {
        mAckable = true;
    }
}

void Handler80211::UpdateBSSID()
{
    uint64_t lBSSID{0};
    if (mPhysicalDeviceHeaderReader != nullptr) {
        lBSSID = GetRawData<uint64_t>(mLastReceivedData,
                                      mPhysicalDeviceHeaderReader->GetLength() + Net_80211_Constants::cBSSIDIndex);
        lBSSID &= Net_Constants::cBroadcastMac;  // it's actually a uint48.

        mBSSID = lBSSID;
    }
}

void Handler80211::UpdateControlPacketType()
{
    Control80211PacketType lResult{Control80211PacketType::None};

    // Makes more sense if you read this: https://en.wikipedia.org/wiki/802.11_Frame_Types#Frame_Control
    if (mPhysicalDeviceHeaderReader != nullptr) {
        uint8_t lControlType{static_cast<uint8_t>(
            GetRawData<uint8_t>(mLastReceivedData, mPhysicalDeviceHeaderReader->GetLength()) >> 4U)};
        if ((lControlType & 0b1111U) == 0b1000U) {
            lResult = Control80211PacketType::BlockAckRequest;
        } else if ((lControlType & 0b1111U) == 0b1001U) {
            lResult = Control80211PacketType::BlockAck;
        } else if ((lControlType & 0b1111U) == 0b1101U) {
            lResult = Control80211PacketType::ACK;
        } else {
            Logger::GetInstance().Log("Could not determine control packet type: " + std::to_string(lControlType),
                                      Logger::Level::DEBUG);
        }
    }

    mControlPacketType = lResult;
}

void Handler80211::UpdateDataPacketType()
{
    Data80211PacketType lResult{Data80211PacketType::None};

    // Makes more sense if you read this: https://en.wikipedia.org/wiki/802.11_Frame_Types#Frame_Control
    if (mPhysicalDeviceHeaderReader != nullptr) {
        uint8_t lDataType{static_cast<uint8_t>(
            GetRawData<uint8_t>(mLastReceivedData, mPhysicalDeviceHeaderReader->GetLength()) >> 4U)};

        if ((lDataType & 0b1111U) == 0b0000U) {
            lResult = Data80211PacketType::Data;
        } else if ((lDataType & 0b1111U) == 0b0100U) {
            lResult = Data80211PacketType::Null;
        } else if ((lDataType & 0b1111U) == 0b1000U) {
            lResult = Data80211PacketType::QoSData;
        } else if ((lDataType & 0b1111U) == 0b1100U) {
            lResult = Data80211PacketType::QoSNull;
        } else {
            Logger::GetInstance().Log("Could not determine data packet type: " + std::to_string(lDataType),
                                      Logger::Level::DEBUG);
        }
    }

    mDataPacketType = lResult;
}

void Handler80211::UpdateMainPacketType()
{
    Main80211PacketType lResult{Main80211PacketType::None};

    // Makes more sense if you read this: https://en.wikipedia.org/wiki/802.11_Frame_Types#Frame_Control
    if (mPhysicalDeviceHeaderReader != nullptr) {
        uint8_t lMainType{static_cast<uint8_t>(
            GetRawData<uint8_t>(mLastReceivedData, mPhysicalDeviceHeaderReader->GetLength()) >> 2U)};
        if ((lMainType & 0b11U) == 0b00U) {
            lResult = Main80211PacketType::Management;
        } else if ((lMainType & 0b11U) == 0b01U) {
            lResult = Main80211PacketType::Control;
        } else if ((lMainType & 0b11U) == 0b10U) {
            lResult = Main80211PacketType::Data;
        }
        // Ignore extensions
    }

    mMainPacketType = lResult;
}

void Handler80211::UpdateManagementPacketType()
{
    Management80211PacketType lResult{Management80211PacketType::None};

    // Makes more sense if you read this: https://en.wikipedia.org/wiki/802.11_Frame_Types#Frame_Control
    if (mPhysicalDeviceHeaderReader != nullptr) {
        uint8_t lManagementType{static_cast<uint8_t>(
            (GetRawData<uint8_t>(mLastReceivedData, mPhysicalDeviceHeaderReader->GetLength())) >> 4U)};

        if ((lManagementType & 0b1111U) == 0b0000U) {
            lResult = Management80211PacketType::AssociationRequest;
        } else if ((lManagementType & 0b1111U) == 0b0001U) {
            lResult = Management80211PacketType::AssociationResponse;
        } else if ((lManagementType & 0b1111U) == 0b0010U) {
            lResult = Management80211PacketType::ReassociationRequest;
        } else if ((lManagementType & 0b1111U) == 0b0011U) {
            lResult = Management80211PacketType::ReassociationResponse;
        } else if ((lManagementType & 0b1111U) == 0b0100U) {
            lResult = Management80211PacketType::ProbeRequest;
        } else if ((lManagementType & 0b1111U) == 0b0101U) {
            lResult = Management80211PacketType::ProbeResponse;
        } else if ((lManagementType & 0b1111U) == 0b1000U) {
            lResult = Management80211PacketType::Beacon;
        } else if ((lManagementType & 0b1111U) == 0b1010U) {
            lResult = Management80211PacketType::Disassociation;
        } else if ((lManagementType & 0b1111U) == 0b1011U) {
            lResult = Management80211PacketType::Authentication;
        } else if ((lManagementType & 0b1111U) == 0b1100U) {
            lResult = Management80211PacketType::Deauthentication;
        } else if ((lManagementType & 0b1111U) == 0b1101U) {
            lResult = Management80211PacketType::Action;
        } else if ((lManagementType & 0b1111U) == 0b1110U) {
            lResult = Management80211PacketType::ActionNoAck;
        } else {
            Logger::GetInstance().Log("Could not determine management packet type: " + std::to_string(lManagementType),
                                      Logger::Level::DEBUG);
        }
        // Ignore the rest
    }

    mManagementPacketType = lResult;
}

void Handler80211::UpdateRetry()
{
    if (mPhysicalDeviceHeaderReader != nullptr) {
        mRetry = GetRawData<uint8_t>(mLastReceivedData, mPhysicalDeviceHeaderReader->GetLength() + 1) ==
                 Net_80211_Constants::cDataRetryFlag;
    }
}

void Handler80211::UpdateDestinationMac()
{
    if (mPhysicalDeviceHeaderReader != nullptr) {
        uint64_t lDestinationMac{GetRawData<uint64_t>(
            mLastReceivedData,
            mPhysicalDeviceHeaderReader->GetLength() + Net_80211_Constants::cDestinationAddressIndex)};
        lDestinationMac &= Net_Constants::cBroadcastMac;  // it's actually a uint48.

        mDestinationMac = lDestinationMac;
    }
}

void Handler80211::UpdateSourceMac()
{
    if (mPhysicalDeviceHeaderReader != nullptr) {
        uint64_t lSourceMac{GetRawData<uint64_t>(
            mLastReceivedData, mPhysicalDeviceHeaderReader->GetLength() + Net_80211_Constants::cSourceAddressIndex)};
        lSourceMac &= Net_Constants::cBroadcastMac;  // it's actually a uint48.

        mSourceMac = lSourceMac;
    }
}
