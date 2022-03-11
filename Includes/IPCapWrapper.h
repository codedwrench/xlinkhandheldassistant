#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - IPCapWrapper.h
 *
 * This file contains an interface for pcapwrapper
 *
 **/

/**
 * Interface for pcapwrapper.
 */

struct pcap;
struct pcap_dumper;
struct pcap_if;
struct pcap_addr;

using pcap_t        = struct pcap;
using pcap_dumper_t = struct pcap_dumper;
using pcap_if_t     = struct pcap_if;
using pcap_addr_t   = struct pcap_addr;

typedef void (*pcap_handler)(u_char*, const struct pcap_pkthdr*, const u_char*);

namespace PcapDirection
{
    enum Direction
    {
        INOUT = 0,
        IN,
        OUT
    };
}  // namespace PcapDirection

class IPCapWrapper
{
public:
    virtual int            Activate()                                                             = 0;
    virtual void           BreakLoop()                                                            = 0;
    virtual void           Close()                                                                = 0;
    virtual pcap_t*        Create(const char* source, char* errbuf)                               = 0;
    virtual int            Dispatch(int cnt, pcap_handler callback, unsigned char* user)          = 0;
    virtual void           Dump(unsigned char* user, pcap_pkthdr* header, unsigned char* message) = 0;
    virtual void           DumpClose(pcap_dumper_t* dumper)                                       = 0;
    virtual pcap_dumper_t* DumpOpen(const char* outputfile)                                       = 0;
    virtual int            FindAllDevices(pcap_if_t** alldevicesp, char* errbuf)                  = 0;
    virtual void           FreeAllDevices(pcap_if_t* devices)                                     = 0;
    virtual int            GetDatalink()                                                          = 0;
    virtual char*          GetError()                                                             = 0;
    virtual bool           IsActivated()                                                          = 0;
    virtual pcap_t*        OpenDead(int linktype, int snaplen)                                    = 0;
    virtual pcap_t*        OpenOffline(const char* fname, char* errbuf)                           = 0;
    virtual int            NextEx(pcap_pkthdr** header, const unsigned char** pkt_data)           = 0;
    virtual int            SendPacket(std::string_view buffer)                                    = 0;
    virtual int            SetDirection(PcapDirection::Direction direction)                       = 0;
    virtual int            SetImmediateMode(int mode)                                             = 0;
    virtual int            SetSnapLen(int snaplen)                                                = 0;
    virtual int            SetTimeOut(int timeout)                                                = 0;
};
