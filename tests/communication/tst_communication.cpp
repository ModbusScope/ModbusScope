
#include "tst_communication.h"

#include "communication/graphdatahandler.h"
#include "communication/modbuspoll.h"
#include "communicationhelpers.h"
#include "models/graphdatamodel.h"
#include "models/settingsmodel.h"

#include "testslavedata.h"

#include <QMap>
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

    for (quint8 idx = 0; idx < ConnectionTypes::ID_CNT; idx++)
    {
        connData = _pSettingsModel->connectionSettings(idx);

        _serverConnectionDataList.append(QUrl());
        _serverConnectionDataList.last().setPort(connData->port());
        _serverConnectionDataList.last().setHost(connData->ipAddress());

        auto modbusDataMap = new TestSlaveModbus::ModbusDataMap();
        (*modbusDataMap)[QModbusDataUnit::HoldingRegisters] = new TestSlaveData();
        _testSlaveDataList.append(modbusDataMap);
        _testSlaveModbusList.append(new TestSlaveModbus(*_testSlaveDataList.last()));

        auto slaveId = ConnectionTypes::ID_1 + 1 + idx; // TODO: dev: dirty hack
        QVERIFY(_testSlaveModbusList.last()->connect(_serverConnectionDataList.last(), slaveId));
    }
}

void TestCommunication::cleanup()
{
    delete _pGraphDataModel;
    delete _pSettingsModel;

    for (int idx = 0; idx < ConnectionTypes::ID_CNT; idx++)
    {
        _testSlaveModbusList[idx]->disconnectDevice();
    }

    while (!_testSlaveDataList.isEmpty())
    {
        TestSlaveModbus::ModbusDataMap * pLastMap = _testSlaveDataList.last();
        if (!pLastMap->isEmpty())
        {
            qDeleteAll(*pLastMap);
            pLastMap->clear();
        }
        _testSlaveDataList.removeLast();
    }
    qDeleteAll(_testSlaveModbusList);

    _serverConnectionDataList.clear();
    _testSlaveDataList.clear();
    _testSlaveModbusList.clear();
}

void TestCommunication::singleSlaveSuccess()
{
    auto exprList = QStringList() << "${40002}"
                                  << "${40001}";
    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(0, true);
    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(0, 1);

    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(1, true);
    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(1, 2);

    auto resultList = ResultDoubleList() << ResultDouble(2, State::SUCCESS) << ResultDouble(1, State::SUCCESS);

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList);
}

void TestCommunication::typeFloat32()
{
    auto exprList = QStringList() << "${40001: f32b}";
    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(0, true);
    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(0, 0xffff);

    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(1, true);
    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(1, 0x3f7f);

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

    dataMap(ConnectionTypes::ID_2, QModbusDataUnit::HoldingRegisters)->setRegisterState(1, true);
    dataMap(ConnectionTypes::ID_2, QModbusDataUnit::HoldingRegisters)->setRegisterValue(1, 1);

    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(2, true);
    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(2, 5);

    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(3, true);
    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(3, 2);

    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(4, true);
    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(4, 1);

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

    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(0, false);
    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(0, 0);

    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(1, true);
    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(1, 1);

    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(2, false);
    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(2, 2);

    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(3, true);
    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(3, 3);

    auto resultList = ResultDoubleList() << ResultDouble(0, State::INVALID) << ResultDouble(0, State::INVALID)
                                         << ResultDouble(3, State::SUCCESS);

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList);
}

void TestCommunication::readLargeRegisterAddress()
{
    /* Disable already initialized test slave on connection 1 */
    _testSlaveModbusList[ConnectionTypes::ID_1]->disconnect();

    TestSlaveData testSlaveData(71001 - 40001, 50);
    TestSlaveModbus::ModbusDataMap modbusDataMap;
    modbusDataMap[QModbusDataUnit::HoldingRegisters] = &testSlaveData;
    TestSlaveModbus testSlaveModbus(modbusDataMap);

    auto slaveId = ConnectionTypes::ID_1 + 1;
    QVERIFY(testSlaveModbus.connect(_serverConnectionDataList[ConnectionTypes::ID_1], slaveId));

    auto exprList = QStringList() << "${71001}"
                                  << "${71049}";
    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    testSlaveData.setRegisterState(71001 - 40001, true);
    testSlaveData.setRegisterValue(71001 - 40001, 1);

    testSlaveData.setRegisterState(71049 - 40001, true);
    testSlaveData.setRegisterValue(71049 - 40001, 2);

    auto resultList = ResultDoubleList() << ResultDouble(1, State::SUCCESS)
                                         << ResultDouble(2, State::SUCCESS);

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList);
}

void TestCommunication::readVeryLargeRegisterAddress()
{
#if 0
    /* Disable already initialized test slave on connection 1 */
    _testSlaveModbusList[ConnectionTypes::ID_1]->disconnect();

    TestSlaveData testSlaveData(105536 - 40001, 1); /* Last possible holding register (65353+40001) */
    //TestSlaveModbus testSlaveModbus(&testSlaveData);

    QVERIFY(testSlaveModbus.connect(_serverConnectionDataList[ConnectionTypes::ID_1], _pSettingsModel->slaveId(ConnectionTypes::ID_1)));

    auto exprList = QStringList() << "${105536}";
    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    testSlaveData.setRegisterState(105536 - 40001, true);
    testSlaveData.setRegisterValue(105536 - 40001, 1);

    auto resultList = ResultDoubleList() << ResultDouble(1, State::SUCCESS);

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList);
#endif
}

void TestCommunication::unknownConnection()
{
    auto exprList = QStringList() << "${40001@255}"
                                  << "${40001}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(0, true);
    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(0, 2);

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

    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(0, true);
    dataMap(ConnectionTypes::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(0, 5);

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

TestSlaveData* TestCommunication::dataMap(uint32_t connId, QModbusDataUnit::RegisterType type)
{
    return (_testSlaveDataList[connId])->value(type);
}

QTEST_GUILESS_MAIN(TestCommunication)
