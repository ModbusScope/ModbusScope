
#include "tst_modbusconnection.h"

#include <QSignalSpy>
#include <QTest>

Q_DECLARE_METATYPE(ModbusDataUnit);

void TestModbusConnection::init()
{
    qRegisterMetaType<QModbusDevice::Error>("QModbusDevice::Error");

    _slaveId = 1;
    _port = 5020;
    _ip = "127.0.0.1";

    /* Server not started */
}

void TestModbusConnection::cleanup()
{
    _testSlaveModbus.disconnectDevice();
}

void TestModbusConnection::connectionSuccess()
{
    /* Start server */
    QVERIFY(_testSlaveModbus.connect(_ip, _port, _slaveId));

    ModbusConnection* pConnection = new ModbusConnection(this);

    QSignalSpy spySuccess(pConnection, &ModbusConnection::connectionSuccess);
    QSignalSpy spyError(pConnection, &ModbusConnection::connectionError);

    pConnection->configureTcpConnection(constructTcpSettings(_ip, _port));
    pConnection->open(1000);

    QVERIFY(spySuccess.wait(100));

    QCOMPARE(spySuccess.count(), 1);
    QCOMPARE(spyError.count(), 0);

    QVERIFY(pConnection->isConnected());

    pConnection->close();

    QVERIFY(!pConnection->isConnected());
}

void TestModbusConnection::connectionFail()
{
    ModbusConnection* pConnection = new ModbusConnection(this);

    QSignalSpy spySuccess(pConnection, &ModbusConnection::connectionSuccess);
    QSignalSpy spyError(pConnection, &ModbusConnection::connectionError);

    pConnection->configureTcpConnection(constructTcpSettings(_ip, _port));
    pConnection->open(1000);

    QVERIFY(spyError.wait(1500));

    QCOMPARE(spySuccess.count(), 0);
    QCOMPARE(spyError.count(), 1);

    QVERIFY(!pConnection->isConnected());
}

void TestModbusConnection::connectionSuccesAfterFail()
{
    ModbusConnection* pConnection = new ModbusConnection(this);

    QSignalSpy spySuccess(pConnection, &ModbusConnection::connectionSuccess);
    QSignalSpy spyError(pConnection, &ModbusConnection::connectionError);

    pConnection->configureTcpConnection(constructTcpSettings(_ip, _port));
    pConnection->open(1000);

    QVERIFY(spyError.wait(1500));

    QCOMPARE(spySuccess.count(), 0);
    QCOMPARE(spyError.count(), 1);

    QVERIFY(!pConnection->isConnected());

    // Start server
    QVERIFY(_testSlaveModbus.connect(_ip, _port, _slaveId));

    pConnection->configureTcpConnection(constructTcpSettings(_ip, _port));
    pConnection->open(1000);

    QVERIFY(spySuccess.wait(500));

    QCOMPARE(spySuccess.count(), 1);
    QVERIFY(pConnection->isConnected());

    pConnection->close();
}

void TestModbusConnection::readRequestSuccess()
{
    /* Start server */
    QVERIFY(_testSlaveModbus.connect(_ip, _port, _slaveId));

    _testSlaveModbus.testDevice()->configureHoldingRegister(0, true, 0);
    _testSlaveModbus.testDevice()->configureHoldingRegister(1, true, 1);

    /* Open connection */
    ModbusConnection* pConnection = new ModbusConnection(this);
    QSignalSpy spySuccess(pConnection, &ModbusConnection::connectionSuccess);
    pConnection->configureTcpConnection(constructTcpSettings(_ip, _port));
    pConnection->open(1000);

    QVERIFY(spySuccess.wait(100));

    QVERIFY(pConnection->isConnected());

    QSignalSpy spyResultSuccess(pConnection, &ModbusConnection::readRequestSuccess);
    QSignalSpy spyResultProtocolError(pConnection, &ModbusConnection::readRequestProtocolError);
    QSignalSpy spyResultError(pConnection, &ModbusConnection::readRequestError);

    pConnection->sendReadRequest(ModbusDataUnit(40001), 2);

    QVERIFY(spyResultSuccess.wait(100));
    QCOMPARE(spyResultSuccess.count(), 1);
    QCOMPARE(spyResultProtocolError.count(), 0);
    QCOMPARE(spyResultError.count(), 0);

    QList<QVariant> arguments = spyResultSuccess.takeFirst();
    QCOMPARE(arguments.count(), 2);

    /* Check start address */
    QVERIFY((arguments[0].canConvert<ModbusDataUnit>()));
    auto resultAddr = arguments[0].value<ModbusDataUnit>();
    QCOMPARE(resultAddr.fullAddress(), "40001");

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
    QVERIFY(_testSlaveModbus.connect(_ip, _port, _slaveId));

    _testSlaveModbus.testDevice()->configureHoldingRegister(0, false, 0);
    _testSlaveModbus.testDevice()->configureHoldingRegister(1, true, 1);

    /* Open connection */
    ModbusConnection* pConnection = new ModbusConnection(this);
    QSignalSpy spySuccess(pConnection, &ModbusConnection::connectionSuccess);
    pConnection->configureTcpConnection(constructTcpSettings(_ip, _port));
    pConnection->open(1000);

    QVERIFY(spySuccess.wait(100));

    QVERIFY(pConnection->isConnected());

    QSignalSpy spyResultSuccess(pConnection, &ModbusConnection::readRequestSuccess);
    QSignalSpy spyResultProtocolError(pConnection, &ModbusConnection::readRequestProtocolError);
    QSignalSpy spyResultError(pConnection, &ModbusConnection::readRequestError);

    pConnection->sendReadRequest(ModbusDataUnit(40001), 2);

    QVERIFY(spyResultProtocolError.wait(100));
    QCOMPARE(spyResultSuccess.count(), 0);
    QCOMPARE(spyResultProtocolError.count(), 1);
    QCOMPARE(spyResultError.count(), 0);

    QList<QVariant> arguments = spyResultProtocolError.takeFirst();
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

ModbusConnection::tcpSettings_t TestModbusConnection::constructTcpSettings(QString ip, qint32 port)
{
    ModbusConnection::tcpSettings_t tcpSettings = {
        .ip = ip,
        .port = port,
    };

    return tcpSettings;
}

QTEST_GUILESS_MAIN(TestModbusConnection)
