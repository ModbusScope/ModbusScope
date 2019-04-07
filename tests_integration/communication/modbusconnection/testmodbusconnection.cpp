
#include <QtTest/QtTest>

#include "modbusconnection.h"
#include "testslavedata.h"
#include "testslavemodbus.h"

#include "testmodbusconnection.h"

void TestModbusConnection::init()
{
    qRegisterMetaType<QModbusDevice::Error>("QModbusDevice::Error");

    _settingsModel.setIpAddress(SettingsModel::CONNECTION_ID_0, "127.0.0.1");
    _settingsModel.setPort(SettingsModel::CONNECTION_ID_0, 5020);
    _settingsModel.setTimeout(SettingsModel::CONNECTION_ID_0, 500);
    _settingsModel.setSlaveId(SettingsModel::CONNECTION_ID_0, 1);

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

    /* Server not started */
}

void TestModbusConnection::cleanup()
{
    _pTestSlaveModbus->disconnectDevice();

    delete _pTestSlaveData;
    delete _pTestSlaveModbus;
}

void TestModbusConnection::connectionSuccess()
{
    /* Start server */
    QVERIFY(_pTestSlaveModbus->connect(_serverConnectionData, _settingsModel.slaveId(SettingsModel::CONNECTION_ID_0)));

    ModbusConnection * pConnection = new ModbusConnection(this);

    QSignalSpy spySuccess(pConnection, &ModbusConnection::connectionSuccess);
    QSignalSpy spyError(pConnection, &ModbusConnection::connectionError);

    pConnection->openConnection(_serverConnectionData.host(), _serverConnectionData.port(), 1000);

    QVERIFY(spySuccess.wait(100));

    QCOMPARE(spySuccess.count(), 1);
    QCOMPARE(spyError.count(), 0);

    QCOMPARE(pConnection->connectionState(), QModbusDevice::ConnectedState);

    pConnection->closeConnection();

    QCOMPARE(pConnection->connectionState(), QModbusDevice::UnconnectedState);

}

void TestModbusConnection::connectionFail()
{
    ModbusConnection * pConnection = new ModbusConnection(this);

    QSignalSpy spySuccess(pConnection, &ModbusConnection::connectionSuccess);
    QSignalSpy spyError(pConnection, &ModbusConnection::connectionError);

    pConnection->openConnection(_serverConnectionData.host(), _serverConnectionData.port(), 1000);

    QVERIFY(spyError.wait(1500));

    QCOMPARE(spySuccess.count(), 0);
    QCOMPARE(spyError.count(), 1);

    QCOMPARE(pConnection->connectionState(), QModbusDevice::UnconnectedState);
}


void TestModbusConnection::connectionSuccesAfterFail()
{

    ModbusConnection * pConnection = new ModbusConnection(this);

    QSignalSpy spySuccess(pConnection, &ModbusConnection::connectionSuccess);
    QSignalSpy spyError(pConnection, &ModbusConnection::connectionError);

    pConnection->openConnection(_serverConnectionData.host(), _serverConnectionData.port(), 1000);

    QVERIFY(spyError.wait(1500));

    QCOMPARE(spySuccess.count(), 0);
    QCOMPARE(spyError.count(), 1);

    QCOMPARE(pConnection->connectionState(), QModbusDevice::UnconnectedState);

    // Start server
    QVERIFY(_pTestSlaveModbus->connect(_serverConnectionData, _settingsModel.slaveId(SettingsModel::CONNECTION_ID_0)));

    pConnection->openConnection(_serverConnectionData.host(), _serverConnectionData.port(), 1000);

    QVERIFY(spySuccess.wait(500));

    QCOMPARE(spySuccess.count(), 1);
    QCOMPARE(pConnection->connectionState(), QModbusDevice::ConnectedState);

    pConnection->closeConnection();
}

