/* Copyright (c) 2021 [Rick de Bondt] - PluginPacketHandling_Test.cpp
 * This file contains tests for the PacketConverter class.
 **/


#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Includes/NetConversionFunctions.h"
#include "../Includes/PCapReader.h"
#include "../Includes/WirelessPSPPluginDevice.h"
#include "IConnectorMock.h"
#include "IPCapWrapperMock.h"
#include "IWifiInterfaceMock.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::WithArg;

class PluginPacketHandlingTest : public ::testing::Test
{};

// When a packet is received from XLink the source Mac address at the be moved to the back, and the adapter Mac should
// Be put into the source field
TEST_F(PluginPacketHandlingTest, NormalPacketHandlingXLinkSide)
{
    PCapReader                      lPCapReader{true, true, false};
    std::shared_ptr<IWifiInterface> lWifiInterface{std::make_shared<IWifiInterfaceMock>()};
    std::vector<std::string>        lSSIDFilter{""};
    auto                            lPCapWrapperMock{std::make_shared<::testing::NiceMock<IPCapWrapperMock>>()};
    auto                            lPSPPluginDevice{
        std::make_shared<WirelessPSPPluginDevice>(false,
                                                  WirelessPSPPluginDevice_Constants::cReconnectionTimeOut,
                                                  nullptr,
                                                  std::make_shared<HandlerPSPPlugin>(),
                                                  std::static_pointer_cast<IPCapWrapper>(lPCapWrapperMock))};

    // Expectation classes (PCapReader and the like)
    const std::string        lOutputFileName{"../Tests/Output/PluginXLinkTestOutput.pcap"};
    std::vector<std::string> lReceiveBuffer{};
    std::vector<timeval>     lReceiveTimeStamp{};

    std::vector<std::string> lExpectedPackets{};
    std::vector<std::string> lOutputPackets{};
    std::vector<timeval>     lTimeStamp{};

    PCapWrapper lWrapper;
    lWrapper.OpenDead(DLT_EN10MB, 65535);
    pcap_dumper_t*              lDumper{lWrapper.DumpOpen(lOutputFileName.c_str())};
    std::shared_ptr<IConnector> lExpectedConnector{std::make_shared<IConnectorMock>()};
    PCapReader                  lPCapInputReader{false, false, false};
    PCapReader                  lPCapExpectedReader{false, false, false};

    // Expectations
    // The WiFi Mac address obviously needs to stay the same during testing so return a fake one
    EXPECT_CALL(*std::static_pointer_cast<IWifiInterfaceMock>(lWifiInterface), GetAdapterMacAddress)
        .WillOnce(Return(0xb03f29f81800));

    // PCAP set-up
    lPCapInputReader.Open("../Tests/Input/PluginFromXLinkTest.pcapng");
    lPCapInputReader.SetIncomingConnection(lPSPPluginDevice);

    lPCapExpectedReader.Open("../Tests/Input/PluginFromXLinkTest_Expected.pcap");
    lPCapExpectedReader.SetConnector(lExpectedConnector);

    // Put everything sent to the "WiFi-card" into a messagebuffer with timestamps
    ON_CALL(*lPCapWrapperMock, IsActivated()).WillByDefault(Return(true));
    EXPECT_CALL(*lPCapWrapperMock, SendPacket(_))
        .WillRepeatedly(DoAll(WithArg<0>([&](std::string_view aMessage) {
                                  lOutputPackets.emplace_back(std::string(aMessage));
                                  lTimeStamp.push_back(lPCapInputReader.GetHeader()->ts);
                              }),
                              Return(0)));

    // Put all expected packets into a buffer as well for easy comparison
    EXPECT_CALL(*std::static_pointer_cast<IConnectorMock>(lExpectedConnector), Send(_))
        .WillRepeatedly(
            DoAll(WithArg<0>([&](std::string_view aMessage) { lExpectedPackets.emplace_back(std::string(aMessage)); }),
                  Return(true)));

    // Test class set-up
    lPSPPluginDevice->Open("wlan0", lSSIDFilter, lWifiInterface);

    // Normally the XLink Kai connection side blacklists this, but we'll do it here
    lPSPPluginDevice->BlackList(0x0018f8293fb0);

    lPCapInputReader.StartReceiverThread();
    lPCapExpectedReader.StartReceiverThread();

    while (!lPCapInputReader.IsDoneReceiving() || !lPCapExpectedReader.IsDoneReceiving()) {}

    // Should have the same amount of packets
    ASSERT_EQ(lExpectedPackets.size(), lOutputPackets.size());

    // Output a file with the results as well so the results can be further inspected
    int lCount = 0;
    for (auto& lMessage : lOutputPackets) {
        // Packet should be the same
        ASSERT_EQ(lMessage, lExpectedPackets.at(lCount));

        pcap_pkthdr lHeader{};
        lHeader.caplen = lMessage.size();
        lHeader.len    = lMessage.size();
        lHeader.ts     = lTimeStamp.at(lCount);

        lWrapper.Dump(
            reinterpret_cast<unsigned char*>(lDumper), &lHeader, reinterpret_cast<unsigned char*>(lMessage.data()));

        lCount++;
    }

    lWrapper.DumpClose(lDumper);
    lWrapper.Close();

    lPCapInputReader.Close();
    lPCapExpectedReader.Close();
    lPSPPluginDevice->Close();
}

