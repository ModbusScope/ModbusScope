
#include <QtTest/QtTest>
#include <QMap>

#include "communicationhelpers.h"
#include "settingsmodel.h"
#include "modbuspoll.h"
#include "graphdatahandler.h"
#include "graphdatamodel.h"

#include "testslavedata.h"
#include "testslavemodbus.h"

#include "tst_graphdatahandler.h"
#include "tst_communication.h"


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

        _testSlaveDataList.append(new TestSlaveData());
        _testSlaveModbusList.append(new TestSlaveModbus(_testSlaveDataList.last()));

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

    qDeleteAll(_testSlaveDataList);
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

    _testSlaveDataList[Connection::ID_1]->setRegisterState(0, true);
    _testSlaveDataList[Connection::ID_1]->setRegisterValue(0, 1);

    _testSlaveDataList[Connection::ID_1]->setRegisterState(1, true);
    _testSlaveDataList[Connection::ID_1]->setRegisterValue(1, 2);

    auto resultList = QList<bool>() << true << true;
    auto valueList = QList<double>() << 2 << 1;

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList, valueList);
}

void TestCommunication::constantExpression()
{
    auto exprList = QStringList() << "3";
    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto resultList = QList<bool>() << true;
    auto valueList = QList<double>() << 3;

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList, valueList);
}

void TestCommunication::mixed_1()
{
    auto exprList = QStringList() << "${40002@2} + ${40003}"
                                  << "2"
                                  << "${40004:32b}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    _testSlaveDataList[Connection::ID_2]->setRegisterState(1, true);
    _testSlaveDataList[Connection::ID_2]->setRegisterValue(1, 1);

    _testSlaveDataList[Connection::ID_1]->setRegisterState(2, true);
    _testSlaveDataList[Connection::ID_1]->setRegisterValue(2, 5);

    _testSlaveDataList[Connection::ID_1]->setRegisterState(3, true);
    _testSlaveDataList[Connection::ID_1]->setRegisterValue(3, 2);

    _testSlaveDataList[Connection::ID_1]->setRegisterState(4, true);
    _testSlaveDataList[Connection::ID_1]->setRegisterValue(4, 1);

    auto resultList = QList<bool>() << true << true << true;
    auto valueList = QList<double>() << 6 << 2 << 65538;

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList, valueList);
}

void TestCommunication::mixed_fail()
{
    auto exprList = QStringList() << "${40001} + ${40002}"
                                  << "${40003}"
                                  << "${40004}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    _testSlaveDataList[Connection::ID_1]->setRegisterState(0, false);
    _testSlaveDataList[Connection::ID_1]->setRegisterValue(0, 0);

    _testSlaveDataList[Connection::ID_1]->setRegisterState(1, true);
    _testSlaveDataList[Connection::ID_1]->setRegisterValue(1, 1);

    _testSlaveDataList[Connection::ID_1]->setRegisterState(2, false);
    _testSlaveDataList[Connection::ID_1]->setRegisterValue(2, 2);

    _testSlaveDataList[Connection::ID_1]->setRegisterState(3, true);
    _testSlaveDataList[Connection::ID_1]->setRegisterValue(3, 3);

    auto resultList = QList<bool>() << false << false << true;
    auto valueList = QList<double>() << 0 << 0 << 3;

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList, valueList);
}

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

    auto resultList = QList<bool>() << true << true;
    auto valueList = QList<double>() << 1 << 2;

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList, valueList);
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

    auto resultList = QList<bool>() << true;
    auto valueList = QList<double>() << 1;

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList, valueList);
}

void TestCommunication::unknownConnection()
{
    auto exprList = QStringList() << "${40001@255}"
                                  << "${40001}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    _testSlaveDataList[Connection::ID_1]->setRegisterState(0, true);
    _testSlaveDataList[Connection::ID_1]->setRegisterValue(0, 2);

    auto resultList = QList<bool>() << false << true;
    auto valueList = QList<double>() << 0 << 2;

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList, valueList);
}

void TestCommunication::disabledConnection()
{
    auto exprList = QStringList() << "${40001@1}"
                                  << "${40001@2}";

    _pSettingsModel->setConnectionState(Connection::ID_2, false);

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    _testSlaveDataList[Connection::ID_1]->setRegisterState(0, true);
    _testSlaveDataList[Connection::ID_1]->setRegisterValue(0, 5);

    auto resultList = QList<bool>() << true << false;
    auto valueList = QList<double>() << 5 << 0;

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList, valueList);
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

QTEST_GUILESS_MAIN(TestCommunication)