void TestModbusConnection::readRequestSuccess()
{
    /* Start server */
    QVERIFY(_pTestSlaveModbus->connect(_serverConnectionData, _settingsModel.slaveId(SettingsModel::CONNECTION_ID_0)));

    _pTestSlaveData->setRegisterState(0, true);
    _pTestSlaveData->setRegisterState(1, true);

    _pTestSlaveData->setRegisterValue(0, 0);
    _pTestSlaveData->setRegisterValue(1, 1);

    /* Open connection */
    ModbusConnection * pConnection = new ModbusConnection(this);
    QSignalSpy spySuccess(pConnection, &ModbusConnection::connectionSuccess);
    pConnection->openConnection(_serverConnectionData.host(), _serverConnectionData.port(), 1000);

    pConnection->openConnection(_serverConnectionData.host(), _serverConnectionData.port(), 1000);

    QVERIFY(spySuccess.wait(100));

    QCOMPARE(pConnection->connectionState(), QModbusDevice::ConnectedState);

    QSignalSpy spyResultSuccess(pConnection, &ModbusConnection::readRequestSuccess);
    QSignalSpy spyResultProtocolError(pConnection, &ModbusConnection::readRequestProtocolError);
    QSignalSpy spyResultError(pConnection, &ModbusConnection::readRequestError);

    pConnection->sendReadRequest(40001, 2, _settingsModel.slaveId(SettingsModel::CONNECTION_ID_0));

    QVERIFY(spyResultSuccess.wait(100));
    QCOMPARE(spyResultSuccess.count(), 1);
    QCOMPARE(spyResultProtocolError.count(), 0);
    QCOMPARE(spyResultError.count(), 0);

    QList<QVariant> arguments = spyResultSuccess.takeFirst(); // take the first signal
    QCOMPARE(arguments.count(), 2);


    /* Check start address */
    QCOMPARE(arguments.first().toInt(), 40001);

    /* Check result */
    QVERIFY((arguments[1].canConvert<QList<quint16> >()));
    QList<quint16> resultList = arguments[1].value<QList<quint16> >();
    QCOMPARE(resultList.count(), 2);
    QCOMPARE(resultList[0], static_cast<quint16>(0));
    QCOMPARE(resultList[1], static_cast<quint16>(1));

}

void TestModbusConnection::readRequestProtocolError()
{
    /* Start server */
    QVERIFY(_pTestSlaveModbus->connect(_serverConnectionData, _settingsModel.slaveId(SettingsModel::CONNECTION_ID_0)));

    _pTestSlaveData->setRegisterState(0, false);
    _pTestSlaveData->setRegisterState(1, true);

    _pTestSlaveData->setRegisterValue(0, 0);
    _pTestSlaveData->setRegisterValue(1, 1);

    /* Open connection */
    ModbusConnection * pConnection = new ModbusConnection(this);
    QSignalSpy spySuccess(pConnection, &ModbusConnection::connectionSuccess);
    pConnection->openConnection(_serverConnectionData.host(), _serverConnectionData.port(), 1000);

    pConnection->openConnection(_serverConnectionData.host(), _serverConnectionData.port(), 1000);

    QVERIFY(spySuccess.wait(100));

    QCOMPARE(pConnection->connectionState(), QModbusDevice::ConnectedState);

    QSignalSpy spyResultSuccess(pConnection, &ModbusConnection::readRequestSuccess);
    QSignalSpy spyResultProtocolError(pConnection, &ModbusConnection::readRequestProtocolError);
    QSignalSpy spyResultError(pConnection, &ModbusConnection::readRequestError);

    pConnection->sendReadRequest(40001, 2, _settingsModel.slaveId(SettingsModel::CONNECTION_ID_0));

    QVERIFY(spyResultProtocolError.wait(100));
    QCOMPARE(spyResultSuccess.count(), 0);
    QCOMPARE(spyResultProtocolError.count(), 1);
    QCOMPARE(spyResultError.count(), 0);

    QList<QVariant> arguments = spyResultProtocolError.takeFirst(); // take the first signal
    QCOMPARE(arguments.count(), 1);

    /* Check modbus exception */
    QCOMPARE(static_cast<QModbusPdu::ExceptionCode>(arguments.first().toInt()), QModbusPdu::IllegalDataAddress);

}

void TestModbusConnection::readRequestError()
{
    /* TODO:
     * Add test for this case.
     * Is this case possible?
    */
}

QTEST_GUILESS_MAIN(TestModbusConnection)
