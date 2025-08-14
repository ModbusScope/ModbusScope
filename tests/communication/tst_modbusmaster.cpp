
#include "tst_modbusmaster.h"

#include "communication/modbusmaster.h"
#include "testslavedata.h"
#include "testslavemodbus.h"

#include <QMap>
#include <QMetaType>
#include <QSignalSpy>
#include <QTest>

Q_DECLARE_METATYPE(Result<quint16>);

void TestModbusMaster::init()
{
    qRegisterMetaType<Result<quint16>>("Result<quint16>");

    auto connData = _settingsModel.connectionSettings(ConnectionId::ID_1);

    connData->setIpAddress("127.0.0.1");
    connData->setPort(5020);
    connData->setTimeout(500);
    connData->setSlaveId(1);

    _serverConnectionData.setPort(connData->port());
    _serverConnectionData.setHost(connData->ipAddress());

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

    QVERIFY(_pTestSlaveModbus->connect(_serverConnectionData,
                                       _settingsModel.connectionSettings(ConnectionId::ID_1)->slaveId()));
}

void TestModbusMaster::cleanup()
{
    _pTestSlaveModbus->disconnectDevice();

    if (!_testSlaveData.isEmpty())
    {
        qDeleteAll(_testSlaveData);
        _testSlaveData.clear();
    }
    delete _pTestSlaveModbus;
}

void TestModbusMaster::singleRequestSuccess()
{
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterState(0, true);

    ModbusMaster modbusMaster(&_settingsModel, ConnectionId::ID_1);

    auto registerList = QList<ModbusAddress>() << ModbusAddress(40001);
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    for (uint i = 0; i < _cReadCount; i++)
    {
        modbusMaster.readRegisterList(registerList, 128);

        QVERIFY(spyModbusPollDone.wait(100));
        QCOMPARE(spyModbusPollDone.count(), 1);

        QList<QVariant> arguments = spyModbusPollDone.takeFirst();
        QVERIFY(arguments.count() > 0);

        QVariant varResultList = arguments.first();
        QVERIFY(varResultList.canConvert<ModbusResultMap>());
        ModbusResultMap result = varResultList.value<ModbusResultMap >();
        QCOMPARE(result.size(), 1);

        QVERIFY(result[ModbusAddress(40001)].isValid());
        QCOMPARE(result[ModbusAddress(40001)].value(), static_cast<quint32>(0));
    }
}

void TestModbusMaster::singleRequestEmpty()
{
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterState(0, true);

    ModbusMaster modbusMaster(&_settingsModel, ConnectionId::ID_1);

    QList<ModbusAddress> registerList;
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    modbusMaster.readRegisterList(registerList, 128);

    QCOMPARE(spyModbusPollDone.count(), 1);

    QList<QVariant> arguments = spyModbusPollDone.takeFirst();
    QVERIFY(arguments.count() > 0);

    QVariant varResultList = arguments.first();
    QVERIFY(varResultList.canConvert<ModbusResultMap>());
    ModbusResultMap result = varResultList.value<ModbusResultMap >();
    QCOMPARE(result.size(), 0);
}

void TestModbusMaster::singleRequestGatewayNotAvailable()
{
    _pTestSlaveModbus->setException(QModbusPdu::GatewayTargetDeviceFailedToRespond, true);

    ModbusMaster modbusMaster(&_settingsModel, ConnectionId::ID_1);
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);
    auto registerList = QList<ModbusAddress>() << ModbusAddress(40001);

    for (uint i = 0; i < _cReadCount; i++)
    {
        modbusMaster.readRegisterList(registerList, 128);

        QVERIFY(spyModbusPollDone.wait(100));
        QCOMPARE(spyModbusPollDone.count(), 1);

        QList<QVariant> arguments = spyModbusPollDone.takeFirst();
        QVERIFY(arguments.count() > 0);

        QVariant varResultList = arguments.first();
        QVERIFY(varResultList.canConvert<ModbusResultMap>());
        ModbusResultMap result = varResultList.value<ModbusResultMap >();
        QCOMPARE(result.size(), 1);

        QVERIFY(result[ModbusAddress(40001)].isValid() == false);
    }
}

void TestModbusMaster::singleRequestNoResponse()
{
    _pTestSlaveModbus->disconnectDevice();

    ModbusMaster modbusMaster(&_settingsModel, ConnectionId::ID_1);

    auto registerList = QList<ModbusAddress>() << ModbusAddress(40001);
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    for (uint i = 0; i < _cReadCount; i++)
    {
        modbusMaster.readRegisterList(registerList, 128);

        auto connData = _settingsModel.connectionSettings(ConnectionId::ID_1);
        spyModbusPollDone.wait(static_cast<int>(connData->timeout()) + 100);
        QCOMPARE(spyModbusPollDone.count(), 1);

        QList<QVariant> arguments = spyModbusPollDone.takeFirst();
        QVERIFY(arguments.count() > 0);

        QVariant varResultList = arguments.first();
        QVERIFY(varResultList.canConvert<ModbusResultMap>());
        ModbusResultMap result = varResultList.value<ModbusResultMap >();
        QCOMPARE(result.size(), 1);

        QVERIFY(result[ModbusAddress(40001)].isValid() == false);
    }
}

