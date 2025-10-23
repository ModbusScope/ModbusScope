
#include "tst_communication.h"

#include "communication/graphdatahandler.h"
#include "communication/modbuspoll.h"
#include "communicationhelpers.h"
#include "models/graphdatamodel.h"
#include "models/settingsmodel.h"

#include "testslavemodbus.h"

#include <QSignalSpy>
#include <QTest>

using State = ResultState::State;

void TestCommunication::init()
{
    _pSettingsModel = new SettingsModel;
    _pGraphDataModel = new GraphDataModel(_pSettingsModel);

    auto connData = _pSettingsModel->connectionSettings(ConnectionTypes::ID_1);
    connData->setIpAddress("127.0.0.1");
    connData->setPort(5020);
    connData->setTimeout(500);

    _pSettingsModel->setConnectionState(ConnectionTypes::ID_2, true);
    connData = _pSettingsModel->connectionSettings(ConnectionTypes::ID_2);
    connData->setIpAddress("127.0.0.1");
    connData->setPort(5021);
    connData->setTimeout(500);

    _pSettingsModel->setConnectionState(ConnectionTypes::ID_3, true);
    connData = _pSettingsModel->connectionSettings(ConnectionTypes::ID_3);
    connData->setIpAddress("127.0.0.1");
    connData->setPort(5022);
    connData->setTimeout(500);

    deviceId_t devId = Device::cFirstDeviceId;
    _pSettingsModel->addDevice(devId);
    _pSettingsModel->deviceSettings(devId)->setConnectionId(ConnectionTypes::ID_1);
    _pSettingsModel->deviceSettings(devId)->setSlaveId(ConnectionTypes::ID_1 + 1); // TODO: dev: dirty hack

    devId++;
    _pSettingsModel->addDevice(devId);
    _pSettingsModel->deviceSettings(devId)->setConnectionId(ConnectionTypes::ID_2);
    _pSettingsModel->deviceSettings(devId)->setSlaveId(ConnectionTypes::ID_1 + 2);

    devId++;
    _pSettingsModel->addDevice(devId);
    _pSettingsModel->deviceSettings(devId)->setConnectionId(ConnectionTypes::ID_3);
    _pSettingsModel->deviceSettings(devId)->setSlaveId(ConnectionTypes::ID_1 + 3);

    _pSettingsModel->setPollTime(100);

    const auto deviceList = _pSettingsModel->deviceList();
    for (deviceId_t devId : std::as_const(deviceList))
    {
        auto device = _pSettingsModel->deviceSettings(devId);

        connData = _pSettingsModel->connectionSettings(device->connectionId());

        _testSlaveMap[devId] = new TestSlaveModbus();
        auto& testSlave = _testSlaveMap[devId];

        QVERIFY(testSlave->connect(connData->ipAddress(), connData->port(), device->slaveId()));
    }
}

void TestCommunication::cleanup()
{
    for (auto& testSlave : _testSlaveMap)
    {
        testSlave->disconnect();
        delete testSlave;
    }

    delete _pGraphDataModel;
    delete _pSettingsModel;
}

void TestCommunication::singleSlaveSuccess()
{
    auto exprList = QStringList() << "${40002}"
                                  << "${40001}";
    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto device = _testSlaveMap[Device::cFirstDeviceId]->testDevice();
    device->configureHoldingRegister(0, true, 1);
    device->configureHoldingRegister(1, true, 2);

    auto resultList = ResultDoubleList() << ResultDouble(2, State::SUCCESS) << ResultDouble(1, State::SUCCESS);

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList);
}

void TestCommunication::typeFloat32()
{
    auto exprList = QStringList() << "${40001: f32b}";
    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto device = _testSlaveMap[Device::cFirstDeviceId]->testDevice();
    device->configureHoldingRegister(0, true, 0xffff);
    device->configureHoldingRegister(1, true, 0x3f7f);

    auto resultList = ResultDoubleList() << ResultDouble(static_cast<double>(0.999999940395355225f), State::SUCCESS);

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList);
}

