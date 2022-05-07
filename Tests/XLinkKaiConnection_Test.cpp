/* Copyright (c) 2020 [Rick de Bondt] - WindowModel_Test.cpp
 * This file contains tests for loading or saving WindowModel members.
 **/

#include "XLinkKaiConnection.h"

#include <functional>
#include <memory>

#include <gmock/gmock-actions.h>
#include <gtest/gtest.h>

#include "IPCapDeviceMock.h"
#include "IUDPSocketWrapperMock.h"

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

    XLinkKaiConnection mXLinkKaiConnection{std::static_pointer_cast<IUDPSocketWrapper>(mSocketWrapperMock)};

    XLinkKaiConnectionTest()
    {
        mXLinkKaiConnection.SetIncomingConnection(std::static_pointer_cast<IPCapDevice>(mPCapDeviceMock));
    }
};

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

    // Connction to XLink Kai
    std::string_view lConnectString{"connect;XLHA_Device;XLHA;"};
    EXPECT_CALL(*mSocketWrapperMock, SendTo(lConnectString)).WillOnce(Return(lConnectString.size()));

    // All settings should be sent
    std::string_view lDDSString{"setting;ddsonly;true;"};
    EXPECT_CALL(*mSocketWrapperMock, SendTo(lDDSString)).WillOnce(Return(lDDSString.size()));

    std::string_view lSetTitleId{"info;titleid;ULES00125;"};
    EXPECT_CALL(*mSocketWrapperMock, SendTo(lSetTitleId)).WillOnce(Return(lSetTitleId.size()));

    // After sending the essid, we will close the connection...
    std::string_view lSetESSID{"info;essid;PSP_AULES00125_BOUTLLOB;"};
    EXPECT_CALL(*mSocketWrapperMock, SendTo(lSetESSID)).WillOnce(Return(lSetESSID.size()));

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

    mXLinkKaiConnection.Open(lIPAddress);
    mXLinkKaiConnection.StartReceiverThread();

    while (!lEndTest) {
        // Wait until the thread is done
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Force connection to close before destructor of google test is called
    mXLinkKaiConnection.Close(true);
}
