/* Copyright (c) 2020 [Rick de Bondt] - Handler8023.cpp */

#include "Handler8023.h"

#include "Logger.h"
#include "NetConversionFunctions.h"

std::string Handler8023::ConvertPacketOut(uint64_t aBSSID, RadioTapReader::PhysicalDeviceParameters aParameters)
{
    std::string lReturn;
    if (mLastReceivedData.size() > Net_8023_Constants::cHeaderLength) {
        unsigned int lIeee80211HeaderSize{sizeof(ieee80211_hdr)};
        unsigned int lLLCHeaderSize{sizeof(uint64_t)};
        unsigned int lDataSize{
            static_cast<unsigned int>(mLastReceivedData.size() - (Net_8023_Constants::cHeaderLength * sizeof(char)))};

        unsigned int lReserveSize{lIeee80211HeaderSize + lLLCHeaderSize + lDataSize};

        std::vector<char> lFullPacket;
        lFullPacket.reserve(lReserveSize);
        lFullPacket.resize(lReserveSize);

        unsigned int lIndex{0};

        // RadioTap Header
        int lRadioTapSize = InsertRadioTapHeader(&lFullPacket[0], aParameters);
        lIndex += lRadioTapSize;

        lFullPacket.reserve(lReserveSize + lRadioTapSize);
        lFullPacket.resize(lReserveSize + lRadioTapSize);

        // IEEE80211 Header
        InsertIEEE80211Header(&lFullPacket[0], mSourceMac, mDestinationMac, aBSSID, lIndex);
        lIndex += lIeee80211HeaderSize;

        // Logical Link Control (LLC) header
        uint64_t lLLC = Net_80211_Constants::cSnapLLC;

        // Set EtherType from ethernet frame
        lLLC |= static_cast<uint64_t>(mEtherType) << 48LLU;

        memcpy(&lFullPacket[0] + lIndex, &lLLC, sizeof(lLLC));
        lIndex += lLLCHeaderSize;

        // Data, without header included
        memcpy(&lFullPacket[0] + lIndex,
               mLastReceivedData.data() + (Net_8023_Constants::cHeaderLength * (sizeof(char))),
               mLastReceivedData.size() - (Net_8023_Constants::cHeaderLength * (sizeof(char))));

        lReturn = std::string(lFullPacket.begin(), lFullPacket.end());
    } else {
        Logger::GetInstance().Log("The header has an invalid length, cannot convert the packet",
                                  Logger::Level::WARNING);
    }

    return lReturn;
}

MacBlackList& Handler8023::GetBlackList()
{
    return mBlackList;
}

uint64_t Handler8023::GetDestinationMac() const
{
    return mDestinationMac;
}

[[nodiscard]] uint16_t Handler8023::GetEtherType() const
{
    return mEtherType;
}

std::string_view Handler8023::GetPacket()
{
    return mLastReceivedData;
}

uint64_t Handler8023::GetSourceMac() const
{
    return mSourceMac;
}

bool Handler8023::IsBroadcastPacket() const
{
    return mIsBroadcastPacket;
}

void Handler8023::Update(std::string_view aPacket)
{
    // Save data in object and fill RadioTap parameters.
    mLastReceivedData = aPacket;

    auto lSourceMac = GetRawData<uint64_t>(mLastReceivedData, Net_8023_Constants::cSourceAddressIndex);
    lSourceMac &= Net_Constants::cBroadcastMac;  // it's actually a uint48.

    // Clean extranous data as well
    mSourceMac = lSourceMac;

    auto lDestinationMac = GetRawData<uint64_t>(mLastReceivedData, Net_8023_Constants::cDestinationAddressIndex);
    lDestinationMac &= Net_Constants::cBroadcastMac;  // it's actually a uint48.

    mDestinationMac    = lDestinationMac;
    mIsBroadcastPacket = mDestinationMac == Net_Constants::cBroadcastMac;

    mEtherType = GetRawData<uint16_t>(mLastReceivedData, Net_8023_Constants::cEtherTypeIndex);
}
