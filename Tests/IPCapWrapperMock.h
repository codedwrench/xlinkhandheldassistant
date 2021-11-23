/* Copyright (c) 2021 [Rick de Bondt] - IPCapWrapperMock.h
 * This file contains a mock for IPCapWrapper.
 **/

#include "../Includes/IPCapWrapper.h"

class IPCapWrapperMock : public IPCapWrapper
{
public:
    MOCK_METHOD(int, Activate, ());
    MOCK_METHOD(void, BreakLoop, ());
    MOCK_METHOD(void, Close, ());
    MOCK_METHOD(pcap_t*, Create, (const char* source, char* errbuf));
    MOCK_METHOD(int, Dispatch, (int cnt, pcap_handler callback, unsigned char* user));
    MOCK_METHOD(void, Dump, (unsigned char* user, pcap_pkthdr* header, unsigned char* message));
    MOCK_METHOD(void, DumpClose, (pcap_dumper_t * dumper));
    MOCK_METHOD(pcap_dumper_t*, DumpOpen, (const char* outputfile));
    MOCK_METHOD(int, FindAllDevices, (pcap_if_t * *alldevicesp, char* errbuf));
    MOCK_METHOD(void, FreeAllDevices, (pcap_if_t * devices));
    MOCK_METHOD(int, GetDatalink, ());
    MOCK_METHOD(char*, GetError, ());
    MOCK_METHOD(bool, IsActivated, ());
    MOCK_METHOD(pcap_t*, OpenDead, (int linktype, int snaplen));
    MOCK_METHOD(pcap_t*, OpenOffline, (const char* fname, char* errbuf));
    MOCK_METHOD(int, NextEx, (pcap_pkthdr * *header, const unsigned char** pkt_data));
    MOCK_METHOD(int, SendPacket, (std::string_view buffer));
    MOCK_METHOD(int, SetDirection, (pcap_direction_t direction));
    MOCK_METHOD(int, SetImmediateMode, (int mode));
    MOCK_METHOD(int, SetSnapLen, (int snaplen));
    MOCK_METHOD(int, SetTimeOut, (int timeout));
};
