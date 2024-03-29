/* Copyright (c) 2020 [Rick de Bondt] - WindowModel_Test.cpp
 * This file contains tests for loading or saving WindowModel members.
 **/

#include "XLinkKaiConnection.h"

#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include <gmock/gmock-actions.h>
#include <gmock/gmock-spec-builders.h>
#include <gtest/gtest.h>

#include "IPCapDeviceMock.h"
#include "ITimerMock.h"
#include "IUDPSocketWrapperMock.h"
#include "MonitorDevice.h"

using testing::_;
using testing::Assign;
using testing::DoAll;
using testing::Invoke;
using testing::Return;
using testing::ReturnPointee;
using testing::SaveArg;

constexpr std::string_view cDefaultTitleId = "ULES00125";
constexpr std::string_view cDefaultESSID = "PSP_AULES00125_BOUTLLOB";
constexpr std::string_view cFullKeepAliveString = "keepalive;";

class MonitorDeviceDerived : virtual public IPCapDeviceMock, virtual public MonitorDevice
{
    // To make sure that the dynamic cast functions
};

class XLinkKaiConnectionTest : public ::testing::Test
{
public:
    std::shared_ptr<IUDPSocketWrapperMock> mSocketWrapperMock{std::make_shared<IUDPSocketWrapperMock>()};
    std::shared_ptr<IPCapDeviceMock>       mPCapDeviceMock{std::make_shared<IPCapDeviceMock>()};
    std::shared_ptr<ITimerMock>            mTimerMock{std::make_shared<ITimerMock>()};

    std::shared_ptr<XLinkKaiConnection> mXLinkKaiConnection{
        std::make_shared<XLinkKaiConnection>(std::static_pointer_cast<IUDPSocketWrapper>(mSocketWrapperMock))};

    XLinkKaiConnectionTest()
    {
        mXLinkKaiConnection->SetIncomingConnection(std::static_pointer_cast<IPCapDevice>(mPCapDeviceMock));
    }

protected:
    void SetConnectionMessages();
};

void XLinkKaiConnectionTest::SetConnectionMessages()
{
    // Connection to XLink Kai
    std::string_view lConnectString{"connect;XLHA_Device;XLHA;"};
    EXPECT_CALL(*mSocketWrapperMock, SendTo(lConnectString))
        .WillOnce(Return(lConnectString.size()))
        .RetiresOnSaturation();

    // All settings should be sent
    std::string_view lDDSString{"setting;ddsonly;true;"};
    EXPECT_CALL(*mSocketWrapperMock, SendTo(lDDSString)).WillOnce(Return(lDDSString.size())).RetiresOnSaturation();

    std::string_view lSetTitleId{"info;titleid;ULES00125;"};
    EXPECT_CALL(*mSocketWrapperMock, SendTo(lSetTitleId)).WillOnce(Return(lSetTitleId.size())).RetiresOnSaturation();

    // After sending the essid, we will close the connection...
    std::string_view lSetESSID{"info;essid;PSP_AULES00125_BOUTLLOB;"};
    EXPECT_CALL(*mSocketWrapperMock, SendTo(lSetESSID)).WillOnce(Return(lSetESSID.size())).RetiresOnSaturation();
}

