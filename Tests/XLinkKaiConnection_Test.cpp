/* Copyright (c) 2020 [Rick de Bondt] - WindowModel_Test.cpp
 * This file contains tests for loading or saving WindowModel members.
 **/

#include "XLinkKaiConnection.h"

#include <functional>
#include <memory>

#include <gmock/gmock-actions.h>
#include <gmock/gmock-spec-builders.h>
#include <gtest/gtest.h>

#include "IPCapDeviceMock.h"
#include "IUDPSocketWrapperMock.h"
#include "ITimerMock.h"

using testing::_;
using testing::Assign;
using testing::DoAll;
using testing::Invoke;
using testing::Return;
using testing::ReturnPointee;
using testing::SaveArg;

class XLinkKaiConnectionTest : public ::testing::Test
{
public:
    std::shared_ptr<IUDPSocketWrapperMock> mSocketWrapperMock{std::make_shared<IUDPSocketWrapperMock>()};
    std::shared_ptr<IPCapDeviceMock>       mPCapDeviceMock{std::make_shared<IPCapDeviceMock>()};
    std::shared_ptr<ITimerMock>            mConnectionTimerMock{std::make_shared<ITimerMock>()};
    std::shared_ptr<ITimerMock>            mKeepAliveTimerMock{std::make_shared<ITimerMock>()};

    std::shared_ptr<XLinkKaiConnection>    mXLinkKaiConnection{std::make_shared<XLinkKaiConnection>(std::static_pointer_cast<IUDPSocketWrapper>(mSocketWrapperMock))};

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
    EXPECT_CALL(*mSocketWrapperMock, SendTo(lConnectString)).WillOnce(Return(lConnectString.size())).RetiresOnSaturation();

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
    EXPECT_CALL(*mPCapDeviceMock, GetTitleId()).WillRepeatedly(Return("ULES00125"));
    EXPECT_CALL(*mPCapDeviceMock, GetESSID()).WillRepeatedly(Return("PSP_AULES00125_BOUTLLOB"));

    // Save the arguments from here so we can use the private callback in xlink kai connection
    EXPECT_CALL(*mSocketWrapperMock, AsyncReceiveFrom(_, _, _))
        .WillRepeatedly(DoAll(SaveArg<0>(&lBuffer), SaveArg<2>(&lCallBack)));

    // General requirements
    EXPECT_CALL(*mSocketWrapperMock, IsOpen()).WillRepeatedly(ReturnPointee(&lOpened));
    EXPECT_CALL(*mSocketWrapperMock, StartThread()).WillRepeatedly(Return());
    EXPECT_CALL(*mSocketWrapperMock, IsThreadStopped()).WillRepeatedly(ReturnPointee(&lThreadStopCalled));

    // Opening the socket
    EXPECT_CALL(*mSocketWrapperMock, Open(lIPAddress, 34523)).WillOnce(DoAll(Assign(&lOpened, true), 
    Return(true)));

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
    mXLinkKaiConnection = std::make_shared<XLinkKaiConnection>(mSocketWrapperMock, mConnectionTimerMock);
    mXLinkKaiConnection->SetIncomingConnection(std::static_pointer_cast<IPCapDevice>(mPCapDeviceMock));

    std::string_view lIPAddress{"127.0.0.1"};
    
    bool                        lThreadStopCalled{false};
    bool                        lOpened{false};
    bool                        lEndTest{false};
    int                         lThreadCallCount{0};
    std::function<void(size_t)> lCallBack;
    char*                       lBuffer = nullptr;

    // Incoming connection will return these
    EXPECT_CALL(*mPCapDeviceMock, GetTitleId()).WillRepeatedly(Return("ULES00125"));
    EXPECT_CALL(*mPCapDeviceMock, GetESSID()).WillRepeatedly(Return("PSP_AULES00125_BOUTLLOB"));

    // Save the arguments from here so we can use the private callback in xlink kai connection
    EXPECT_CALL(*mSocketWrapperMock, AsyncReceiveFrom(_, _, _))
        .WillRepeatedly(DoAll(SaveArg<0>(&lBuffer), SaveArg<2>(&lCallBack)));

    // General requirements
    EXPECT_CALL(*mSocketWrapperMock, IsOpen()).WillRepeatedly(ReturnPointee(&lOpened));
    EXPECT_CALL(*mSocketWrapperMock, StartThread()).WillRepeatedly(Return());
    EXPECT_CALL(*mSocketWrapperMock, IsThreadStopped()).WillRepeatedly(ReturnPointee(&lThreadStopCalled));

    // Opening the socket
    EXPECT_CALL(*mSocketWrapperMock, Open(lIPAddress, 34523)).Times(2).WillRepeatedly(DoAll(Assign(&lOpened, true), 
    Return(true)));

    // First connection attempt
    std::string_view lConnectString{"connect;XLHA_Device;XLHA;"};
    EXPECT_CALL(*mSocketWrapperMock, SendTo(lConnectString)).WillOnce(Return(lConnectString.size())).RetiresOnSaturation();
    
    // Second connection attempt
    SetConnectionMessages();

    // And ofcourse this should cause a disconnect to be sent to XLink Kai
    std::string_view lDisconnect{"disconnect;"};
    EXPECT_CALL(*mSocketWrapperMock, SendTo(lDisconnect)).WillOnce(Return(lDisconnect.size()));

    // Also gets called in the destructor
    EXPECT_CALL(*mSocketWrapperMock, Close()).Times(3).WillRepeatedly(Assign(&lOpened, false));

    // If the program wants to quit, the test should not stop it from doing so
    EXPECT_CALL(*mSocketWrapperMock, StopThread()).WillRepeatedly(Assign(&lThreadStopCalled, true));

    EXPECT_CALL(*mConnectionTimerMock, Start(_));
    EXPECT_CALL(*mConnectionTimerMock, IsTimedOut()).WillOnce(Return(true));

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
        } else if (lThreadCallCount == 1 || 
                   lThreadCallCount == 3) {
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
