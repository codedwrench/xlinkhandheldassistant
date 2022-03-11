/* Copyright (c) 2021 [Rick de Bondt] - PromiscuousPacketHandling_Test.cpp
 * This file contains tests for the WirelessPromiscuousDevice class.
 **/

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "NetConversionFunctions.h"
#include "PCapReader.h"
#include "WirelessPromiscuousDevice.h"
#include "IConnectorMock.h"
#include "IPCapWrapperMock.h"
#include "IWifiInterfaceMock.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::WithArg;

class PromiscuousPacketHandlingTest : public ::testing::Test
{};

TEST_F(PromiscuousPacketHandlingTest, NormalPacketHandlingXLinkSide)
{
    std::shared_ptr<IWifiInterface> lWifiInterface{std::make_shared<IWifiInterfaceMock>()};
    std::vector<std::string>        lSSIDFilter{""};
    auto                            lPCapWrapperMock{std::make_shared<::testing::NiceMock<IPCapWrapperMock>>()};
    auto                            lPromiscuousDevice{
        std::make_shared<WirelessPromiscuousDevice>(false,
                                                    WirelessPromiscuousBase_Constants::cReconnectionTimeOut,
                                                    nullptr,
                                                    std::make_shared<Handler8023>(),
                                                    std::static_pointer_cast<IPCapWrapper>(lPCapWrapperMock))};

    // Expectation classes (PCapReader and the like)
    const std::string        lOutputFileName{"../Tests/Output/PromiscuousTestXLink.pcap"};
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
    lPCapInputReader.Open("../Tests/Input/PromiscuousTestXLink.pcap");
    lPCapInputReader.SetIncomingConnection(lPromiscuousDevice);

    // Should be the same as the input
    lPCapExpectedReader.Open("../Tests/Input/PromiscuousTestXLink.pcap");
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
    lPromiscuousDevice->Open("wlan0", lSSIDFilter, lWifiInterface);

    // Normally the XLink Kai connection side blacklists this, but we'll do it here
    lPromiscuousDevice->BlackList(0x0018f8293fb0);

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
    lPromiscuousDevice->Close();
}


// This test should prove that the VRRP Mac sent by XLink Kai would be replaced by the adapter Mac on DDS
TEST_F(PromiscuousPacketHandlingTest, ReplaceMacXLinkSide)
{
    std::shared_ptr<IWifiInterface> lWifiInterface{std::make_shared<IWifiInterfaceMock>()};
    std::vector<std::string>        lSSIDFilter{""};
    auto                            lPCapWrapperMock{std::make_shared<::testing::NiceMock<IPCapWrapperMock>>()};
    auto                            lPromiscuousDevice{
        std::make_shared<WirelessPromiscuousDevice>(false,
                                                    WirelessPromiscuousBase_Constants::cReconnectionTimeOut,
                                                    nullptr,
                                                    std::make_shared<Handler8023>(),
                                                    std::static_pointer_cast<IPCapWrapper>(lPCapWrapperMock))};

    // Expectation classes (PCapReader and the like)
    const std::string        lOutputFileName{"../Tests/Output/DDSMacReplaceTest.pcap"};
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
    lPCapInputReader.Open("../Tests/Input/DDSMacReplaceTest.pcap");
    lPCapInputReader.SetIncomingConnection(lPromiscuousDevice);

    lPCapExpectedReader.Open("../Tests/Input/DDSMacReplaceTest_Expected.pcap");
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
    lPromiscuousDevice->Open("wlan0", lSSIDFilter, lWifiInterface);

    // Normally the XLink Kai connection side blacklists this, but we'll do it here
    lPromiscuousDevice->BlackList(0xd44b5e02ed20);
    lPromiscuousDevice->BlackList(0x00005e0001fe);

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
    lPromiscuousDevice->Close();
}

// This test should prove that the VRRP Mac sent by XLink Kai would be replaced by the adapter Mac on DDS on arp replies
TEST_F(PromiscuousPacketHandlingTest, ReplaceMacArpReplyXLinkSide)
{
    std::shared_ptr<IWifiInterface> lWifiInterface{std::make_shared<IWifiInterfaceMock>()};
    std::vector<std::string>        lSSIDFilter{""};
    auto                            lPCapWrapperMock{std::make_shared<::testing::NiceMock<IPCapWrapperMock>>()};
    auto                            lPromiscuousDevice{
        std::make_shared<WirelessPromiscuousDevice>(false,
                                                    WirelessPromiscuousBase_Constants::cReconnectionTimeOut,
                                                    nullptr,
                                                    std::make_shared<Handler8023>(),
                                                    std::static_pointer_cast<IPCapWrapper>(lPCapWrapperMock))};

    // Expectation classes (PCapReader and the like)
    const std::string        lOutputFileName{"../Tests/Output/DDSArpMacReplaceTestReply.pcap"};
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
    lPCapInputReader.Open("../Tests/Input/DDSArpMacReplaceTestReply.pcap");
    lPCapInputReader.SetIncomingConnection(lPromiscuousDevice);

    lPCapExpectedReader.Open("../Tests/Input/DDSArpMacReplaceTestReply_Expected.pcap");
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
    lPromiscuousDevice->Open("wlan0", lSSIDFilter, lWifiInterface);

    // Normally the XLink Kai connection side blacklists this, but we'll do it here
    lPromiscuousDevice->BlackList(0xd44b5e02ed20);
    lPromiscuousDevice->BlackList(0x00005e0001fe);

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
    lPromiscuousDevice->Close();
}

