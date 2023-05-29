/* Copyright (c) 2021 [Rick de Bondt] - PCapWrapper.cpp */

#include "PCapWrapper.h"

int PCapWrapper::Activate()
{
    return pcap_activate(mHandler);
}

void PCapWrapper::BreakLoop()
{
    if (mHandler != nullptr) {
        pcap_breakloop(mHandler);
    }
}

void PCapWrapper::Close()
{
    if (mHandler != nullptr) {
        pcap_close(mHandler);
        mHandler = nullptr;
    }
}

pcap_t* PCapWrapper::Create(const char* source, char* errbuf)
{
    mHandler = pcap_create(source, errbuf);
    return mHandler;
}

int PCapWrapper::Dispatch(int cnt, pcap_handler callback, unsigned char* user)
{
    return pcap_dispatch(mHandler, cnt, callback, user);
}

void PCapWrapper::Dump(unsigned char* user, pcap_pkthdr* header, unsigned char* message)
{
    return pcap_dump(user, header, message);
}

void PCapWrapper::DumpClose(pcap_dumper_t* dumper)
{
    pcap_dump_close(dumper);
}

pcap_dumper_t* PCapWrapper::DumpOpen(const char* outputfile)
{
    return pcap_dump_open(mHandler, outputfile);
}

int PCapWrapper::FindAllDevices(pcap_if_t** alldevicesp, char* errbuf)
{
    return pcap_findalldevs(alldevicesp, errbuf);
}

void PCapWrapper::FreeAllDevices(pcap_if_t* alldevices)
{
    return pcap_freealldevs(alldevices);
}

int PCapWrapper::GetDatalink()
{
    return pcap_datalink(mHandler);
}

bool PCapWrapper::IsActivated()
{
    return mHandler != nullptr;
}

char* PCapWrapper::GetError()
{
    return pcap_geterr(mHandler);
}

pcap_t* PCapWrapper::OpenDead(int linktype, int snaplen)
{
    mHandler = pcap_open_dead(linktype, snaplen);
    return mHandler;
}

pcap_t* PCapWrapper::OpenOffline(const char* fname, char* errbuf)
{
    mHandler = pcap_open_offline(fname, errbuf);
    return mHandler;
}

int PCapWrapper::NextEx(pcap_pkthdr** header, const unsigned char** pkt_data)
{
    return pcap_next_ex(mHandler, header, pkt_data);
}

int PCapWrapper::SendPacket(std::string_view buffer)
{
    return pcap_sendpacket(
        mHandler, reinterpret_cast<const unsigned char*>(buffer.data()), static_cast<int>(buffer.size()));
}

int PCapWrapper::SetDirection(PcapDirection::Direction direction)
{
    return pcap_setdirection(mHandler, static_cast<pcap_direction_t>(direction));
}

int PCapWrapper::SetImmediateMode(int mode)
{
    return pcap_set_immediate_mode(mHandler, mode);
}

int PCapWrapper::SetPromiscuousMode(int promisc)
{
    return pcap_set_promisc(mHandler, promisc);
}

int PCapWrapper::SetSnapLen(int snaplen)
{
    return pcap_set_snaplen(mHandler, snaplen);
}

int PCapWrapper::SetTimeOut(int timeout)
{
    return pcap_set_timeout(mHandler, timeout);
}