void TestModbusMaster::singleRequestInvalidAddressOnce()
{
    ModbusMaster modbusMaster(&_settingsModel, ConnectionId::ID_1);
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    auto registerList = QList<ModbusAddress>() << ModbusAddress(40001) << ModbusAddress(40002) << ModbusAddress(40003);

    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterState(0, false);
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterState(1, true);
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterState(2, true);

    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterValue(1, 1);
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterValue(2, 2);

    for (uint i = 0; i < _cReadCount; i++)
    {
        modbusMaster.readRegisterList(registerList, 128);

        QVERIFY(spyModbusPollDone.wait(100));
        QCOMPARE(spyModbusPollDone.count(), 1);

        QList<QVariant> arguments = spyModbusPollDone.takeFirst();
        QVERIFY(arguments.count() > 0);

        QVariant varResultList = arguments.first();
        QVERIFY(varResultList.canConvert<ModbusResultMap>());
        ModbusResultMap result = varResultList.value<ModbusResultMap >();
        QCOMPARE(result.size(), 3);

        QVERIFY(result[ModbusAddress(40001)].isValid() == false);

        QVERIFY(result[ModbusAddress(40002)].isValid());
        QCOMPARE(result[ModbusAddress(40002)].value(), static_cast<quint16>(1));

        QVERIFY(result[ModbusAddress(40003)].isValid());
        QCOMPARE(result[ModbusAddress(40003)].value(), static_cast<quint16>(2));

    }
}

void TestModbusMaster::singleRequestInvalidAddressPersistent()
{
    _pTestSlaveModbus->setException(QModbusPdu::IllegalDataAddress, true);

    ModbusMaster modbusMaster(&_settingsModel, ConnectionId::ID_1);
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    auto registerList = QList<ModbusAddress>() << ModbusAddress(40001);

    for (uint i = 0; i < _cReadCount; i++)
    {
        modbusMaster.readRegisterList(registerList, 128);

        QVERIFY(spyModbusPollDone.wait(100));
        QCOMPARE(spyModbusPollDone.count(), 1);

        QList<QVariant> arguments = spyModbusPollDone.takeFirst();
        QVERIFY(arguments.count() > 0);

        QVariant varResultList = arguments.first();
        QVERIFY(varResultList.canConvert<ModbusResultMap>());
        ModbusResultMap result = varResultList.value<ModbusResultMap >();
        QCOMPARE(result.size(), 1);

        QVERIFY(result[ModbusAddress(40001)].isValid() == false);
    }
}

void TestModbusMaster::multiRequestSuccess()
{
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterState(0, true);
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterState(1, true);
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterState(3, true);

    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterValue(0, 0);
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterValue(1, 1);
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterValue(3, 3);

    ModbusMaster modbusMaster(&_settingsModel, ConnectionId::ID_1);

    auto registerList = QList<ModbusAddress>() << ModbusAddress(40001) << ModbusAddress(40002) << ModbusAddress(40004);
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    for (uint i = 0; i < _cReadCount; i++)
    {
        modbusMaster.readRegisterList(registerList, 128);

        auto timeout = _settingsModel.connectionSettings(ConnectionId::ID_1)->timeout();
        QVERIFY(spyModbusPollDone.wait(static_cast<int>(timeout)));
        QCOMPARE(spyModbusPollDone.count(), 1);

        QList<QVariant> arguments = spyModbusPollDone.takeFirst();
        QVERIFY(arguments.count() > 0);

        QVariant varResultList = arguments.first();
        QVERIFY(varResultList.canConvert<ModbusResultMap>());
        ModbusResultMap result = varResultList.value<ModbusResultMap >();
        QCOMPARE(result.size(), 3);

        QVERIFY(result[ModbusAddress(40001)].isValid());
        QCOMPARE(result[ModbusAddress(40001)].value(), static_cast<quint16>(0));

        QVERIFY(result[ModbusAddress(40002)].isValid());
        QCOMPARE(result[ModbusAddress(40002)].value(), static_cast<quint16>(1));

        QVERIFY(result[ModbusAddress(40004)].isValid());
        QCOMPARE(result[ModbusAddress(40004)].value(), static_cast<quint16>(3));
    }
}

