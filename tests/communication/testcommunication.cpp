
#include <QtTest/QtTest>
#include <QMap>

#include "communicationhelpers.h"
#include "settingsmodel.h"
#include "modbuspoll.h"
#include "graphdatahandler.h"
#include "graphdatamodel.h"

#include "testslavedata.h"
#include "testslavemodbus.h"

#include "testgraphdatahandler.h"
#include "testcommunication.h"

Q_DECLARE_METATYPE(Result);

void TestCommunication::init()
{
    qRegisterMetaType<Result>("Result");
    qRegisterMetaType<QList<Result> >("QList<Result>");

    _pSettingsModel = new SettingsModel;
    _pGraphDataModel = new GraphDataModel(_pSettingsModel);

    _pSettingsModel->setIpAddress(SettingsModel::CONNECTION_ID_0, "127.0.0.1");
    _pSettingsModel->setPort(SettingsModel::CONNECTION_ID_0, 5020);
    _pSettingsModel->setTimeout(SettingsModel::CONNECTION_ID_0, 500);
    _pSettingsModel->setSlaveId(SettingsModel::CONNECTION_ID_0, 1);

    _pSettingsModel->setConnectionState(SettingsModel::CONNECTION_ID_2, true);
    _pSettingsModel->setIpAddress(SettingsModel::CONNECTION_ID_2, "127.0.0.1");
    _pSettingsModel->setPort(SettingsModel::CONNECTION_ID_2, 5021);
    _pSettingsModel->setTimeout(SettingsModel::CONNECTION_ID_2, 500);
    _pSettingsModel->setSlaveId(SettingsModel::CONNECTION_ID_2, 2);

    _pSettingsModel->setConnectionState(SettingsModel::CONNECTION_ID_3, true);
    _pSettingsModel->setIpAddress(SettingsModel::CONNECTION_ID_3, "127.0.0.1");
    _pSettingsModel->setPort(SettingsModel::CONNECTION_ID_3, 5022);
    _pSettingsModel->setTimeout(SettingsModel::CONNECTION_ID_3, 500);
    _pSettingsModel->setSlaveId(SettingsModel::CONNECTION_ID_3, 3);

    _pSettingsModel->setPollTime(100);

    for (quint8 idx = 0; idx < SettingsModel::CONNECTION_ID_CNT; idx++)
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

    for (int idx = 0; idx < SettingsModel::CONNECTION_ID_CNT; idx++)
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

    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterState(0, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterValue(0, 1);

    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterState(1, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterValue(1, 2);

    auto resultList = QList<bool>() << true << true;
    auto valueList = QList<double>() << 2 << 1;

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

    _testSlaveDataList[SettingsModel::CONNECTION_ID_2]->setRegisterState(1, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_2]->setRegisterValue(1, 1);

    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterState(2, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterValue(2, 5);

    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterState(3, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterValue(3, 2);

    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterState(4, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterValue(4, 1);

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

    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterState(0, false);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterValue(0, 0);

    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterState(1, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterValue(1, 1);

    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterState(2, false);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterValue(2, 2);

    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterState(3, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterValue(3, 3);

    auto resultList = QList<bool>() << false << false << true;
    auto valueList = QList<double>() << 0 << 0 << 3;

    QList<QVariant> rawRegData;
    doHandleRegisterData(rawRegData);

    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList, valueList);
}

void TestCommunication::doHandleRegisterData(QList<QVariant>& actRawData)
{
    GraphDataHandler dataHandler(_pGraphDataModel);
    ModbusPoll modbusPoll(_pSettingsModel);
    connect(&modbusPoll, &ModbusPoll::registerDataReady, &dataHandler, &GraphDataHandler::handleRegisterData);

    QList<ModbusRegister> registerList;
    dataHandler.processActiveRegisters();
    dataHandler.modbusRegisterList(registerList);

    QSignalSpy spyDataReady(&dataHandler, &GraphDataHandler::graphDataReady);

    modbusPoll.startCommunication(registerList);

    QVERIFY(spyDataReady.wait(static_cast<int>(_pSettingsModel->timeout(SettingsModel::CONNECTION_ID_0)) + 100));
    QCOMPARE(spyDataReady.count(), 1);

    actRawData = spyDataReady.takeFirst();
}


QTEST_GUILESS_MAIN(TestCommunication)
