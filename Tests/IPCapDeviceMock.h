/* Copyright (c) 2021 [Rick de Bondt] - IPCapDeviceMock.ch
 * This file contains a mock for IPCapDevice.
 **/

#include "IPCapDevice.h"

class IPCapDeviceMock : public IPCapDevice
{
public:
    MOCK_METHOD(void, BlackList, (uint64_t aMac));
    MOCK_METHOD(void, Close, ());
    MOCK_METHOD(bool, Connect, (std::string_view aESSID));
    MOCK_METHOD(bool, Open, (std::string_view aName, std::vector<std::string>& aSSIDFilter));
    MOCK_METHOD(std::string, DataToString, (const unsigned char* aData, const pcap_pkthdr* aHeader));
    MOCK_METHOD(const unsigned char*, GetData, ());
    MOCK_METHOD(const pcap_pkthdr*, GetHeader, ());
    MOCK_METHOD(std::string, GetESSID, ());
    MOCK_METHOD(std::string, GetTitleId, ());
    MOCK_METHOD(bool, ReadCallback, (const unsigned char* aData, const pcap_pkthdr* aHeader));
    MOCK_METHOD(bool, Send, (std::string_view aData));
    MOCK_METHOD(void, SetConnector, (std::shared_ptr<IConnector> aDevice));
    MOCK_METHOD(void, ShowPacketStatistics, (const pcap_pkthdr* aHeader), (const));
    MOCK_METHOD(bool, StartReceiverThread, ());
};