void TestCommunication::constantExpression()
{
    auto exprList = QStringList() << "3";
    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto resultList = ResultDoubleList() << ResultDouble(3, State::SUCCESS);

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList);
}

void TestCommunication::mixed_1()
{
    auto exprList = QStringList() << "${40002@2} + ${40003}"
                                  << "2"
                                  << "${40004:32b}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto device = _testSlaveMap[Device::cFirstDeviceId]->testDevice();
    auto device_2 = _testSlaveMap[Device::cFirstDeviceId + 1]->testDevice();
    device_2->configureHoldingRegister(1, true, 1);
    device->configureHoldingRegister(2, true, 5);
    device->configureHoldingRegister(3, true, 2);
    device->configureHoldingRegister(4, true, 1);

    auto resultList = ResultDoubleList() << ResultDouble(6, State::SUCCESS) << ResultDouble(2, State::SUCCESS)
                                         << ResultDouble(65538, State::SUCCESS);

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList);
}

void TestCommunication::mixed_fail()
{
    auto exprList = QStringList() << "${40001} + ${40002}"
                                  << "${40003}"
                                  << "${40004}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto device = _testSlaveMap[Device::cFirstDeviceId]->testDevice();
    device->configureHoldingRegister(0, false, 0);
    device->configureHoldingRegister(1, true, 1);
    device->configureHoldingRegister(2, false, 2);
    device->configureHoldingRegister(3, true, 3);

    auto resultList = ResultDoubleList() << ResultDouble(0, State::INVALID) << ResultDouble(0, State::INVALID)
                                         << ResultDouble(3, State::SUCCESS);

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList);
}

void TestCommunication::readLargeRegisterAddress()
{
    auto testSlaveData = new TestSlaveData(71001 - 40001, 50);

    auto device = _testSlaveMap[Device::cFirstDeviceId]->testDevice();
    device->setSlaveData(QModbusDataUnit::HoldingRegisters, testSlaveData);

    testSlaveData->setRegisterState(71001 - 40001, true);
    testSlaveData->setRegisterValue(71001 - 40001, 1);

    auto exprList = QStringList() << "${71001}";
    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto resultList = ResultDoubleList() << ResultDouble(1, State::SUCCESS);

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList);
}

void TestCommunication::unknownConnection()
{
    auto exprList = QStringList() << "${40001@255}"
                                  << "${40001}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto device = _testSlaveMap[Device::cFirstDeviceId]->testDevice();
    device->configureHoldingRegister(0, true, 2);

    auto resultList = ResultDoubleList() << ResultDouble(0, State::INVALID)
                                         << ResultDouble(2, State::SUCCESS);

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList);
}

void TestCommunication::disabledConnection()
{
    auto exprList = QStringList() << "${40001@1}"
                                  << "${40001@2}";

    _pSettingsModel->setConnectionState(ConnectionTypes::ID_2, false);

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto device = _testSlaveMap[Device::cFirstDeviceId]->testDevice();
    device->configureHoldingRegister(0, true, 5);

    auto resultList = ResultDoubleList() << ResultDouble(5, State::SUCCESS)
                                         << ResultDouble(0, State::INVALID);

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList);
}

void TestCommunication::doHandleRegisterData(QList<QVariant>& actRawData)
{
    GraphDataHandler dataHandler;
    ModbusPoll modbusPoll(_pSettingsModel);
    connect(&modbusPoll, &ModbusPoll::registerDataReady, &dataHandler, &GraphDataHandler::handleRegisterData);

    QList<ModbusRegister> registerList;
    dataHandler.setupExpressions(_pGraphDataModel, registerList);

    QSignalSpy spyDataReady(&dataHandler, &GraphDataHandler::graphDataReady);

    modbusPoll.startCommunication(registerList);

    auto timeout = _pSettingsModel->connectionSettings(ConnectionTypes::ID_1)->timeout();
    QVERIFY(spyDataReady.wait(static_cast<int>(timeout) + 100));
    QCOMPARE(spyDataReady.count(), 1);

    actRawData = spyDataReady.takeFirst();
}

QTEST_GUILESS_MAIN(TestCommunication)
