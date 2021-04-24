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
};

class PluginPacketHandlingTest : public ::testing::Test
{};

TEST_F(PluginPacketHandlingTest, BroadcastMacToBeUsed)
{
    std::shared_ptr<IConnector> lConnector{std::make_shared<IConnectorMock>()};
    std::shared_ptr<IConnector> lExpectedConnector{std::make_shared<IConnectorMock>()};
    pcap_t*                     lHandler{pcap_open_dead(DLT_IEEE802_11_RADIO, 65535)};
    std::vector<std::string>    lSSIDFilter{""};
    const std::string           lOutputFileName{"../Tests/Output/BroadcastMac.pcap"};
    pcap_dumper_t*              lDumper{pcap_dump_open(lHandler, lOutputFileName.c_str())};

    WirelessPSPPluginDeviceDerived lPSPPluginDevice{};
    lPSPPluginDevice.SetConnector(lConnector);
    PCapReaderDerived lPCapReader{true, false};
    PCapReaderDerived lPCapExpectedReader{true, false};
    lPCapReader.Open("../Tests/Input/BroadcastMacTest.pcapng", lSSIDFilter);
    lPCapReader.ReadNextData();
    lPCapReader.SetConnector(lExpectedConnector);

    lPCapExpectedReader.Open("../Tests/Input/BroadcastMacTest.pcapng", lSSIDFilter);
    lPCapExpectedReader.ReadNextData();
    lPCapExpectedReader.SetConnector(lExpectedConnector);

    std::vector<std::string> lSendBuffer{};
    EXPECT_CALL(lPSPPluginDevice, Send(_))
        .WillOnce(DoAll(WithArg<0>([&](std::string_view aMessage) { lSendBuffer.emplace_back(std::string(aMessage)); }),
                        Return(true)));

    lPSPPluginDevice.ReadCallback(lPCapReader.GetData(), lPCapReader.GetHeader());

    std::string lExpectedString = std::string(reinterpret_cast<const char*>(lPCapExpectedReader.GetData()));
    ASSERT_EQ(lSendBuffer.size(), 1);
    ASSERT_EQ(lSendBuffer.front(), lExpectedString);
}