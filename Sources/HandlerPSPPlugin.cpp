/* Copyright (c) 2021 [Rick de Bondt] - HandlerPSPPlugin.cpp */

#include "HandlerPSPPlugin.h"

#include <climits>

#include "Logger.h"
#include "NetConversionFunctions.h"

std::string HandlerPSPPlugin::ConvertPacketOut()
{
    std::string lReturn{mLastReceivedData.data(), mLastReceivedData.size()};

    // With the plugin the destination mac is kept at the end of the packet
    memcpy(lReturn.data() + Net_8023_Constants::cDestinationAddressIndex,
           &mDestinationMac,
           Net_8023_Constants::cDestinationAddressLength);

    lReturn.resize(lReturn.size() - Net_8023_Constants::cDestinationAddressLength);

    return lReturn;
}

std::string HandlerPSPPlugin::ConvertPacketIn(std::string_view aData, uint64_t aAdapterMac)
{
    std::string lData{aData.data(), aData.size()};

    std::string lActualSourceMac{
        lData.substr(Net_8023_Constants::cSourceAddressIndex, Net_8023_Constants::cSourceAddressLength)};

    memcpy(
        lData.data() + Net_8023_Constants::cSourceAddressIndex, &aAdapterMac, Net_8023_Constants::cSourceAddressLength);

    lData.append(lActualSourceMac);

    return lData;
}

MacBlackList& HandlerPSPPlugin::GetBlackList()
{
    return mBlackList;
}

uint64_t HandlerPSPPlugin::GetDestinationMac() const
{
    return mDestinationMac;
}

std::string_view HandlerPSPPlugin::GetPacket()
{
    return mLastReceivedData;
}

uint64_t HandlerPSPPlugin::GetSourceMac() const
{
    return mSourceMac;
}

bool HandlerPSPPlugin::IsBroadcastPacket() const
{
    return mIsBroadcastPacket;
}

uint16_t HandlerPSPPlugin::GetEtherType() const
{
    return mEtherType;
}

void HandlerPSPPlugin::Update(std::string_view aPacket)
{
    mLastReceivedData = aPacket;
    mEtherType        = 0;

    if (mLastReceivedData.size() > Net_8023_Constants::cHeaderLength) {
        auto lSourceMac = GetRawData<uint64_t>(mLastReceivedData, Net_8023_Constants::cSourceAddressIndex);
        lSourceMac &= Net_Constants::cBroadcastMac;  // it's actually a uint48.

        mSourceMac = lSourceMac;

        // If the packet was a broadcast packet, the Mac is not at the end, so don't get it there.
        if ((GetRawData<uint64_t>(mLastReceivedData, Net_8023_Constants::cDestinationAddressIndex) &
             Net_Constants::cBroadcastMac) == Net_Constants::cBroadcastMac) {
            mDestinationMac    = Net_Constants::cBroadcastMac;
            mIsBroadcastPacket = true;
        } else {
            // Now the size will have to be able to contain the destination length as well.
            if (mLastReceivedData.size() >=
                Net_8023_Constants::cHeaderLength + Net_8023_Constants::cDestinationAddressLength) {
                // Taking 8 bytes instead of 6 makes sure we don't actually go out of bounds on our packet
                auto lDestinationMac =
                    GetRawData<uint64_t>(mLastReceivedData, mLastReceivedData.size() - sizeof(uint64_t));

                // Shift 2 bytes
                lDestinationMac >>= (CHAR_BIT * 2U);

                mDestinationMac    = lDestinationMac;
                mIsBroadcastPacket = false;
            }
        }

        mEtherType = GetRawData<uint16_t>(mLastReceivedData, Net_8023_Constants::cEtherTypeIndex);
    }
}