void TestModbusMaster::multiRequestGatewayNotAvailable()
{
    _pTestSlaveModbus->setException(QModbusPdu::IllegalDataAddress, true);

    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterState(0, true);
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterState(1, true);
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterState(3, true);

    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterValue(0, 0);
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterValue(1, 1);
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterValue(3, 3);

    ModbusMaster modbusMaster(&_settingsModel, ConnectionId::ID_1);

    auto registerList = QList<ModbusAddress>() << ModbusAddress(40001) << ModbusAddress(40002) << ModbusAddress(40004);
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    for (uint i = 0; i < _cReadCount; i++)
    {
        modbusMaster.readRegisterList(registerList, 128);

        QVERIFY(spyModbusPollDone.wait(100));
        QCOMPARE(spyModbusPollDone.count(), 1);

        QList<QVariant> arguments = spyModbusPollDone.takeFirst();
        QVERIFY(arguments.count() > 0);

        QVariant varResultList = arguments.first();
        QVERIFY(varResultList.canConvert<ModbusResultMap>());
        ModbusResultMap result = varResultList.value<ModbusResultMap >();
        QCOMPARE(result.size(), 3);

        QVERIFY(result[ModbusAddress(40001)].isValid() == false);
        QVERIFY(result[ModbusAddress(40002)].isValid() == false);
        QVERIFY(result[ModbusAddress(40004)].isValid() == false);
    }
}

void TestModbusMaster::multiRequestNoResponse()
{

    _pTestSlaveModbus->disconnectDevice();

    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterState(0, true);
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterState(1, true);
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterState(3, true);

    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterValue(0, 0);
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterValue(1, 1);
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterValue(3, 3);

    ModbusMaster modbusMaster(&_settingsModel, ConnectionId::ID_1);

    auto registerList = QList<ModbusAddress>() << ModbusAddress(40001) << ModbusAddress(40002) << ModbusAddress(40004);
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    for (uint i = 0; i < _cReadCount; i++)
    {
        modbusMaster.readRegisterList(registerList, 128);

        auto timeout = _settingsModel.connectionSettings(ConnectionId::ID_1)->timeout();
        spyModbusPollDone.wait(static_cast<int>(timeout) + 100);
        QCOMPARE(spyModbusPollDone.count(), 1);

        QList<QVariant> arguments = spyModbusPollDone.takeFirst();
        QVERIFY(arguments.count() > 0);

        QVariant varResultList = arguments.first();
        QVERIFY(varResultList.canConvert<ModbusResultMap>());
        ModbusResultMap result = varResultList.value<ModbusResultMap >();
        QCOMPARE(result.size(), 3);

        QVERIFY(result[ModbusAddress(40001)].isValid() == false);
        QVERIFY(result[ModbusAddress(40002)].isValid() == false);
        QVERIFY(result[ModbusAddress(40004)].isValid() == false);
    }
}

void TestModbusMaster::multiRequestInvalidAddress()
{
    _pTestSlaveModbus->setException(QModbusPdu::IllegalDataAddress, true);

    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterState(0, true);
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterState(1, true);
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterState(3, true);

    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterValue(0, 0);
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterValue(1, 1);
    _testSlaveData[QModbusDataUnit::HoldingRegisters]->setRegisterValue(3, 3);

    ModbusMaster modbusMaster(&_settingsModel, ConnectionId::ID_1);

    auto registerList = QList<ModbusAddress>() << ModbusAddress(40001) << ModbusAddress(40002) << ModbusAddress(40004);
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    for (uint i = 0; i < _cReadCount; i++)
    {
        modbusMaster.readRegisterList(registerList, 128);

        auto timeout = _settingsModel.connectionSettings(ConnectionId::ID_1)->timeout();
        QVERIFY(spyModbusPollDone.wait(static_cast<int>(timeout) + 100));
        QCOMPARE(spyModbusPollDone.count(), 1);

        QList<QVariant> arguments = spyModbusPollDone.takeFirst();
        QVERIFY(arguments.count() > 0);

        QVariant varResultList = arguments.first();
        QVERIFY(varResultList.canConvert<ModbusResultMap>());
        ModbusResultMap result = varResultList.value<ModbusResultMap >();
        QCOMPARE(result.size(), 3);

        QVERIFY(result[ModbusAddress(40001)].isValid() == false);
        QVERIFY(result[ModbusAddress(40002)].isValid() == false);
        QVERIFY(result[ModbusAddress(40004)].isValid() == false);
    }
}


/* TODO:
 * Add extra test with actual timeout of no response
 * When test slave is disconnected, the port is closed and the error will come directly
 * and not after a response
 *
 * But this test is still valuable.
 * This test when ModbusControl server is not enabled
 */

QTEST_GUILESS_MAIN(TestModbusMaster)
