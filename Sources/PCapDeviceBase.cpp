/* Copyright (c) 2021 [Rick de Bondt] - PCapDeviceBase.cpp */

#include "PCapDeviceBase.h"

#include "Logger.h"
#include "PCapWrapper.h"

void PCapDeviceBase::SetConnector(std::shared_ptr<IConnector> aDevice)
{
    mConnector = aDevice;
}

void PCapDeviceBase::ShowPacketStatistics(const pcap_pkthdr* aHeader) const
{
    Logger::GetInstance().Log("Packet # " + std::to_string(mPacketCount), Logger::Level::TRACE);

    // Show the size in bytes of the packet
    Logger::GetInstance().Log("Packet size: " + std::to_string(aHeader->len) + " bytes", Logger::Level::TRACE);

    // Show Epoch Time
    Logger::GetInstance().Log(
        "Epoch time: " + std::to_string(aHeader->ts.tv_sec) + ":" + std::to_string(aHeader->ts.tv_usec),
        Logger::Level::TRACE);

    // Show a warning if the length captured is different
    if (aHeader->len != aHeader->caplen) {
        Logger::GetInstance().Log("Capture size different than packet size:" + std::to_string(aHeader->len) + " bytes",
                                  Logger::Level::TRACE);
    }
}

std::shared_ptr<IConnector> PCapDeviceBase::GetConnector()
{
    return mConnector;
}

const unsigned char* PCapDeviceBase::GetData()
{
    return mData;
}

const pcap_pkthdr* PCapDeviceBase::GetHeader()
{
    return mHeader;
}

bool PCapDeviceBase::IsHosting() const
{
    return mHosting;
}

void PCapDeviceBase::IncreasePacketCount()
{
    mPacketCount++;
}

void PCapDeviceBase::SetData(const unsigned char* aData)
{
    mData = aData;
}

void PCapDeviceBase::SetHeader(const pcap_pkthdr* aHeader)
{
    mHeader = aHeader;
}

std::string PCapDeviceBase::DataToString(const unsigned char* aData, const pcap_pkthdr* aHeader)
{
    // Convert from char* to string
    std::string lData{};

    if ((aData != nullptr) && (aHeader != nullptr)) {
        lData.resize(aHeader->caplen);
        for (unsigned int lCount = 0; lCount < aHeader->caplen; lCount++) {
            lData.at(lCount) = aData[lCount];
        }
    }

    return lData;
}

void PCapDeviceBase::SetHosting(bool aHosting)
{
    mHosting = aHosting;
}
