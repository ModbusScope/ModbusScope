
#include <QtTest/QtTest>

#include "modbusconnection.h"
#include "testslavedata.h"
#include "testslavemodbus.h"

#include "testmodbusconnection.h"

void TestModbusConnection::initTestCase()
{
    qRegisterMetaType<QModbusDevice::Error>("QModbusDevice::Error");
}

void TestModbusConnection::connectionSuccess()
{
    TestSlaveData testSlaveData;
    TestSlaveModbus testSlaveModbus(&testSlaveData);

    QUrl serverConnectionData = QUrl::fromUserInput("127.0.0.1:5002");
    QVERIFY(testSlaveModbus.connect(serverConnectionData, 1));

    ModbusConnection * pConnection = new ModbusConnection(this);

    QSignalSpy spySuccess(pConnection, &ModbusConnection::connectionSuccess);
    QSignalSpy spyError(pConnection, &ModbusConnection::connectionError);

    pConnection->openConnection(serverConnectionData.host(), serverConnectionData.port(), 1000);

    QVERIFY(spySuccess.wait(100));

    QCOMPARE(spySuccess.count(), 1);
    QCOMPARE(spyError.count(), 0);

    QCOMPARE(pConnection->connectionState(), QModbusDevice::ConnectedState);

    pConnection->closeConnection();

    QCOMPARE(pConnection->connectionState(), QModbusDevice::UnconnectedState);

}

void TestModbusConnection::connectionFail()
{
    QUrl serverConnectionData = QUrl::fromUserInput("127.0.0.1:5002");

    ModbusConnection * pConnection = new ModbusConnection(this);

    QSignalSpy spySuccess(pConnection, &ModbusConnection::connectionSuccess);
    QSignalSpy spyError(pConnection, &ModbusConnection::connectionError);

    pConnection->openConnection(serverConnectionData.host(), serverConnectionData.port(), 1000);

    QVERIFY(spyError.wait(1500));

    QCOMPARE(spySuccess.count(), 0);
    QCOMPARE(spyError.count(), 1);

    QCOMPARE(pConnection->connectionState(), QModbusDevice::UnconnectedState);
}


void TestModbusConnection::connectionSuccesAfterFail()
{
    QUrl serverConnectionData = QUrl::fromUserInput("127.0.0.1:5002");

    ModbusConnection * pConnection = new ModbusConnection(this);

    QSignalSpy spySuccess(pConnection, &ModbusConnection::connectionSuccess);
    QSignalSpy spyError(pConnection, &ModbusConnection::connectionError);

    pConnection->openConnection(serverConnectionData.host(), serverConnectionData.port(), 1000);

    QVERIFY(spyError.wait(1500));

    QCOMPARE(spySuccess.count(), 0);
    QCOMPARE(spyError.count(), 1);

    QCOMPARE(pConnection->connectionState(), QModbusDevice::UnconnectedState);

    // Start server
    TestSlaveData testSlaveData;
    TestSlaveModbus testSlaveModbus(&testSlaveData);

    QVERIFY(testSlaveModbus.connect(serverConnectionData, 1));

    pConnection->openConnection(serverConnectionData.host(), serverConnectionData.port(), 1000);

    QVERIFY(spySuccess.wait(500));

    QCOMPARE(spySuccess.count(), 1);
    QCOMPARE(pConnection->connectionState(), QModbusDevice::ConnectedState);

    pConnection->closeConnection();
}

QTEST_GUILESS_MAIN(TestModbusConnection)
