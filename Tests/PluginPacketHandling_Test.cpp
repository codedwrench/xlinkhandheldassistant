/* Copyright (c) 2021 [Rick de Bondt] - PluginPacketHandling_Test.cpp
 * This file contains tests for the PacketConverter class.
 **/


#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Includes/NetConversionFunctions.h"
#include "../Includes/PCapReader.h"
#include "../Includes/WirelessPSPPluginDevice.h"
#include "../Includes/XLinkKaiConnection.h"
#include "IConnectorMock.h"
#include "IWifiInterfaceMock.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::WithArg;

class PCapReaderDerived : public PCapReader
{
public:
    using PCapReader::PCapReader;
    MOCK_METHOD(bool, Send, (std::string_view aData));
};

class WirelessPSPPluginDeviceDerived : public WirelessPSPPluginDevice
{
public:
    using WirelessPSPPluginDevice::WirelessPSPPluginDevice;
    MOCK_METHOD(bool, Send, (std::string_view aData));
    MOCK_METHOD(bool, Send, (std::string_view aData, bool aModifyData));
};

class PluginPacketHandlingTest : public ::testing::Test
{};

// When a packet is received from the PSP the MAC address at the end should be moved back to its proper place before
// being sent to XLink Kai, when a packet is received from XLink Kai the original MAC address should be moved to the end
// of the packet.
TEST_F(PluginPacketHandlingTest, NormalPacketHandling)
{
    // Class to be tested, with components needed for it to function
    std::shared_ptr<IWifiInterface> lWifiInterface{std::make_shared<IWifiInterfaceMock>()};
    std::shared_ptr<IConnector>     lConnector{std::make_shared<IConnectorMock>()};
    std::vector<std::string>        lSSIDFilter{""};
    WirelessPSPPluginDeviceDerived  lPSPPluginDevice{};

    // Expectation classes (PCapReader and the like)
    const std::string        lOutputFileName{"../Tests/Output/BroadcastMacOutput.pcap"};
    std::vector<std::string> lSendBuffer{};
    std::vector<timeval>     lTimeStamp{};

    pcap_t*                     lHandler{pcap_open_dead(DLT_EN10MB, 65535)};
    pcap_dumper_t*              lDumper{pcap_dump_open(lHandler, lOutputFileName.c_str())};
    std::shared_ptr<IConnector> lExpectedConnector{std::make_shared<IConnectorMock>()};
    PCapReaderDerived           lPCapReader{true, false};
    PCapReaderDerived           lPCapExpectedReader{true, false};

    // Expectations
    // The WiFi Mac address obviously needs to stay the same during testing so return a fake one
    EXPECT_CALL(*std::dynamic_pointer_cast<IWifiInterfaceMock>(lWifiInterface), GetAdapterMACAddress)
        .WillOnce(Return(0xb03f29f81800));

    // This will catch our MAC broadcast
    EXPECT_CALL(lPSPPluginDevice, Send(_, false))
        .WillRepeatedly(DoAll(WithArg<0>([&](std::string_view aMessage) {
                                  lSendBuffer.emplace_back(std::string(aMessage));
                                  timeval lTv;
                                  lTv.tv_sec = lTimeStamp.back().tv_sec;
                                  lTv.tv_usec =
                                      lTimeStamp.back().tv_usec + 1000;  // Pretend that we sent this 1000 USec later
                                  lTimeStamp.emplace_back(lTv);
                              }),
                              Return(true)));

    // PCAP set-up
    lPCapReader.Open("../Tests/Input/BroadcastMacTest.pcapng", lSSIDFilter);
    lPCapReader.SetConnector(lExpectedConnector);

    lPCapExpectedReader.Open("../Tests/Input/BroadcastMacTest_Expected.pcapng", lSSIDFilter);
    lPCapExpectedReader.SetConnector(lExpectedConnector);

    // Readying data
    lPCapReader.ReadNextData();
    lTimeStamp.emplace_back(lPCapReader.GetHeader()->ts);

    lPCapExpectedReader.ReadNextData();

    // Test class set-up
    lPSPPluginDevice.SetConnector(lConnector);
    lPSPPluginDevice.Open("wlan0", lSSIDFilter, lWifiInterface);

    // Testing the broadcast
    lPSPPluginDevice.ReadCallback(lPCapReader.GetData(), lPCapReader.GetHeader());

    std::string lExpectedString =
        std::string(reinterpret_cast<const char*>(lPCapExpectedReader.GetData()), lPCapExpectedReader.GetHeader()->len);
    ASSERT_EQ(lSendBuffer.size(), 1);
    ASSERT_EQ(lSendBuffer.front(), lExpectedString);

    // Output a file with the results as well so the results can be further inspected
    int lCount = 0;
    for (auto& lMessage : lSendBuffer) {
        pcap_pkthdr lHeader{};
        lHeader.caplen = lMessage.size();
        lHeader.len    = lMessage.size();
        lHeader.ts     = lTimeStamp.at(lCount);

        pcap_dump(reinterpret_cast<u_char*>(lDumper), &lHeader, reinterpret_cast<const u_char*>(lMessage.c_str()));

        lCount++;
    }

    pcap_dump_close(lDumper);
    pcap_close(lHandler);

    lPCapReader.Close();
    lPCapExpectedReader.Close();
    lPSPPluginDevice.Close();
}