// This test should prove that the VRRP Mac sent by XLink Kai would be replaced by the adapter Mac on DDS on arp requests
TEST_F(PromiscuousPacketHandlingTest, ReplaceMacArpRequestXLinkSide)
{
    std::shared_ptr<IWifiInterface> lWifiInterface{std::make_shared<IWifiInterfaceMock>()};
    std::vector<std::string>        lSSIDFilter{""};
    auto                            lPCapWrapperMock{std::make_shared<::testing::NiceMock<IPCapWrapperMock>>()};
    auto                            lPromiscuousDevice{
        std::make_shared<WirelessPromiscuousDevice>(false,
                                                    WirelessPromiscuousBase_Constants::cReconnectionTimeOut,
                                                    nullptr,
                                                    std::make_shared<Handler8023>(),
                                                    std::static_pointer_cast<IPCapWrapper>(lPCapWrapperMock))};

    // Expectation classes (PCapReader and the like)
    const std::string        lOutputFileName{"../Tests/Output/DDSArpMacReplaceTestRequest.pcap"};
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
    lPCapInputReader.Open("../Tests/Input/DDSArpMacReplaceTestRequest.pcap");
    lPCapInputReader.SetIncomingConnection(lPromiscuousDevice);

    lPCapExpectedReader.Open("../Tests/Input/DDSArpMacReplaceTestRequest_Expected.pcap");
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
    lPromiscuousDevice->Open("wlan0", lSSIDFilter, lWifiInterface);

    // Normally the XLink Kai connection side blacklists this, but we'll do it here
    lPromiscuousDevice->BlackList(0xd44b5e02ed20);
    lPromiscuousDevice->BlackList(0x00005e0001fe);

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
    lPromiscuousDevice->Close();
}

// When a packet is received from the PSP the Mac address at the end should be moved back to its proper place before
// being sent to XLink Kai.
TEST_F(PromiscuousPacketHandlingTest, NormalPacketHandlingDeviceSide)
{
    // Class to be tested, with components needed for it to function
    std::shared_ptr<IWifiInterface> lWifiInterface{std::make_shared<IWifiInterfaceMock>()};
    std::shared_ptr<IConnector>     lOutputConnector{std::make_shared<IConnectorMock>()};
    std::vector<std::string>        lSSIDFilter{""};
    auto                            lPCapWrapperMock{std::make_shared<::testing::NiceMock<IPCapWrapperMock>>()};
    WirelessPromiscuousDevice       lPromiscuousDevice{false,
                                                 WirelessPromiscuousBase_Constants::cReconnectionTimeOut,
                                                 nullptr,
                                                 std::make_shared<Handler8023>(),
                                                 std::static_pointer_cast<IPCapWrapper>(lPCapWrapperMock)};

    // Expectation classes (PCapReader and the like)
    const std::string        lOutputFileName{"../Tests/Output/PromiscuousTestDevice.pcap"};
    std::vector<std::string> lReceiveBuffer{};
    std::vector<timeval>     lReceiveTimeStamp{};

    std::vector<std::string> lInputPackets{};
    std::vector<std::string> lExpectedPackets{};
    std::vector<std::string> lOutputPackets{};
    std::vector<timeval>     lTimeStamp{};

    PCapWrapper lWrapper;
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

    // PCAP set-up
    lPCapInputReader.Open("../Tests/Input/PromiscuousTestDevice.pcap");
    lPCapInputReader.SetConnector(lInputConnector);

    // Should be the same as the input
    lPCapExpectedReader.Open("../Tests/Input/PromiscuousTestDevice.pcap");
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
    lPromiscuousDevice.SetConnector(lOutputConnector);
    lPromiscuousDevice.Open("wlan0", lSSIDFilter, lWifiInterface);

    // Normally the XLink Kai connection side blacklists this, but we'll do it here
    lPromiscuousDevice.BlackList(0xd44b5e69dfa6);

    // Receiving data
    for (int lCount = 0; lCount < lInputPackets.size(); lCount++) {
        pcap_pkthdr lHeader{};
        lHeader.caplen = lInputPackets.at(lCount).size();
        lHeader.len    = lInputPackets.at(lCount).size();
        lHeader.ts     = lTimeStamp.at(lCount);

        lPromiscuousDevice.ReadCallback(reinterpret_cast<const unsigned char*>(lInputPackets.at(lCount).data()),
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
    lPromiscuousDevice.Close();
}