// When a packet is received from the PSP the Mac address at the end should be moved back to its proper place before
// being sent to XLink Kai.
TEST_F(PluginPacketHandlingTest, NormalPacketHandlingPSPSide)
{
    // Class to be tested, with components needed for it to function
    std::shared_ptr<IWifiInterface> lWifiInterface{std::make_shared<IWifiInterfaceMock>()};
    std::shared_ptr<IConnector>     lOutputConnector{std::make_shared<IConnectorMock>()};
    std::vector<std::string>        lSSIDFilter{""};
    auto                            lPCapWrapperMock{std::make_shared<::testing::NiceMock<IPCapWrapperMock>>()};
    WirelessPSPPluginDevice         lPSPPluginDevice{false,
                                             WirelessPSPPluginDevice_Constants::cReconnectionTimeOut,
                                             nullptr,
                                             std::make_shared<HandlerPSPPlugin>(),
                                             std::static_pointer_cast<IPCapWrapper>(lPCapWrapperMock)};

    // Expectation classes (PCapReader and the like)
    const std::string        lOutputFileName{"../Tests/Output/PluginPSPTestOutput.pcap"};
    std::vector<std::string> lReceiveBuffer{};
    std::vector<timeval>     lReceiveTimeStamp{};

    std::vector<std::string> lInputPackets{};
    std::vector<std::string> lExpectedPackets{};
    std::vector<std::string> lOutputPackets{};
    std::vector<timeval>     lTimeStamp{};

    PCapWrapper lWrapper;
    PCapWrapper lPCapReaderWrapper;
    lWrapper.OpenDead(DLT_EN10MB, 65535);
    pcap_dumper_t*              lDumper{lWrapper.DumpOpen(lOutputFileName.c_str())};
    std::shared_ptr<IConnector> lInputConnector{std::make_shared<IConnectorMock>()};
    std::shared_ptr<IConnector> lExpectedConnector{std::make_shared<IConnectorMock>()};
    PCapReader                  lPCapInputReader{false, false, false};
    PCapReader                  lPCapExpectedReader{false, false, false};

    // Expectations
    // The WiFi Mac address obviously needs to stay the same during testing so return a fake one
    EXPECT_CALL(*std::static_pointer_cast<IWifiInterfaceMock>(lWifiInterface), GetAdapterMacAddress)
        .WillOnce(Return(0xb03f29f81800));

    // This will catch our Mac broadcast, more extensive testing done in later test.
    ON_CALL(*lPCapWrapperMock, IsActivated()).WillByDefault(Return(true));
    EXPECT_CALL(*lPCapWrapperMock, SendPacket(_)).WillOnce(Return(0));

    // PCAP set-up
    lPCapInputReader.Open("../Tests/Input/PluginFromPSPTest.pcapng");
    lPCapInputReader.SetConnector(lInputConnector);

    lPCapExpectedReader.Open("../Tests/Input/PluginFromPSPTest_Expected.pcap");
    lPCapExpectedReader.SetConnector(lExpectedConnector);

    EXPECT_CALL(*std::static_pointer_cast<IConnectorMock>(lInputConnector), Send(_))
        .WillRepeatedly(DoAll(WithArg<0>([&](std::string_view aMessage) {
                                  lInputPackets.emplace_back(aMessage);
                                  lTimeStamp.push_back(lPCapInputReader.GetHeader()->ts);
                              }),
                              Return(true)));

    EXPECT_CALL(*std::static_pointer_cast<IConnectorMock>(lOutputConnector), Send(_))
        .WillRepeatedly(
            DoAll(WithArg<0>([&](std::string_view aMessage) { lOutputPackets.emplace_back(std::string(aMessage)); }),
                  Return(true)));

    EXPECT_CALL(*std::static_pointer_cast<IConnectorMock>(lExpectedConnector), Send(_))
        .WillRepeatedly(
            DoAll(WithArg<0>([&](std::string_view aMessage) { lExpectedPackets.emplace_back(std::string(aMessage)); }),
                  Return(true)));

    lPCapInputReader.StartReceiverThread();
    lPCapExpectedReader.StartReceiverThread();

    while (!lPCapInputReader.IsDoneReceiving() || !lPCapExpectedReader.IsDoneReceiving()) {}

    // Test class set-up
    lPSPPluginDevice.SetConnector(lOutputConnector);
    lPSPPluginDevice.Open("wlan0", lSSIDFilter, lWifiInterface);
    // Normally the XLink Kai connection side blacklists this, but we'll do it here
    lPSPPluginDevice.BlackList(0x0018f8293fb0);

    // Receiving data
    for (int lCount = 0; lCount < lInputPackets.size(); lCount++) {
        pcap_pkthdr lHeader{};
        lHeader.caplen = lInputPackets.at(lCount).size();
        lHeader.len    = lInputPackets.at(lCount).size();
        lHeader.ts     = lTimeStamp.at(lCount);

        lPSPPluginDevice.ReadCallback(reinterpret_cast<const unsigned char*>(lInputPackets.at(lCount).data()),
                                      &lHeader);
    }

    // Should have the same amount of packets
    ASSERT_EQ(lExpectedPackets.size(), lOutputPackets.size());

    // Output a file with the results as well so the results can be further inspected
    int lCount = 0;
    for (auto& lMessage : lOutputPackets) {
        // Packet should be the same
        ASSERT_EQ(lMessage, lExpectedPackets.at(lCount));

        pcap_pkthdr lHeader{};
        lHeader.caplen = lMessage.size();
        lHeader.len    = lMessage.size();
        lHeader.ts     = lTimeStamp.at(lCount);

        lWrapper.Dump(
            reinterpret_cast<unsigned char*>(lDumper), &lHeader, reinterpret_cast<unsigned char*>(lMessage.data()));

        lCount++;
    }

    lWrapper.DumpClose(lDumper);
    lWrapper.Close();

    lPCapInputReader.Close();
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
    auto                            lPCapWrapperMock{std::make_shared<::testing::NiceMock<IPCapWrapperMock>>()};
    WirelessPSPPluginDevice         lPSPPluginDevice{false,
                                             WirelessPSPPluginDevice_Constants::cReconnectionTimeOut,
                                             nullptr,
                                             std::make_shared<HandlerPSPPlugin>(),
                                             std::static_pointer_cast<IPCapWrapper>(lPCapWrapperMock)};

    // Expectation classes (PCapReader and the like)
    const std::string        lOutputFileName{"../Tests/Output/BroadcastMacOutput.pcap"};
    std::vector<std::string> lSendBuffer{};
    std::vector<timeval>     lTimeStamp{};

    PCapWrapper lWrapper{};
    lWrapper.OpenDead(DLT_EN10MB, 65535);
    pcap_dumper_t*              lDumper{lWrapper.DumpOpen(lOutputFileName.c_str())};
    std::shared_ptr<IConnector> lExpectedConnector{std::make_shared<IConnectorMock>()};
    PCapReader                  lPCapReader{true, false, false};
    PCapReader                  lPCapExpectedReader{true, false, false};

    // Expectations
    // The WiFi Mac address obviously needs to stay the same during testing so return a fake one
    EXPECT_CALL(*std::static_pointer_cast<IWifiInterfaceMock>(lWifiInterface), GetAdapterMacAddress)
        .WillOnce(Return(0xb03f29f81800));

    // This will catch our Mac broadcast
    ON_CALL(*lPCapWrapperMock, IsActivated()).WillByDefault(Return(true));
    EXPECT_CALL(*lPCapWrapperMock, SendPacket(_))
        .WillRepeatedly(DoAll(WithArg<0>([&](std::string_view aMessage) {
                                  lSendBuffer.emplace_back(std::string(aMessage));
                                  timeval lTv;
                                  lTv.tv_sec = lTimeStamp.back().tv_sec;
                                  lTv.tv_usec =
                                      lTimeStamp.back().tv_usec + 1000;  // Pretend that we sent this 1000 USec later
                                  lTimeStamp.emplace_back(lTv);
                              }),
                              Return(0)));

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

        lWrapper.Dump(
            reinterpret_cast<unsigned char*>(lDumper), &lHeader, reinterpret_cast<unsigned char*>(lMessage.data()));

        lCount++;
    }

    lWrapper.DumpClose(lDumper);
    lWrapper.Close();

    lPCapReader.Close();
    lPCapExpectedReader.Close();
    lPSPPluginDevice.Close();
}

