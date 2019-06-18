
#include <QtTest/QtTest>
#include <QMap>

#include "modbusmaster.h"
#include "testslavedata.h"
#include "testslavemodbus.h"

#include "testcommunicationmanager.h"

void TestCommunicationManager::init()
{
    _pSettingsModel = new SettingsModel;
    _pGuiModel = new GuiModel;
    _pErrorLogModel = new ErrorLogModel;

    _pSettingsModel->setIpAddress(SettingsModel::CONNECTION_ID_0, "127.0.0.1");
    _pSettingsModel->setPort(SettingsModel::CONNECTION_ID_0, 5020);
    _pSettingsModel->setTimeout(SettingsModel::CONNECTION_ID_0, 500);
    _pSettingsModel->setSlaveId(SettingsModel::CONNECTION_ID_0, 1);

    _pSettingsModel->setIpAddress(SettingsModel::CONNECTION_ID_1, "127.0.0.1");
    _pSettingsModel->setPort(SettingsModel::CONNECTION_ID_1, 5021);
    _pSettingsModel->setTimeout(SettingsModel::CONNECTION_ID_1, 500);
    _pSettingsModel->setSlaveId(SettingsModel::CONNECTION_ID_1, 2);

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

void TestCommunicationManager::cleanup()
{
    delete _pSettingsModel;
    delete _pGuiModel;
    delete _pErrorLogModel;

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

void TestCommunicationManager::singleSlaveSuccess()
{
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterState(0, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterValue(0, 5);

    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterState(1, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterValue(1, 65000);

    GraphDataModel graphDataModel(_pSettingsModel);
    graphDataModel.add();
    graphDataModel.setRegisterAddress(0, 40001);

    graphDataModel.add();
    graphDataModel.setRegisterAddress(1, 40002);

    CommunicationManager conMan(_pSettingsModel, _pGuiModel, &graphDataModel, _pErrorLogModel);

    QSignalSpy spyReceivedData(&conMan, &CommunicationManager::handleReceivedData);

    /*-- Start communication --*/
    QVERIFY(conMan.startCommunication());

    QVERIFY(spyReceivedData.wait(20));
    QCOMPARE(spyReceivedData.count(), 1);

    QList<QVariant> arguments = spyReceivedData.takeFirst(); // take the first signal

    QList<bool> resultList({true, true});
    QList<double> valueList({5, 65000});

    /* Verify arguments of signal */
    verifyReceivedDataSignal(arguments, resultList, valueList);
}

void TestCommunicationManager::singleSlaveCheckProcessing()
{
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterState(0, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterValue(0, 5);

    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterState(1, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterValue(1, 65535);

    GraphDataModel graphDataModel(_pSettingsModel);
    graphDataModel.add();
    graphDataModel.setRegisterAddress(0, 40001);

    /* Set processing parameters */
    graphDataModel.setDivideFactor(0, 5);
    graphDataModel.setMultiplyFactor(0, 2);

    /* Set processing parameters */
    graphDataModel.add();
    graphDataModel.setRegisterAddress(1, 40002);
    graphDataModel.setUnsigned(1, false);

    CommunicationManager conMan(_pSettingsModel, _pGuiModel, &graphDataModel, _pErrorLogModel);

    QSignalSpy spyReceivedData(&conMan, &CommunicationManager::handleReceivedData);

    /*-- Start communication --*/
    QVERIFY(conMan.startCommunication());

    QVERIFY(spyReceivedData.wait(20));
    QCOMPARE(spyReceivedData.count(), 1);

    QList<QVariant> arguments = spyReceivedData.takeFirst(); // take the first signal

    QList<bool> resultList({true, true});
    QList<double> valueList({2, -1});

    /* Verify arguments of signal */
    verifyReceivedDataSignal(arguments, resultList, valueList);
}

void TestCommunicationManager::singleSlaveFail()
{
    for (quint8 idx = 0; idx < SettingsModel::CONNECTION_ID_CNT; idx++)
    {
        _testSlaveModbusList[idx]->disconnectDevice();
    }

    GraphDataModel graphDataModel(_pSettingsModel);
    graphDataModel.add();
    graphDataModel.setRegisterAddress(0, 40001);

    graphDataModel.add();
    graphDataModel.setRegisterAddress(1, 40002);

    CommunicationManager conMan(_pSettingsModel, _pGuiModel, &graphDataModel, _pErrorLogModel);

    QSignalSpy spyReceivedData(&conMan, &CommunicationManager::handleReceivedData);

    /*-- Start communication --*/
    QVERIFY(conMan.startCommunication());

    QVERIFY(spyReceivedData.wait(150));
    QCOMPARE(spyReceivedData.count(), 1);

    QList<QVariant> arguments = spyReceivedData.takeFirst(); // take the first signal

    QList<bool> resultList({false, false});
    QList<double> valueList({0, 0});

    /* Verify arguments of signal */
    verifyReceivedDataSignal(arguments, resultList, valueList);
}

void TestCommunicationManager::multiSlaveSuccess()
{
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterState(0, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterValue(0, 5020);

    _testSlaveDataList[SettingsModel::CONNECTION_ID_1]->setRegisterState(0, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_1]->setRegisterValue(0, 5021);

    GraphDataModel graphDataModel(_pSettingsModel);
    graphDataModel.add();
    graphDataModel.setConnectionId(0, SettingsModel::CONNECTION_ID_0);
    graphDataModel.setRegisterAddress(0, 40001);

    graphDataModel.add();
    graphDataModel.setConnectionId(1, SettingsModel::CONNECTION_ID_1);
    graphDataModel.setRegisterAddress(1, 40001);

    CommunicationManager conMan(_pSettingsModel, _pGuiModel, &graphDataModel, _pErrorLogModel);

    QSignalSpy spyReceivedData(&conMan, &CommunicationManager::handleReceivedData);

    /*-- Start communication --*/
    QVERIFY(conMan.startCommunication());

    QVERIFY(spyReceivedData.wait(20));
    QCOMPARE(spyReceivedData.count(), 1);

    QList<QVariant> arguments = spyReceivedData.takeFirst(); // take the first signal

    QList<bool> resultList({true, true});
    QList<double> valueList({5020, 5021});

    /* Verify arguments of signal */
    verifyReceivedDataSignal(arguments, resultList, valueList);
}

void TestCommunicationManager::multiSlaveSuccess_2()
{
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterState(0, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterValue(0, 5020);

    _testSlaveDataList[SettingsModel::CONNECTION_ID_1]->setRegisterState(1, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_1]->setRegisterValue(1, 5021);

    GraphDataModel graphDataModel(_pSettingsModel);
    graphDataModel.add();
    graphDataModel.setConnectionId(0, SettingsModel::CONNECTION_ID_0);
    graphDataModel.setRegisterAddress(0, 40001);

    graphDataModel.add();
    graphDataModel.setConnectionId(1, SettingsModel::CONNECTION_ID_1);
    graphDataModel.setRegisterAddress(1, 40002);

    CommunicationManager conMan(_pSettingsModel, _pGuiModel, &graphDataModel, _pErrorLogModel);

    QSignalSpy spyReceivedData(&conMan, &CommunicationManager::handleReceivedData);

    /*-- Start communication --*/
    QVERIFY(conMan.startCommunication());

    QVERIFY(spyReceivedData.wait(20));
    QCOMPARE(spyReceivedData.count(), 1);

    QList<QVariant> arguments = spyReceivedData.takeFirst(); // take the first signal

    QList<bool> resultList({true, true});
    QList<double> valueList({5020, 5021});

    /* Verify arguments of signal */
    verifyReceivedDataSignal(arguments, resultList, valueList);
}

void TestCommunicationManager::multiSlaveSuccess_3()
{
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterState(0, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterValue(0, 5020);

    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterState(1, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterValue(1, 5021);

    _testSlaveDataList[SettingsModel::CONNECTION_ID_1]->setRegisterState(1, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_1]->setRegisterValue(1, 5022);

    GraphDataModel graphDataModel(_pSettingsModel);
    graphDataModel.add();
    graphDataModel.setConnectionId(0, SettingsModel::CONNECTION_ID_0);
    graphDataModel.setRegisterAddress(0, 40001);

    graphDataModel.add();
    graphDataModel.setConnectionId(1, SettingsModel::CONNECTION_ID_1);
    graphDataModel.setRegisterAddress(1, 40002);

    graphDataModel.add();
    graphDataModel.setConnectionId(2, SettingsModel::CONNECTION_ID_0);
    graphDataModel.setRegisterAddress(2, 40002);

    CommunicationManager conMan(_pSettingsModel, _pGuiModel, &graphDataModel, _pErrorLogModel);

    QSignalSpy spyReceivedData(&conMan, &CommunicationManager::handleReceivedData);

    /*-- Start communication --*/
    QVERIFY(conMan.startCommunication());

    QVERIFY(spyReceivedData.wait(20));
    QCOMPARE(spyReceivedData.count(), 1);

    QList<QVariant> arguments = spyReceivedData.takeFirst(); // take the first signal

    QList<bool> resultList({true, true, true});
    QList<double> valueList({5020, 5022, 5021});

    /* Verify arguments of signal */
    verifyReceivedDataSignal(arguments, resultList, valueList);
}


void TestCommunicationManager::multiSlaveSingleFail()
{
    _testSlaveModbusList[SettingsModel::CONNECTION_ID_0]->disconnectDevice();

    _testSlaveDataList[SettingsModel::CONNECTION_ID_1]->setRegisterState(0, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_1]->setRegisterValue(0, 5021);

    GraphDataModel graphDataModel(_pSettingsModel);
    graphDataModel.add();
    graphDataModel.setConnectionId(0, SettingsModel::CONNECTION_ID_0);
    graphDataModel.setRegisterAddress(0, 40001);

    graphDataModel.add();
    graphDataModel.setConnectionId(1, SettingsModel::CONNECTION_ID_1);
    graphDataModel.setRegisterAddress(1, 40001);

    CommunicationManager conMan(_pSettingsModel, _pGuiModel, &graphDataModel, _pErrorLogModel);

    QSignalSpy spyReceivedData(&conMan, &CommunicationManager::handleReceivedData);

    /*-- Start communication --*/
    QVERIFY(conMan.startCommunication());

    QVERIFY(spyReceivedData.wait(20));
    QCOMPARE(spyReceivedData.count(), 1);

    QList<QVariant> arguments = spyReceivedData.takeFirst(); // take the first signal

    QList<bool> resultList({false, true});
    QList<double> valueList({0, 5021});

    /* Verify arguments of signal */
    verifyReceivedDataSignal(arguments, resultList, valueList);
}

void TestCommunicationManager::multiSlaveAllFail()
{
    for (quint8 idx = 0; idx < SettingsModel::CONNECTION_ID_CNT; idx++)
    {
        _testSlaveModbusList[idx]->disconnectDevice();
    }

    GraphDataModel graphDataModel(_pSettingsModel);
    graphDataModel.add();
    graphDataModel.setConnectionId(0, SettingsModel::CONNECTION_ID_0);
    graphDataModel.setRegisterAddress(0, 40001);

    graphDataModel.add();
    graphDataModel.setConnectionId(1, SettingsModel::CONNECTION_ID_1);
    graphDataModel.setRegisterAddress(1, 40001);

    CommunicationManager conMan(_pSettingsModel, _pGuiModel, &graphDataModel, _pErrorLogModel);

    QSignalSpy spyReceivedData(&conMan, &CommunicationManager::handleReceivedData);

    /*-- Start communication --*/
    QVERIFY(conMan.startCommunication());

    QVERIFY(spyReceivedData.wait(20));
    QCOMPARE(spyReceivedData.count(), 1);

    QList<QVariant> arguments = spyReceivedData.takeFirst(); // take the first signal

    QList<bool> resultList({false, false});
    QList<double> valueList({0, 0});

    /* Verify arguments of signal */
    verifyReceivedDataSignal(arguments, resultList, valueList);
}

void TestCommunicationManager::verifyReceivedDataSignal(QList<QVariant> arguments, QList<bool> expResultList, QList<double> expValueList)
{
    /* Verify result */
    QVERIFY((arguments[0].canConvert<QList<bool> >()));
    QList<bool> resultList = arguments[0].value<QList<bool> >();
    QCOMPARE(resultList.count(), expResultList.size());

    for(int idx = 0; idx < resultList.size(); idx++)
    {
        QCOMPARE(resultList[idx], expResultList[idx]);
    }

    /* Verify values */
    QVERIFY((arguments[1].canConvert<QList<double> >()));
    QList<double> valueList = arguments[1].value<QList<double> >();
    QCOMPARE(valueList.count(), expValueList.size());

    for(int idx = 0; idx < resultList.size(); idx++)
    {
        QCOMPARE(valueList[idx], expValueList[idx]);
    }
}

QTEST_GUILESS_MAIN(TestCommunicationManager)