// We are expecting a packet to be sent out when a broadcast message has been received in the format:
// [ Destination Mac ] [ Source Mac ] 88c8 [ Source Mac ]
TEST_F(PluginPacketHandlingTest, BroadcastMacToBeUsed)
{
    // Class to be tested, with components needed for it to function
    std::shared_ptr<IWifiInterface> lWifiInterface{std::make_shared<IWifiInterfaceMock>()};
    std::shared_ptr<IConnector>     lConnector{std::make_shared<IConnectorMock>()};
    std::vector<std::string>        lSSIDFilter{""};
    WirelessPSPPluginDeviceDerived  lPSPPluginDevice{};

    // Expectation classes (PCapReader and the like)
    const std::string        lOutputFileName{"../Tests/Output/BroadcastMacOutput.pcap"};
    std::vector<std::string> lSendBuffer{};
    std::vector<timeval>     lTimeStamp{};

    pcap_t*                     lHandler{pcap_open_dead(DLT_EN10MB, 65535)};
    pcap_dumper_t*              lDumper{pcap_dump_open(lHandler, lOutputFileName.c_str())};
    std::shared_ptr<IConnector> lExpectedConnector{std::make_shared<IConnectorMock>()};
    PCapReaderDerived           lPCapReader{true, false};
    PCapReaderDerived           lPCapExpectedReader{true, false};

    // Expectations
    // The WiFi Mac address obviously needs to stay the same during testing so return a fake one
    EXPECT_CALL(*std::dynamic_pointer_cast<IWifiInterfaceMock>(lWifiInterface), GetAdapterMACAddress)
        .WillOnce(Return(0xb03f29f81800));

    // This will catch our MAC broadcast
    EXPECT_CALL(lPSPPluginDevice, Send(_, false))
        .WillRepeatedly(DoAll(WithArg<0>([&](std::string_view aMessage) {
                                  lSendBuffer.emplace_back(std::string(aMessage));
                                  timeval lTv;
                                  lTv.tv_sec = lTimeStamp.back().tv_sec;
                                  lTv.tv_usec =
                                      lTimeStamp.back().tv_usec + 1000;  // Pretend that we sent this 1000 USec later
                                  lTimeStamp.emplace_back(lTv);
                              }),
                              Return(true)));

    // PCAP set-up
    lPCapReader.Open("../Tests/Input/BroadcastMacTest.pcapng", lSSIDFilter);
    lPCapReader.SetConnector(lExpectedConnector);

    lPCapExpectedReader.Open("../Tests/Input/BroadcastMacTest_Expected.pcapng", lSSIDFilter);
    lPCapExpectedReader.SetConnector(lExpectedConnector);

    // Readying data
    lPCapReader.ReadNextData();
    lTimeStamp.emplace_back(lPCapReader.GetHeader()->ts);

    lPCapExpectedReader.ReadNextData();

    // Test class set-up
    lPSPPluginDevice.SetConnector(lConnector);
    lPSPPluginDevice.Open("wlan0", lSSIDFilter, lWifiInterface);

    // Testing the broadcast
    lPSPPluginDevice.ReadCallback(lPCapReader.GetData(), lPCapReader.GetHeader());

    std::string lExpectedString =
        std::string(reinterpret_cast<const char*>(lPCapExpectedReader.GetData()), lPCapExpectedReader.GetHeader()->len);
    ASSERT_EQ(lSendBuffer.size(), 1);
    ASSERT_EQ(lSendBuffer.front(), lExpectedString);

    // Output a file with the results as well so the results can be further inspected
    int lCount = 0;
    for (auto& lMessage : lSendBuffer) {
        pcap_pkthdr lHeader{};
        lHeader.caplen = lMessage.size();
        lHeader.len    = lMessage.size();
        lHeader.ts     = lTimeStamp.at(lCount);

        pcap_dump(reinterpret_cast<u_char*>(lDumper), &lHeader, reinterpret_cast<const u_char*>(lMessage.c_str()));

        lCount++;
    }

    pcap_dump_close(lDumper);
    pcap_close(lHandler);

    lPCapReader.Close();
    lPCapExpectedReader.Close();
    lPSPPluginDevice.Close();
}

// This tests that a broadcast packet is not sent when the destination of a packet is not FF:FF:FF:FF:FF:FF
// Because that would mean the PSP is already sending directly to the PC
TEST_F(PluginPacketHandlingTest, NoBroadcastMacToBeUsedWhenNormalPacket)
{
    PCapReaderDerived               lPCapReader{true, false};
    std::shared_ptr<IWifiInterface> lWifiInterface{std::make_shared<IWifiInterfaceMock>()};
    std::shared_ptr<IConnector>     lConnector{std::make_shared<IConnectorMock>()};
    std::vector<std::string>        lSSIDFilter{""};
    WirelessPSPPluginDeviceDerived  lPSPPluginDevice{};

    // The WiFi Mac address obviously needs to stay the same during testing so return a fake one
    EXPECT_CALL(*std::dynamic_pointer_cast<IWifiInterfaceMock>(lWifiInterface), GetAdapterMACAddress)
        .WillOnce(Return(0xb03f29f81800));

    // Never expect a call on the device itself (Broadcast), but do expect it to forward the packet to XLink Kai
    EXPECT_CALL(lPSPPluginDevice, Send(_, _)).Times(0);
    EXPECT_CALL(lPSPPluginDevice, Send(_)).Times(0);
    EXPECT_CALL(*std::dynamic_pointer_cast<IConnectorMock>(lConnector), Send(_)).WillOnce(Return(true));

    // PCAP set-up
    lPCapReader.Open("../Tests/Input/NoBroadcastMacTest.pcapng", lSSIDFilter);
    lPCapReader.SetConnector(lConnector);

    // Readying data
    lPCapReader.ReadNextData();

    // Test class set-up
    lPSPPluginDevice.SetConnector(lConnector);
    lPSPPluginDevice.Open("wlan0", lSSIDFilter, lWifiInterface);

    // Testing
    lPSPPluginDevice.ReadCallback(lPCapReader.GetData(), lPCapReader.GetHeader());
}
