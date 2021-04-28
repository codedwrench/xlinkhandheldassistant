#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - PCapWrapper.h
 *
 * This file contains wrapper functions for communication with pcap
 *
 **/

#include <pcap/pcap.h>
#include <string_view>

#include "IPCapWrapper.h"

class PCapWrapper : public IPCapWrapper
{
public:
   int Activate() override;
   void BreakLoop() override;
   void Close() override;
   pcap_t* Create(const char* source, char* errbuf) override;
   int Dispatch(int cnt, pcap_handler callback, unsigned char* user) override;
   void Dump(unsigned char* user, pcap_pkthdr* header, unsigned char* message) override;
   void DumpClose(pcap_dumper_t* dumper) override;
   pcap_dumper_t* DumpOpen(const char* outputfile) override;
   int FindAllDevices(pcap_if_t** alldevicesp, char* errbuf) override;
   void FreeAllDevices(pcap_if_t* devices) override;
   int GetDatalink() override;
   char* GetError() override;
   bool IsActivated() override;
   pcap_t* OpenDead(int linktype, int snaplen) override;
   pcap_t* OpenOffline(const char* fname, char *errbuf) override;
   int NextEx(pcap_pkthdr** header, const unsigned char **pkt_data) override;
   int SendPacket(std::string_view buffer) override;
   int SetDirection(pcap_direction_t direction) override;
   int SetSnapLen(int snaplen) override;
   int SetTimeOut(int timeout) override;
private:
   pcap_t* mHandler;
};