// This tests that a broadcast packet is not sent when the destination of a packet is not FF:FF:FF:FF:FF:FF
// Because that would mean the PSP is already sending directly to the PC
TEST_F(PluginPacketHandlingTest, NoBroadcastMacToBeUsedWhenNormalPacket)
{
    PCapReader                      lPCapReader{true, false, false};
    std::shared_ptr<IWifiInterface> lWifiInterface{std::make_shared<IWifiInterfaceMock>()};
    std::shared_ptr<IConnector>     lConnector{std::make_shared<IConnectorMock>()};
    std::vector<std::string>        lSSIDFilter{""};
    auto                            lPCapWrapperMock{std::make_shared<::testing::NiceMock<IPCapWrapperMock>>()};
    WirelessPSPPluginDevice         lPSPPluginDevice{false,
                                             WirelessPSPPluginDevice_Constants::cReconnectionTimeOut,
                                             nullptr,
                                             std::make_shared<HandlerPSPPlugin>(),
                                             std::static_pointer_cast<IPCapWrapper>(lPCapWrapperMock)};

    // The WiFi Mac address obviously needs to stay the same during testing so return a fake one
    EXPECT_CALL(*std::static_pointer_cast<IWifiInterfaceMock>(lWifiInterface), GetAdapterMacAddress)
        .WillOnce(Return(0xb03f29f81800));

    // Never expect a call on the device itself (Broadcast), but do expect it to forward the packet to XLink Kai
    EXPECT_CALL(*lPCapWrapperMock, SendPacket(_)).Times(0);
    EXPECT_CALL(*std::static_pointer_cast<IConnectorMock>(lConnector), Send(_)).WillOnce(Return(true));

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
