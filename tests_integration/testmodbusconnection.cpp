
#include <QtTest/QtTest>

#include "src/communication/modbusconnection.h"
#include "testslave/testslavedata.h"
#include "testslave/testslavemodbus.h"

#include "testmodbusconnection.h"

void TestModbusConnection::connectionSuccess()
{
    qRegisterMetaType<QModbusDevice::Error>("QModbusDevice::Error");

    TestSlaveModbus testSlaveModbus;
    TestSlaveData testSlaveData(&testSlaveModbus);

    QUrl serverConnectionData = QUrl::fromUserInput("127.0.0.1:5002");
    QVERIFY(testSlaveModbus.connect(serverConnectionData, true));

    ModbusConnection * pConnection = new ModbusConnection(this);

    QSignalSpy spySuccess(pConnection, &ModbusConnection::connectionSuccess);
    QSignalSpy spyError(pConnection, &ModbusConnection::errorOccurred);

    pConnection->openConnection(serverConnectionData.host(), serverConnectionData.port(), 1000);

    QVERIFY(spySuccess.wait(100));

    QCOMPARE(spySuccess.count(), 1);
    QCOMPARE(spyError.count(), 0);

    QCOMPARE(pConnection->state(), QModbusDevice::ConnectedState);

    pConnection->closeConnection();

    QCOMPARE(pConnection->state(), QModbusDevice::UnconnectedState);

}

QTEST_MAIN(TestModbusConnection)
