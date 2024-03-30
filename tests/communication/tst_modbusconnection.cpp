
#include <QtTest/QtTest>

#include "tst_modbusconnection.h"

Q_DECLARE_METATYPE(ModbusAddress);

void TestModbusConnection::init()
{
    qRegisterMetaType<QModbusDevice::Error>("QModbusDevice::Error");

    _slaveId = 1;
    _serverConnectionData.setPort(5020);
    _serverConnectionData.setHost("127.0.0.1");

    if (!_testSlaveData.isEmpty())
    {
        qDeleteAll(_testSlaveData);
        _testSlaveData.clear();
    }
    if (!_pTestSlaveModbus.isNull())
    {
        delete _pTestSlaveModbus;
    }

    _testSlaveData[QModbusDataUnit::HoldingRegisters] = new TestSlaveData();
    _pTestSlaveModbus = new TestSlaveModbus(_testSlaveData);

    /* Server not started */
}

void TestModbusConnection::cleanup()
{
    _pTestSlaveModbus->disconnectDevice();

    if (!_testSlaveData.isEmpty())
    {
        qDeleteAll(_testSlaveData);
        _testSlaveData.clear();
    }
    delete _pTestSlaveModbus;
}

void TestModbusConnection::connectionSuccess()
{
    /* Start server */
    QVERIFY(_pTestSlaveModbus->connect(_serverConnectionData, _slaveId));

    ModbusConnection * pConnection = new ModbusConnection(this);

    QSignalSpy spySuccess(pConnection, &ModbusConnection::connectionSuccess);
    QSignalSpy spyError(pConnection, &ModbusConnection::connectionError);

    pConnection->openTcpConnection(constructTcpSettings(_serverConnectionData.host(), _serverConnectionData.port()), 1000);

    QVERIFY(spySuccess.wait(100));

    QCOMPARE(spySuccess.count(), 1);
    QCOMPARE(spyError.count(), 0);

    QVERIFY(pConnection->isConnected());

    pConnection->closeConnection();

    QVERIFY(!pConnection->isConnected());

}

void TestModbusConnection::connectionFail()
{
    ModbusConnection * pConnection = new ModbusConnection(this);

    QSignalSpy spySuccess(pConnection, &ModbusConnection::connectionSuccess);
    QSignalSpy spyError(pConnection, &ModbusConnection::connectionError);

    pConnection->openTcpConnection(constructTcpSettings(_serverConnectionData.host(), _serverConnectionData.port()), 1000);

    QVERIFY(spyError.wait(1500));

    QCOMPARE(spySuccess.count(), 0);
    QCOMPARE(spyError.count(), 1);

    QVERIFY(!pConnection->isConnected());
}


void TestModbusConnection::connectionSuccesAfterFail()
{

    ModbusConnection * pConnection = new ModbusConnection(this);

    QSignalSpy spySuccess(pConnection, &ModbusConnection::connectionSuccess);
    QSignalSpy spyError(pConnection, &ModbusConnection::connectionError);

    pConnection->openTcpConnection(constructTcpSettings(_serverConnectionData.host(), _serverConnectionData.port()), 1000);

    QVERIFY(spyError.wait(1500));

    QCOMPARE(spySuccess.count(), 0);
    QCOMPARE(spyError.count(), 1);

    QVERIFY(!pConnection->isConnected());

    // Start server
    QVERIFY(_pTestSlaveModbus->connect(_serverConnectionData, _slaveId));

    pConnection->openTcpConnection(constructTcpSettings(_serverConnectionData.host(), _serverConnectionData.port()), 1000);

    QVERIFY(spySuccess.wait(500));

    QCOMPARE(spySuccess.count(), 1);
    QVERIFY(pConnection->isConnected());

    pConnection->closeConnection();
}

void TestModbusConnection::readRequestSuccess()
{
    /* Start server */
    QVERIFY(_pTestSlaveModbus->connect(_serverConnectionData, _slaveId));

    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterState(0, true);
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterState(1, true);

    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterValue(0, 0);
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterValue(1, 1);

    /* Open connection */
    ModbusConnection * pConnection = new ModbusConnection(this);
    QSignalSpy spySuccess(pConnection, &ModbusConnection::connectionSuccess);
    pConnection->openTcpConnection(constructTcpSettings(_serverConnectionData.host(), _serverConnectionData.port()), 1000);

    pConnection->openTcpConnection(constructTcpSettings(_serverConnectionData.host(), _serverConnectionData.port()), 1000);

    QVERIFY(spySuccess.wait(100));

    QVERIFY(pConnection->isConnected());

    QSignalSpy spyResultSuccess(pConnection, &ModbusConnection::readRequestSuccess);
    QSignalSpy spyResultProtocolError(pConnection, &ModbusConnection::readRequestProtocolError);
    QSignalSpy spyResultError(pConnection, &ModbusConnection::readRequestError);

    pConnection->sendReadRequest(ModbusAddress(40001), 2, _slaveId);

    QVERIFY(spyResultSuccess.wait(100));
    QCOMPARE(spyResultSuccess.count(), 1);
    QCOMPARE(spyResultProtocolError.count(), 0);
    QCOMPARE(spyResultError.count(), 0);

    QList<QVariant> arguments = spyResultSuccess.takeFirst();
    QCOMPARE(arguments.count(), 2);


    /* Check start address */
    QVERIFY((arguments[0].canConvert<ModbusAddress>()));
    auto resultAddr = arguments[0].value<ModbusAddress>();
    QCOMPARE(resultAddr.address(), 40001);

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
    QVERIFY(_pTestSlaveModbus->connect(_serverConnectionData, _slaveId));

    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterState(0, false);
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterState(1, true);

    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterValue(0, 0);
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterValue(1, 1);

    /* Open connection */
    ModbusConnection * pConnection = new ModbusConnection(this);
    QSignalSpy spySuccess(pConnection, &ModbusConnection::connectionSuccess);
    pConnection->openTcpConnection(constructTcpSettings(_serverConnectionData.host(), _serverConnectionData.port()), 1000);

    pConnection->openTcpConnection(constructTcpSettings(_serverConnectionData.host(), _serverConnectionData.port()), 1000);

    QVERIFY(spySuccess.wait(100));

    QVERIFY(pConnection->isConnected());

    QSignalSpy spyResultSuccess(pConnection, &ModbusConnection::readRequestSuccess);
    QSignalSpy spyResultProtocolError(pConnection, &ModbusConnection::readRequestProtocolError);
    QSignalSpy spyResultError(pConnection, &ModbusConnection::readRequestError);

    pConnection->sendReadRequest(ModbusAddress(40001), 2, _slaveId);

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

ModbusConnection::TcpSettings TestModbusConnection::constructTcpSettings(QString ip, qint32 port)
{
    struct ModbusConnection::TcpSettings tcpSettings =
    {
        .ip = ip,
        .port = port,
    };

    return tcpSettings;
}

QTEST_GUILESS_MAIN(TestModbusConnection)
