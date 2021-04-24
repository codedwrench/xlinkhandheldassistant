/* Copyright (c) 2021 [Rick de Bondt] - PacketConverter_Test.cpp
 * This file contains a mock for IConnector.
 **/

class IConnectorMock : public IConnector
{
public:
    MOCK_METHOD(bool, Open, (std::string_view aArgument));
    MOCK_METHOD(void, Close, ());
    MOCK_METHOD(std::string, LastDataToString, ());
    MOCK_METHOD(bool, ReadNextData, ());
    MOCK_METHOD(bool, Send, (std::string_view aData));
    MOCK_METHOD(bool, Send, (std::string_view aCommand, std::string_view aData));
    MOCK_METHOD(void, SetIncomingConnection, (std::shared_ptr<IPCapDevice> aDevice));
    MOCK_METHOD(bool, StartReceiverThread, ());
};
