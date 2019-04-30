
#include <QtTest/QtTest>
#include <QMap>

#include "modbusmaster.h"
#include "testslavedata.h"
#include "testslavemodbus.h"

#include "testcommunicationmanager.h"

void TestCommunicationManager::init()
{

    _settingsModel.setIpAddress(SettingsModel::CONNECTION_ID_0, "127.0.0.1");
    _settingsModel.setPort(SettingsModel::CONNECTION_ID_0, 5020);
    _settingsModel.setTimeout(SettingsModel::CONNECTION_ID_0, 500);
    _settingsModel.setSlaveId(SettingsModel::CONNECTION_ID_0, 1);

    _settingsModel.setIpAddress(SettingsModel::CONNECTION_ID_1, "127.0.0.1");
    _settingsModel.setPort(SettingsModel::CONNECTION_ID_1, 5021);
    _settingsModel.setTimeout(SettingsModel::CONNECTION_ID_1, 500);
    _settingsModel.setSlaveId(SettingsModel::CONNECTION_ID_1, 2);

    _settingsModel.setPollTime(100);

    _serverConnectionData.setPort(_settingsModel.port(SettingsModel::CONNECTION_ID_0));
    _serverConnectionData.setHost(_settingsModel.ipAddress(SettingsModel::CONNECTION_ID_0));

    if (!_pTestSlaveData.isNull())
    {
        delete _pTestSlaveData;
    }
    if (!_pTestSlaveModbus.isNull())
    {
        delete _pTestSlaveModbus;
    }

    _pTestSlaveData = new TestSlaveData();
    _pTestSlaveModbus= new TestSlaveModbus(_pTestSlaveData);

    QVERIFY(_pTestSlaveModbus->connect(_serverConnectionData, _settingsModel.slaveId(SettingsModel::CONNECTION_ID_0)));

}

void TestCommunicationManager::cleanup()
{
    _pTestSlaveModbus->disconnectDevice();

    delete _pTestSlaveData;
    delete _pTestSlaveModbus;
}

void TestCommunicationManager::singleSlaveSuccess()
{
    _pTestSlaveData->setRegisterState(0, true);
    _pTestSlaveData->setRegisterValue(0, 5);

    _pTestSlaveData->setRegisterState(1, true);
    _pTestSlaveData->setRegisterValue(1, 65000);


    CommunicationManager conMan(&_settingsModel, &_guiModel, &_graphDataModel, &_errorLogModel);

    _graphDataModel.add();
    _graphDataModel.setRegisterAddress(0, 40001);

    _graphDataModel.add();
    _graphDataModel.setRegisterAddress(1, 40002);

    QSignalSpy spyReceivedData(&conMan, &CommunicationManager::handleReceivedData);

    /*-- Start communication --*/
    QVERIFY(conMan.startCommunication());

    QVERIFY(spyReceivedData.wait(150));
    QCOMPARE(spyReceivedData.count(), 1);

    QList<QVariant> arguments = spyReceivedData.takeFirst(); // take the first signal
    QVERIFY(arguments.count() > 0);

    /* Verify result */
    QVERIFY((arguments[0].canConvert<QList<bool> >()));
    QList<bool> resultList = arguments[0].value<QList<bool> >();
    QCOMPARE(resultList.count(), 2);

    QVERIFY(resultList[0]);
    QVERIFY(resultList[1]);

    /* Verify values */
    QVERIFY((arguments[1].canConvert<QList<double> >()));
    QList<double> valueList = arguments[1].value<QList<double> >();
    QCOMPARE(valueList.count(), 2);

    QCOMPARE(valueList[0], static_cast<double>(5));
    QCOMPARE(valueList[1], static_cast<double>(65000));

}

QTEST_GUILESS_MAIN(TestCommunicationManager)
