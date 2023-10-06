
#include <QtTest/QtTest>
#include <QMap>

#include "communicationhelpers.h"
#include "settingsmodel.h"
#include "modbuspoll.h"
#include "graphdatahandler.h"
#include "graphdatamodel.h"

#include "testslavedata.h"
#include "testslavemodbus.h"

#include "tst_communication.h"

using State = ResultState::State;

void TestCommunication::init()
{
    _pSettingsModel = new SettingsModel;
    _pGraphDataModel = new GraphDataModel(_pSettingsModel);

    _pSettingsModel->setIpAddress(Connection::ID_1, "127.0.0.1");
    _pSettingsModel->setPort(Connection::ID_1, 5020);
    _pSettingsModel->setTimeout(Connection::ID_1, 500);
    _pSettingsModel->setSlaveId(Connection::ID_1, 1);

    _pSettingsModel->setConnectionState(Connection::ID_2, true);
    _pSettingsModel->setIpAddress(Connection::ID_2, "127.0.0.1");
    _pSettingsModel->setPort(Connection::ID_2, 5021);
    _pSettingsModel->setTimeout(Connection::ID_2, 500);
    _pSettingsModel->setSlaveId(Connection::ID_2, 2);

    _pSettingsModel->setConnectionState(Connection::ID_3, true);
    _pSettingsModel->setIpAddress(Connection::ID_3, "127.0.0.1");
    _pSettingsModel->setPort(Connection::ID_3, 5022);
    _pSettingsModel->setTimeout(Connection::ID_3, 500);
    _pSettingsModel->setSlaveId(Connection::ID_3, 3);

    _pSettingsModel->setPollTime(100);

    for (quint8 idx = 0; idx < Connection::ID_CNT; idx++)
    {
        _serverConnectionDataList.append(QUrl());
        _serverConnectionDataList.last().setPort(_pSettingsModel->port(idx));
        _serverConnectionDataList.last().setHost(_pSettingsModel->ipAddress(idx));

        auto modbusDataMap = new TestSlaveModbus::ModbusDataMap();
        (*modbusDataMap)[QModbusDataUnit::HoldingRegisters] = new TestSlaveData();
        _testSlaveDataList.append(modbusDataMap);
        _testSlaveModbusList.append(new TestSlaveModbus(*_testSlaveDataList.last()));

        QVERIFY(_testSlaveModbusList.last()->connect(_serverConnectionDataList.last(), _pSettingsModel->slaveId(idx)));
    }
}

void TestCommunication::cleanup()
{
    delete _pGraphDataModel;
    delete _pSettingsModel;

    for (int idx = 0; idx < Connection::ID_CNT; idx++)
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


    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(0, true);
    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(0, 1);

    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(1, true);
    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(1, 2);

    auto resultList = ResultDoubleList() << ResultDouble(2, State::SUCCESS) << ResultDouble(1, State::SUCCESS);

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList);
}

void TestCommunication::typeFloat32()
{
    auto exprList = QStringList() << "${40001: f32b}";
    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(0, true);
    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(0, 0xffff);

    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(1, true);
    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(1, 0x3f7f);

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

    dataMap(Connection::ID_2, QModbusDataUnit::HoldingRegisters)->setRegisterState(1, true);
    dataMap(Connection::ID_2, QModbusDataUnit::HoldingRegisters)->setRegisterValue(1, 1);

    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(2, true);
    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(2, 5);

    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(3, true);
    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(3, 2);

    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(4, true);
    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(4, 1);

    auto resultList = ResultDoubleList() << ResultDouble(6, State::SUCCESS)
                                         << ResultDouble(2, State::SUCCESS)
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

    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(0, false);
    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(0, 0);

    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(1, true);
    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(1, 1);

    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(2, false);
    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(2, 2);

    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(3, true);
    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(3, 3);

    auto resultList = ResultDoubleList() << ResultDouble(0, State::INVALID)
                                         << ResultDouble(0, State::INVALID)
                                         << ResultDouble(3, State::SUCCESS);

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList);
}
#if 0
TODO: Enable when it is possible to define {h3000}
void TestCommunication::readLargeRegisterAddress()
{
    /* Disable already initialized test slave on connection 1 */
    _testSlaveModbusList[Connection::ID_1]->disconnect();

    TestSlaveData testSlaveData(71001 - 40001, 50);
    TestSlaveModbus testSlaveModbus(&testSlaveData);

    QVERIFY(testSlaveModbus.connect(_serverConnectionDataList[Connection::ID_1], _pSettingsModel->slaveId(Connection::ID_1)));

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
    /* Disable already initialized test slave on connection 1 */
    _testSlaveModbusList[Connection::ID_1]->disconnect();

    TestSlaveData testSlaveData(105536 - 40001, 1); /* Last possible holding register (65353+40001) */
    TestSlaveModbus testSlaveModbus(&testSlaveData);

    QVERIFY(testSlaveModbus.connect(_serverConnectionDataList[Connection::ID_1], _pSettingsModel->slaveId(Connection::ID_1)));

    auto exprList = QStringList() << "${105536}";
    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    testSlaveData.setRegisterState(105536 - 40001, true);
    testSlaveData.setRegisterValue(105536 - 40001, 1);

    auto resultList = ResultDoubleList() << ResultDouble(1, State::SUCCESS);

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList);
}
#endif
void TestCommunication::unknownConnection()
{
    auto exprList = QStringList() << "${40001@255}"
                                  << "${40001}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(0, true);
    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(0, 2);

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

    _pSettingsModel->setConnectionState(Connection::ID_2, false);

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterState(0, true);
    dataMap(Connection::ID_1, QModbusDataUnit::HoldingRegisters)->setRegisterValue(0, 5);

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
    dataHandler.processActiveRegisters(_pGraphDataModel);
    dataHandler.modbusRegisterList(registerList);

    QSignalSpy spyDataReady(&dataHandler, &GraphDataHandler::graphDataReady);

    modbusPoll.startCommunication(registerList);

    QVERIFY(spyDataReady.wait(static_cast<int>(_pSettingsModel->timeout(Connection::ID_1)) + 100));
    QCOMPARE(spyDataReady.count(), 1);

    actRawData = spyDataReady.takeFirst();
}

TestSlaveData* TestCommunication::dataMap(uint32_t connId, QModbusDataUnit::RegisterType type)
{
    return (_testSlaveDataList[connId])->value(type);
}

QTEST_GUILESS_MAIN(TestCommunication)