TEST_F(XLinkKaiConnectionTest, TestReceiverThreadConnectHappyFlow)
{
    std::string_view lIPAddress{"127.0.0.1"};

    bool                        lThreadStopCalled{false};
    bool                        lOpened{false};
    bool                        lEndTest{false};
    int                         lThreadCallCount{0};
    std::function<void(size_t)> lCallBack;
    char*                       lBuffer = nullptr;

    // Incoming connection will return these
    EXPECT_CALL(*mPCapDeviceMock, GetTitleId()).WillRepeatedly(Return(std::string(cDefaultTitleId)));
    EXPECT_CALL(*mPCapDeviceMock, GetESSID()).WillRepeatedly(Return(std::string(cDefaultESSID)));

    // Save the arguments from here so we can use the private callback in xlink kai connection
    EXPECT_CALL(*mSocketWrapperMock, AsyncReceiveFrom(_, _, _))
        .WillRepeatedly(DoAll(SaveArg<0>(&lBuffer), SaveArg<2>(&lCallBack)));

    // General requirements
    EXPECT_CALL(*mSocketWrapperMock, IsOpen()).WillRepeatedly(ReturnPointee(&lOpened));
    EXPECT_CALL(*mSocketWrapperMock, StartThread()).WillRepeatedly(Return());
    EXPECT_CALL(*mSocketWrapperMock, IsThreadStopped()).WillRepeatedly(ReturnPointee(&lThreadStopCalled));

    // Opening the socket
    EXPECT_CALL(*mSocketWrapperMock, Open(lIPAddress, 34523)).WillOnce(DoAll(Assign(&lOpened, true), Return(true)));

    SetConnectionMessages();

    // And ofcourse this should cause a disconnect to be sent to XLink Kai
    std::string_view lDisconnect{"disconnect;"};
    EXPECT_CALL(*mSocketWrapperMock, SendTo(lDisconnect)).WillOnce(Return(lDisconnect.size()));

    // Also gets called in the destructor
    EXPECT_CALL(*mSocketWrapperMock, Close()).Times(2).WillRepeatedly(Assign(&lOpened, false));

    // If the program wants to quit, the test should not stop it from doing so
    EXPECT_CALL(*mSocketWrapperMock, StopThread()).WillRepeatedly(Assign(&lThreadStopCalled, true));

    // Try to sync actions with ReceiverThread
    EXPECT_CALL(*mSocketWrapperMock, PollThread()).WillRepeatedly(Invoke([&] {
        lThreadCallCount++;

        if (lThreadCallCount == 1) {
            // Simulate a connection
            std::string lConnected{"connected;XLHA_Device;XLHA;"};
            strcpy(lBuffer, lConnected.c_str());
            lCallBack(lConnected.size());
        } else if (lThreadCallCount == 2) {
            // Sending Settings
            // Also send a normal packet
            std::string_view lMessage{"e;e;testmessage"};
            EXPECT_CALL(*mSocketWrapperMock, SendTo(lMessage)).WillOnce(Return(lMessage.size()));
            ASSERT_TRUE(mXLinkKaiConnection->Send("testmessage"));
        } else {
            // Stop the connection regardless of success
            lEndTest = true;
        }
    }));

    mXLinkKaiConnection->Open(lIPAddress);
    mXLinkKaiConnection->StartReceiverThread();

    while (!lEndTest) {
        // Wait until the thread is done
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Force connection to close before destructor of google test is called
    mXLinkKaiConnection->Close(true);
    mXLinkKaiConnection = nullptr;
}

// Tests what happens if the connection times out
TEST_F(XLinkKaiConnectionTest, TestReceiverThreadConnectionTimeout)
{
    // Create a mocked timer so we don't have to actually wait in the unittest
    EXPECT_CALL(*mSocketWrapperMock, Close()).RetiresOnSaturation();
    mXLinkKaiConnection = std::make_shared<XLinkKaiConnection>(mSocketWrapperMock, mTimerMock);
    mXLinkKaiConnection->SetIncomingConnection(std::static_pointer_cast<IPCapDevice>(mPCapDeviceMock));

    std::string_view lIPAddress{"127.0.0.1"};

    bool                        lThreadStopCalled{false};
    bool                        lOpened{false};
    bool                        lEndTest{false};
    int                         lThreadCallCount{0};
    std::function<void(size_t)> lCallBack;
    char*                       lBuffer = nullptr;

    // Incoming connection will return these
    EXPECT_CALL(*mPCapDeviceMock, GetTitleId()).WillRepeatedly(Return(std::string(cDefaultTitleId)));
    EXPECT_CALL(*mPCapDeviceMock, GetESSID()).WillRepeatedly(Return(std::string(cDefaultESSID)));

    // Save the arguments from here so we can use the private callback in xlink kai connection
    EXPECT_CALL(*mSocketWrapperMock, AsyncReceiveFrom(_, _, _))
        .WillRepeatedly(DoAll(SaveArg<0>(&lBuffer), SaveArg<2>(&lCallBack)));

    // General requirements
    EXPECT_CALL(*mSocketWrapperMock, IsOpen()).WillRepeatedly(ReturnPointee(&lOpened));
    EXPECT_CALL(*mSocketWrapperMock, StartThread()).WillRepeatedly(Return());
    EXPECT_CALL(*mSocketWrapperMock, IsThreadStopped()).WillRepeatedly(ReturnPointee(&lThreadStopCalled));

    // Opening the socket
    EXPECT_CALL(*mSocketWrapperMock, Open(lIPAddress, 34523))
        .Times(2)
        .WillRepeatedly(DoAll(Assign(&lOpened, true), Return(true)));

    // First connection attempt
    std::string_view lConnectString{"connect;XLHA_Device;XLHA;"};
    EXPECT_CALL(*mSocketWrapperMock, SendTo(lConnectString))
        .WillOnce(Return(lConnectString.size()))
        .RetiresOnSaturation();

    // Second connection attempt
    SetConnectionMessages();

    // And ofcourse this should cause a disconnect to be sent to XLink Kai
    std::string_view lDisconnect{"disconnect;"};
    EXPECT_CALL(*mSocketWrapperMock, SendTo(lDisconnect)).WillOnce(Return(lDisconnect.size()));

    // Also gets called in the destructor
    EXPECT_CALL(*mSocketWrapperMock, Close()).Times(3).WillRepeatedly(Assign(&lOpened, false));

    // If the program wants to quit, the test should not stop it from doing so
    EXPECT_CALL(*mSocketWrapperMock, StopThread()).WillRepeatedly(Assign(&lThreadStopCalled, true));

    // Expect the timer to be restarted when trying to reconnect
    EXPECT_CALL(*mTimerMock, Start(_)).Times(2);
    EXPECT_CALL(*mTimerMock, IsTimedOut()).WillOnce(Return(true));

    // Try to sync actions with ReceiverThread
    EXPECT_CALL(*mSocketWrapperMock, PollThread()).WillRepeatedly(Invoke([&] {
        lThreadCallCount++;

        // (0) First run, connection is initiated
        // (1) Second run, no response, timeout
        // (2) Third run, reconnect
        // (3) Fourth run, send settings
        // (4) Fifth run, disconnect

        if (lThreadCallCount == 2) {
            // Simulate a connection
            std::string lConnected{"connected;XLHA_Device;XLHA;"};
            strcpy(lBuffer, lConnected.c_str());
            lCallBack(lConnected.size());
        } else if (lThreadCallCount == 1 || lThreadCallCount == 3) {
            // Sending settings or doing nothing
        } else {
            // Send a disconnect
            lEndTest = true;
        }
    }));

    mXLinkKaiConnection->Open(lIPAddress);
    mXLinkKaiConnection->StartReceiverThread();

    while (!lEndTest) {
        // Wait until the thread is done
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Force connection to close before destructor of google test is called
    mXLinkKaiConnection->Close(true);
    mXLinkKaiConnection = nullptr;
}

// Tests that XLHA tries to reconnect to XLink Kai when XLink Kai stops responding to keep alives
TEST_F(XLinkKaiConnectionTest, TestReceiverThreadNoXlinkKaiResponse)
{
    // Create a mocked timer so we don't have to actually wait in the unittest
    EXPECT_CALL(*mSocketWrapperMock, Close()).RetiresOnSaturation();
    mXLinkKaiConnection = std::make_shared<XLinkKaiConnection>(mSocketWrapperMock, nullptr, mTimerMock);
    mXLinkKaiConnection->SetIncomingConnection(std::static_pointer_cast<IPCapDevice>(mPCapDeviceMock));

    std::string_view lIPAddress{"127.0.0.1"};

    bool                        lThreadStopCalled{false};
    bool                        lOpened{false};
    bool                        lEndTest{false};
    int                         lThreadCallCount{0};
    std::function<void(size_t)> lCallBack;
    char*                       lBuffer = nullptr;

    // Incoming connection will return these
    EXPECT_CALL(*mPCapDeviceMock, GetTitleId()).WillRepeatedly(Return(std::string(cDefaultTitleId)));
    EXPECT_CALL(*mPCapDeviceMock, GetESSID()).WillRepeatedly(Return(std::string(cDefaultESSID)));

    // Save the arguments from here so we can use the private callback in xlink kai connection
    EXPECT_CALL(*mSocketWrapperMock, AsyncReceiveFrom(_, _, _))
        .WillRepeatedly(DoAll(SaveArg<0>(&lBuffer), SaveArg<2>(&lCallBack)));

    // General requirements
    EXPECT_CALL(*mSocketWrapperMock, IsOpen()).WillRepeatedly(ReturnPointee(&lOpened));
    EXPECT_CALL(*mSocketWrapperMock, StartThread()).WillRepeatedly(Return());
    EXPECT_CALL(*mSocketWrapperMock, IsThreadStopped()).WillRepeatedly(ReturnPointee(&lThreadStopCalled));

    // Opening the socket
    EXPECT_CALL(*mSocketWrapperMock, Open(lIPAddress, 34523))
        .Times(2)
        .WillRepeatedly(DoAll(Assign(&lOpened, true), Return(true)));

    // Connecting, X2
    SetConnectionMessages();
    SetConnectionMessages();

    // And ofcourse this should cause a disconnect to be sent to XLink Kai
    std::string_view lDisconnect{"disconnect;"};
    EXPECT_CALL(*mSocketWrapperMock, SendTo(lDisconnect)).WillOnce(Return(lDisconnect.size()));

    // Also gets called in the destructor
    EXPECT_CALL(*mSocketWrapperMock, Close()).Times(3).WillRepeatedly(Assign(&lOpened, false));

    // If the program wants to quit, the test should not stop it from doing so
    EXPECT_CALL(*mSocketWrapperMock, StopThread()).WillRepeatedly(Assign(&lThreadStopCalled, true));

    // Expect the timer to be restarted when data received,
    // Those would be answers to 2x a connection request.
    EXPECT_CALL(*mTimerMock, Start(_)).Times(2);
    EXPECT_CALL(*mTimerMock, IsTimedOut()).WillOnce(Return(false)).WillOnce(Return(true)).WillRepeatedly(Return(false));

    // Try to sync actions with ReceiverThread
    EXPECT_CALL(*mSocketWrapperMock, PollThread()).WillRepeatedly(Invoke([&] {
        lThreadCallCount++;

        // (0) First run, connection is initiated
        // (1) Second run, connected
        // (2) Third run, settings sent
        // (3) Fourth run, XLink Kai stopped responding, timer timed out
        // (4) Fifth run, reconnection attempt
        // (5) Sixth run, connection succeeded, disconnect

        if (lThreadCallCount == 1 || lThreadCallCount == 4) {
            // Simulate a connection
            std::string lConnected{"connected;XLHA_Device;XLHA;"};
            strcpy(lBuffer, lConnected.c_str());
            lCallBack(lConnected.size());
        } else if (lThreadCallCount == 2 || lThreadCallCount == 3) {
            // Sending settings or doing nothing
        } else {
            // Send a disconnect
            lEndTest = true;
        }
    }));

    mXLinkKaiConnection->Open(lIPAddress);
    mXLinkKaiConnection->StartReceiverThread();

    while (!lEndTest) {
        // Wait until the thread is done
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Force connection to close before destructor of google test is called
    mXLinkKaiConnection->Close(true);
    mXLinkKaiConnection = nullptr;
}

// Tests that a proper response is given to a keepalive
TEST_F(XLinkKaiConnectionTest, TestReceiverThreadKeepAlive)
{
    // Create a mocked timer so we don't have to actually wait in the unittest
    EXPECT_CALL(*mSocketWrapperMock, Close()).RetiresOnSaturation();
    mXLinkKaiConnection = std::make_shared<XLinkKaiConnection>(mSocketWrapperMock, nullptr, mTimerMock);
    mXLinkKaiConnection->SetIncomingConnection(std::static_pointer_cast<IPCapDevice>(mPCapDeviceMock));

    std::string_view lIPAddress{"127.0.0.1"};

    bool                        lThreadStopCalled{false};
    bool                        lOpened{false};
    bool                        lEndTest{false};
    int                         lThreadCallCount{0};
    std::function<void(size_t)> lCallBack;
    char*                       lBuffer = nullptr;

    // Incoming connection will return these
    EXPECT_CALL(*mPCapDeviceMock, GetTitleId()).WillRepeatedly(Return(std::string(cDefaultTitleId)));
    EXPECT_CALL(*mPCapDeviceMock, GetESSID()).WillRepeatedly(Return(std::string(cDefaultESSID)));

    // Save the arguments from here so we can use the private callback in xlink kai connection
    EXPECT_CALL(*mSocketWrapperMock, AsyncReceiveFrom(_, _, _))
        .WillRepeatedly(DoAll(SaveArg<0>(&lBuffer), SaveArg<2>(&lCallBack)));

    // General requirements
    EXPECT_CALL(*mSocketWrapperMock, IsOpen()).WillRepeatedly(ReturnPointee(&lOpened));
    EXPECT_CALL(*mSocketWrapperMock, StartThread()).WillRepeatedly(Return());
    EXPECT_CALL(*mSocketWrapperMock, IsThreadStopped()).WillRepeatedly(ReturnPointee(&lThreadStopCalled));

    // Opening the socket
    EXPECT_CALL(*mSocketWrapperMock, Open(lIPAddress, 34523)).WillOnce(DoAll(Assign(&lOpened, true), Return(true)));

    SetConnectionMessages();

    // And ofcourse this should cause a disconnect to be sent to XLink Kai
    std::string_view lDisconnect{"disconnect;"};
    EXPECT_CALL(*mSocketWrapperMock, SendTo(lDisconnect)).WillOnce(Return(lDisconnect.size()));

    // Also gets called in the destructor
    EXPECT_CALL(*mSocketWrapperMock, Close()).Times(2).WillRepeatedly(Assign(&lOpened, false));

    // If the program wants to quit, the test should not stop it from doing so
    EXPECT_CALL(*mSocketWrapperMock, StopThread()).WillRepeatedly(Assign(&lThreadStopCalled, true));

    // 2 starts, one on connect, the other on the keepalive
    EXPECT_CALL(*mTimerMock, Start(_)).Times(2);
    EXPECT_CALL(*mTimerMock, IsTimedOut()).WillRepeatedly(Return(false));

    // Try to sync actions with ReceiverThread
    EXPECT_CALL(*mSocketWrapperMock, PollThread()).WillRepeatedly(Invoke([&] {
        lThreadCallCount++;

        if (lThreadCallCount == 1) {
            // Simulate a connection
            std::string lConnected{"connected;XLHA_Device;XLHA;"};
            strcpy(lBuffer, lConnected.c_str());
            lCallBack(lConnected.size());
        } else if (lThreadCallCount == 2) {
            // Sending Settings
        } else if (lThreadCallCount == 3) {

            // Should send keepalive back when keepalive gotten from xlink kai
            EXPECT_CALL(*mSocketWrapperMock, SendTo(cFullKeepAliveString)).WillOnce(Return(true));

            // Keep alive from xlink kai
            std::string lKeepAlive{cFullKeepAliveString};
            strcpy(lBuffer, lKeepAlive.c_str());
            lCallBack(lKeepAlive.size());
        } else {
            // Stop the connection regardless of success
            lEndTest = true;
        }
    }));

    mXLinkKaiConnection->Open(lIPAddress);
    mXLinkKaiConnection->StartReceiverThread();

    while (!lEndTest) {
        // Wait until the thread is done
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Force connection to close before destructor of google test is called
    mXLinkKaiConnection->Close(true);
    mXLinkKaiConnection = nullptr;
}

// Tests that normal traffic is passed normally
TEST_F(XLinkKaiConnectionTest, TestReceiverThreadNormalData)
{
    // Create a mocked timer so we don't have to actually wait in the unittest
    EXPECT_CALL(*mSocketWrapperMock, Close()).RetiresOnSaturation();
    mXLinkKaiConnection = std::make_shared<XLinkKaiConnection>(mSocketWrapperMock, nullptr, mTimerMock);
    mXLinkKaiConnection->SetIncomingConnection(std::static_pointer_cast<IPCapDevice>(mPCapDeviceMock));

    std::string_view lIPAddress{"127.0.0.1"};

    bool                        lThreadStopCalled{false};
    bool                        lOpened{false};
    bool                        lEndTest{false};
    int                         lThreadCallCount{0};
    std::function<void(size_t)> lCallBack;
    char*                       lBuffer = nullptr;

    // Incoming connection will return these
    EXPECT_CALL(*mPCapDeviceMock, GetTitleId()).WillRepeatedly(Return(std::string(cDefaultTitleId)));
    EXPECT_CALL(*mPCapDeviceMock, GetESSID()).WillRepeatedly(Return(std::string(cDefaultESSID)));

    // Save the arguments from here so we can use the private callback in xlink kai connection
    EXPECT_CALL(*mSocketWrapperMock, AsyncReceiveFrom(_, _, _))
        .WillRepeatedly(DoAll(SaveArg<0>(&lBuffer), SaveArg<2>(&lCallBack)));

    // General requirements
    EXPECT_CALL(*mSocketWrapperMock, IsOpen()).WillRepeatedly(ReturnPointee(&lOpened));
    EXPECT_CALL(*mSocketWrapperMock, StartThread()).WillRepeatedly(Return());
    EXPECT_CALL(*mSocketWrapperMock, IsThreadStopped()).WillRepeatedly(ReturnPointee(&lThreadStopCalled));

    // Opening the socket
    EXPECT_CALL(*mSocketWrapperMock, Open(lIPAddress, 34523)).WillOnce(DoAll(Assign(&lOpened, true), Return(true)));

    SetConnectionMessages();

    // And ofcourse this should cause a disconnect to be sent to XLink Kai
    std::string_view lDisconnect{"disconnect;"};
    EXPECT_CALL(*mSocketWrapperMock, SendTo(lDisconnect)).WillOnce(Return(lDisconnect.size()));

    // Also gets called in the destructor
    EXPECT_CALL(*mSocketWrapperMock, Close()).Times(2).WillRepeatedly(Assign(&lOpened, false));

    // If the program wants to quit, the test should not stop it from doing so
    EXPECT_CALL(*mSocketWrapperMock, StopThread()).WillRepeatedly(Assign(&lThreadStopCalled, true));

    // 2 starts, one on connect, the other on the data
    EXPECT_CALL(*mTimerMock, Start(_)).Times(2);
    EXPECT_CALL(*mTimerMock, IsTimedOut()).WillRepeatedly(Return(false));

    // Try to sync actions with ReceiverThread
    EXPECT_CALL(*mSocketWrapperMock, PollThread()).WillRepeatedly(Invoke([&] {
        lThreadCallCount++;

        if (lThreadCallCount == 1) {
            // Simulate a connection
            std::string lConnected{"connected;XLHA_Device;XLHA;"};
            strcpy(lBuffer, lConnected.c_str());
            lCallBack(lConnected.size());
        } else if (lThreadCallCount == 2) {
            // Sending Settings
        } else if (lThreadCallCount == 3) {
            // Data from xlink kai, hard to read, but will do the job, essentially a psp broadcast
            // Data itself is 172 bytes,then we add +4 for e;e;
            std::array<unsigned char, 176> lData{
                0x65, 0x3b, 0x65, 0x3b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x18, 0xf8, 0x29, 0x3f, 0xb0,
                0x88, 0xc8, 0x00, 0x01, 0x01, 0x02, 0x00, 0x80, 0x63, 0x6f, 0x64, 0x65, 0x64, 0x77, 0x72, 0x65,
                0x6e, 0x63, 0x68, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x04, 0x10, 0x00, 0x06, 0x06, 0x02, 0x04,
                0x10, 0x00, 0x08, 0x02, 0x03, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x06, 0xf5, 0x2d, 0x55, 0x29};

            // Skip e;e; for the expected sent over the WiFI adapter result
            std::string_view lDataView{reinterpret_cast<char*>(&lData.at(4)), lData.size() - 4};

            //  Should pass data to incoming connection and blacklist the source mac from being sent trhough the other
            //  side
            EXPECT_CALL(*mPCapDeviceMock, BlackList(0xb03f29f81800));
            EXPECT_CALL(*mPCapDeviceMock, Send(lDataView));

            memcpy(lBuffer, lData.data(), lData.size());
            lCallBack(lData.size());

        } else {
            // Stop the connection regardless of success
            lEndTest = true;
        }
    }));

    mXLinkKaiConnection->Open(lIPAddress);
    mXLinkKaiConnection->StartReceiverThread();

    while (!lEndTest) {
        // Wait until the thread is done
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Force connection to close before destructor of google test is called
    mXLinkKaiConnection->Close(true);
    mXLinkKaiConnection = nullptr;
}

// Tests that normal traffic is passed normally to a monitor device
TEST_F(XLinkKaiConnectionTest, TestReceiverThreadMonitorData)
{
    std::shared_ptr<MonitorDeviceDerived> lMonitorDeviceDerived{std::make_shared<MonitorDeviceDerived>()};

    // Create a mocked timer so we don't have to actually wait in the unittest
    EXPECT_CALL(*mSocketWrapperMock, Close()).RetiresOnSaturation();
    mXLinkKaiConnection = std::make_shared<XLinkKaiConnection>(mSocketWrapperMock, nullptr, mTimerMock);
    mXLinkKaiConnection->SetIncomingConnection(
        std::static_pointer_cast<IPCapDevice>(static_pointer_cast<IPCapDeviceMock>(lMonitorDeviceDerived)));

    std::string_view lIPAddress{"127.0.0.1"};

    bool                        lThreadStopCalled{false};
    bool                        lOpened{false};
    bool                        lEndTest{false};
    int                         lThreadCallCount{0};
    std::function<void(size_t)> lCallBack;
    char*                       lBuffer = nullptr;

    // Incoming connection will return these
    EXPECT_CALL(*lMonitorDeviceDerived, GetTitleId()).WillRepeatedly(Return(std::string(cDefaultTitleId)));
    EXPECT_CALL(*lMonitorDeviceDerived, GetESSID()).WillRepeatedly(Return(std::string(cDefaultESSID)));

    // Save the arguments from here so we can use the private callback in xlink kai connection
    EXPECT_CALL(*mSocketWrapperMock, AsyncReceiveFrom(_, _, _))
        .WillRepeatedly(DoAll(SaveArg<0>(&lBuffer), SaveArg<2>(&lCallBack)));

    // General requirements
    EXPECT_CALL(*mSocketWrapperMock, IsOpen()).WillRepeatedly(ReturnPointee(&lOpened));
    EXPECT_CALL(*mSocketWrapperMock, StartThread()).WillRepeatedly(Return());
    EXPECT_CALL(*mSocketWrapperMock, IsThreadStopped()).WillRepeatedly(ReturnPointee(&lThreadStopCalled));

    // Opening the socket
    EXPECT_CALL(*mSocketWrapperMock, Open(lIPAddress, 34523)).WillOnce(DoAll(Assign(&lOpened, true), Return(true)));

    SetConnectionMessages();

    // And ofcourse this should cause a disconnect to be sent to XLink Kai
    std::string_view lDisconnect{"disconnect;"};
    EXPECT_CALL(*mSocketWrapperMock, SendTo(lDisconnect)).WillOnce(Return(lDisconnect.size()));

    // Also gets called in the destructor
    EXPECT_CALL(*mSocketWrapperMock, Close()).Times(2).WillRepeatedly(Assign(&lOpened, false));

    // If the program wants to quit, the test should not stop it from doing so
    EXPECT_CALL(*mSocketWrapperMock, StopThread()).WillRepeatedly(Assign(&lThreadStopCalled, true));

    // 2 starts, one on connect, the other on the data
    EXPECT_CALL(*mTimerMock, Start(_)).Times(2);
    EXPECT_CALL(*mTimerMock, IsTimedOut()).WillRepeatedly(Return(false));

    // Try to sync actions with ReceiverThread
    EXPECT_CALL(*mSocketWrapperMock, PollThread()).WillRepeatedly(Invoke([&] {
        lThreadCallCount++;

        if (lThreadCallCount == 1) {
            // Simulate a connection
            std::string lConnected{"connected;XLHA_Device;XLHA;"};
            strcpy(lBuffer, lConnected.c_str());
            lCallBack(lConnected.size());
        } else if (lThreadCallCount == 2) {
            // Sending Settings
        } else if (lThreadCallCount == 3) {
            // Data from xlink kai, hard to read, but will do the job, essentially a psp broadcast
            // Data itself is 172 bytes,then we add +4 for e;e;
            std::array<unsigned char, 176> lData{
                0x65, 0x3b, 0x65, 0x3b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x18, 0xf8, 0x29, 0x3f, 0xb0,
                0x88, 0xc8, 0x00, 0x01, 0x01, 0x02, 0x00, 0x80, 0x63, 0x6f, 0x64, 0x65, 0x64, 0x77, 0x72, 0x65,
                0x6e, 0x63, 0x68, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x04, 0x10, 0x00, 0x06, 0x06, 0x02, 0x04,
                0x10, 0x00, 0x08, 0x02, 0x03, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x06, 0xf5, 0x2d, 0x55, 0x29};

            // We're looking at (default options):
            // Radiotap version (0x00)
            // Header pad (0x00)
            // Radiotap header length 16 (0x10 0x00)
            // Default present flags (Flags, Rate, Channel and TX Flags) (0x00 0x00 0x80 0x0e)
            // Flags: Short Preamble (0x02)
            // Rate flags: 24mb/s (0x30)
            // Channel: 2412mhz (0x6c 0x09)
            // Channel Flags: 2.4Ghz, Turbo on (0xc0 0x00)
            // Tx Flags: No Ack (0x08 0x00)
            // FC Type: Data (0x08 0x00)
            // Duration id: (0xff 0xff)
            // Destination adress: (0xff 0xff 0xff 0xff 0xff 0xff)
            // Source address: (0x00 0x18 0xf8 0x29 0x3f 0xb0)
            // BSSID (not set so all zeroes): (0x00 0x00 0x00 0x00 0x00 0x00)
            // FCS (0x00 0x00)
            // Snap LLC: (0xaa 0xaa 0x03)
            // Organization Code (0x00 0x00 0x00)
            // Type (0x88 0xc8)
            std::array<unsigned char, 48> lMonitor{
                0x00, 0x00, 0x10, 0x00, 0x0e, 0x80, 0x00, 0x00, 0x02, 0x30, 0x6c, 0x09, 0xc0, 0x00, 0x08, 0x00,
                0x08, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x18, 0xf8, 0x29, 0x3f, 0xb0,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00, 0x88, 0xc8};

            // Skip e;e; and ethernet header for the expected sent over the WiFI adapter result
            std::string      lHeaderString{reinterpret_cast<char*>(lMonitor.data()), lMonitor.size()};
            std::string      lDataString{reinterpret_cast<char*>(&lData.at(18)), lData.size() - 18};
            std::string      lCompleteString{lHeaderString + lDataString};
            std::string_view lDataView{lCompleteString};

            // Should pass data to incoming connection and blacklist the source mac from being sent through the other
            // side
            EXPECT_CALL(*lMonitorDeviceDerived, BlackList(0xb03f29f81800));
            EXPECT_CALL(*lMonitorDeviceDerived, Send(lDataView));

            memcpy(lBuffer, lData.data(), lData.size());
            lCallBack(lData.size());

        } else {
            // Stop the connection regardless of success
            lEndTest = true;
        }
    }));

    mXLinkKaiConnection->Open(lIPAddress);
    mXLinkKaiConnection->StartReceiverThread();

    while (!lEndTest) {
        // Wait until the thread is done
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Force connection to close before destructor of google test is called
    mXLinkKaiConnection->Close(true);
    mXLinkKaiConnection = nullptr;
}

// Tests that XLHA does not send anything when the socket is closed
TEST_F(XLinkKaiConnectionTest, TestNoSendOnClosedSocket)
{
    EXPECT_CALL(*mSocketWrapperMock, Close()).RetiresOnSaturation();

    EXPECT_CALL(*mSocketWrapperMock, IsOpen()).WillOnce(Return(false));
    EXPECT_CALL(*mSocketWrapperMock, SendTo(_)).Times(0);

    ASSERT_FALSE(mXLinkKaiConnection->Send("testmessage"));
}

// Tests that XLHA does not send anything besides connection requests when xlink kai is not connected yet
TEST_F(XLinkKaiConnectionTest, TestNoSendOnNoConnection)
{
    EXPECT_CALL(*mSocketWrapperMock, Close()).RetiresOnSaturation();

    EXPECT_CALL(*mSocketWrapperMock, IsOpen()).WillOnce(Return(true));
    EXPECT_CALL(*mSocketWrapperMock, SendTo(_)).Times(0);

    ASSERT_FALSE(mXLinkKaiConnection->Send("testmessage"));
}

// Tests that XLHA can send connection requests when no connection has been made yet
TEST_F(XLinkKaiConnectionTest, TestSendConnectionRequestOnNoConnection)
{
    EXPECT_CALL(*mSocketWrapperMock, Close()).RetiresOnSaturation();

    std::string_view lConnectString{"connect;XLHA_Device;XLHA;"};

    EXPECT_CALL(*mSocketWrapperMock, IsOpen()).WillOnce(Return(true));
    EXPECT_CALL(*mSocketWrapperMock, SendTo(lConnectString)).WillOnce(Return(lConnectString.size()));

    ASSERT_TRUE(mXLinkKaiConnection->Send("connect;XLHA_Device;XLHA;", ""));
}


// Tests that XLHA can send disconnection requests when no connection has been made
TEST_F(XLinkKaiConnectionTest, TestSendDisconnectionRequestOnNoConnection)
{
    EXPECT_CALL(*mSocketWrapperMock, Close()).RetiresOnSaturation();

    std::string_view lDisconnectString{"disconnect;"};

    EXPECT_CALL(*mSocketWrapperMock, IsOpen()).WillOnce(Return(true));
    EXPECT_CALL(*mSocketWrapperMock, SendTo(lDisconnectString)).WillOnce(Return(lDisconnectString.size()));

    ASSERT_TRUE(mXLinkKaiConnection->Send("disconnect;", ""));
}

// Tests that an incoming SSID switch gets sent to the pcap device
TEST_F(XLinkKaiConnectionTest, TestSSIDSwitchFromXLinkKai)
{
    std::string_view lIPAddress{"127.0.0.1"};

    bool                        lThreadStopCalled{false};
    bool                        lOpened{false};
    bool                        lEndTest{false};
    int                         lThreadCallCount{0};
    std::function<void(size_t)> lCallBack;
    char*                       lBuffer = nullptr;

    // Incoming connection will return these
    EXPECT_CALL(*mPCapDeviceMock, GetTitleId()).WillRepeatedly(Return(std::string(cDefaultTitleId)));
    EXPECT_CALL(*mPCapDeviceMock, GetESSID()).WillRepeatedly(Return(std::string(cDefaultESSID)));

    // Save the arguments from here so we can use the private callback in xlink kai connection
    EXPECT_CALL(*mSocketWrapperMock, AsyncReceiveFrom(_, _, _))
        .WillRepeatedly(DoAll(SaveArg<0>(&lBuffer), SaveArg<2>(&lCallBack)));

    // General requirements
    EXPECT_CALL(*mSocketWrapperMock, IsOpen()).WillRepeatedly(ReturnPointee(&lOpened));
    EXPECT_CALL(*mSocketWrapperMock, StartThread()).WillRepeatedly(Return());
    EXPECT_CALL(*mSocketWrapperMock, IsThreadStopped()).WillRepeatedly(ReturnPointee(&lThreadStopCalled));

    // Opening the socket
    EXPECT_CALL(*mSocketWrapperMock, Open(lIPAddress, 34523)).WillOnce(DoAll(Assign(&lOpened, true), Return(true)));

    SetConnectionMessages();

    // And ofcourse this should cause a disconnect to be sent to XLink Kai
    std::string_view lDisconnect{"disconnect;"};
    EXPECT_CALL(*mSocketWrapperMock, SendTo(lDisconnect)).WillOnce(Return(lDisconnect.size()));

    // Also gets called in the destructor
    EXPECT_CALL(*mSocketWrapperMock, Close()).Times(2).WillRepeatedly(Assign(&lOpened, false));

    // If the program wants to quit, the test should not stop it from doing so
    EXPECT_CALL(*mSocketWrapperMock, StopThread()).WillRepeatedly(Assign(&lThreadStopCalled, true));

    // Tell XLinkConnection instance we are interested in SSIDs
    mXLinkKaiConnection->SetUseHostSSID(true);

    // Try to sync actions with ReceiverThread
    EXPECT_CALL(*mSocketWrapperMock, PollThread()).WillRepeatedly(Invoke([&] {
        lThreadCallCount++;

        if (lThreadCallCount == 1) {
            // Simulate a connection
            std::string lConnected{"connected;XLHA_Device;XLHA;"};
            strcpy(lBuffer, lConnected.c_str());
            lCallBack(lConnected.size());
        } else if (lThreadCallCount == 2) {
            // Sending Settings
        } else if (lThreadCallCount == 3) {
            // Incoming ssid switch from another XLink Kai instance.
            EXPECT_CALL(*mPCapDeviceMock, Connect(cDefaultESSID));

            std::string lSetESSID{"e;d;setessid;PSP_AULES00125_BOUTLLOB;"};
            strcpy(lBuffer, lSetESSID.c_str());
            lCallBack(lSetESSID.size());
        } else {
            // Stop the connection regardless of success
            lEndTest = true;
        }
    }));

    mXLinkKaiConnection->Open(lIPAddress);
    mXLinkKaiConnection->StartReceiverThread();

    while (!lEndTest) {
        // Wait until the thread is done
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Force connection to close before destructor of google test is called
    mXLinkKaiConnection->Close(true);
    mXLinkKaiConnection = nullptr;
}