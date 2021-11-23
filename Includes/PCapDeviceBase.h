#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - PCapDeviceBase.h
 *
 * This file contains the base class for pcap devices.
 **/

#include "IPCapDevice.h"

/**
 * Contains the base class for pcap devices.
 */
class PCapDeviceBase : public IPCapDevice
{
public:
    std::string DataToString(const unsigned char* aData, const pcap_pkthdr* aHeader) override;
    const unsigned char* GetData() override;
    const pcap_pkthdr* GetHeader() override;
    void SetHosting(bool aHosting) override;
    void SetConnector(std::shared_ptr<IConnector> aDevice) override;
    void ShowPacketStatistics(const pcap_pkthdr* aHeader) const override;

protected:
    std::shared_ptr<IConnector> GetConnector();
    [[nodiscard]] bool          IsHosting() const;
    void IncreasePacketCount();
    void SetData(const unsigned char* aData);
    void SetHeader(const pcap_pkthdr* aHeader);

private:
    std::shared_ptr<IConnector>  mConnector{nullptr};
    const unsigned char*         mData{nullptr};
    const pcap_pkthdr*           mHeader{nullptr};
    bool                         mHosting{false};
    unsigned int                 mPacketCount{0};

};
