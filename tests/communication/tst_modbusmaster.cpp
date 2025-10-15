#include "tst_modbusmaster.h"

#include "communication/modbusmaster.h"

#include <QSignalSpy>
#include <QTest>

Q_DECLARE_METATYPE(Result<quint16>);

void TestModbusMaster::init()
{
    qRegisterMetaType<Result<quint16>>("Result<quint16>");

    _pSettingsModel = new SettingsModel;

    auto connData = _pSettingsModel->connectionSettings(ConnectionTypes::ID_1);
    connData->setIpAddress("127.0.0.1");
    connData->setPort(5020);
    connData->setTimeout(500);

    _pSettingsModel->setPollTime(100);

    deviceId_t devId = Device::cFirstDeviceId;
    _pSettingsModel->addDevice(devId);
    _pSettingsModel->deviceSettings(devId)->setConnectionId(ConnectionTypes::ID_1);
    _pSettingsModel->deviceSettings(devId)->setSlaveId(devId);

    const auto deviceList = _pSettingsModel->deviceList();
    for (deviceId_t devId : std::as_const(deviceList))
    {
        auto device = _pSettingsModel->deviceSettings(devId);

        connData = _pSettingsModel->connectionSettings(device->connectionId());

        _testDeviceMap[devId] = new TestDevice();
        auto& testDevice = _testDeviceMap[devId];

        QVERIFY(testDevice->connect(connData->ipAddress(), connData->port(), device->slaveId()));
    }
}

void TestModbusMaster::cleanup()
{
    for (auto& testDevice : _testDeviceMap)
    {
        testDevice->disconnect();
        delete testDevice;
    }

    delete _pSettingsModel;
}

void TestModbusMaster::singleRequestSuccess()
{
    _testDeviceMap[Device::cFirstDeviceId]->configureHoldingRegister(0, true, 0);

    auto conn = new ModbusConnection(); // ModbusMaster takes ownership
    ModbusMaster modbusMaster(conn, _pSettingsModel, ConnectionTypes::ID_1);

    auto registerList = QList<ModbusDataUnit>() << ModbusDataUnit(40001);
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

        QVERIFY(result[ModbusDataUnit(40001)].isValid());
        QCOMPARE(result[ModbusDataUnit(40001)].value(), static_cast<quint32>(0));
    }
}

void TestModbusMaster::singleRequestEmpty()
{
    _testDeviceMap[Device::cFirstDeviceId]->configureHoldingRegister(0, true, 0);

    auto conn = new ModbusConnection(); // ModbusMaster takes ownership
    ModbusMaster modbusMaster(conn, _pSettingsModel, ConnectionTypes::ID_1);

    QList<ModbusDataUnit> registerList;
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
    _testDeviceMap[Device::cFirstDeviceId]->setException(QModbusPdu::GatewayTargetDeviceFailedToRespond, true);

    auto conn = new ModbusConnection(); // ModbusMaster takes ownership
    ModbusMaster modbusMaster(conn, _pSettingsModel, ConnectionTypes::ID_1);
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);
    auto registerList = QList<ModbusDataUnit>() << ModbusDataUnit(40001);

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

        QVERIFY(result[ModbusDataUnit(40001)].isValid() == false);
    }
}

void TestModbusMaster::singleRequestNoResponse()
{
    _testDeviceMap[Device::cFirstDeviceId]->disconnect();

    auto conn = new ModbusConnection(); // ModbusMaster takes ownership
    ModbusMaster modbusMaster(conn, _pSettingsModel, ConnectionTypes::ID_1);

    auto registerList = QList<ModbusDataUnit>() << ModbusDataUnit(40001);
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    for (uint i = 0; i < _cReadCount; i++)
    {
        modbusMaster.readRegisterList(registerList, 128);

        auto connData = _pSettingsModel->connectionSettings(ConnectionTypes::ID_1);
        spyModbusPollDone.wait(static_cast<int>(connData->timeout()) + 100);
        QCOMPARE(spyModbusPollDone.count(), 1);

        QList<QVariant> arguments = spyModbusPollDone.takeFirst();
        QVERIFY(arguments.count() > 0);

        QVariant varResultList = arguments.first();
        QVERIFY(varResultList.canConvert<ModbusResultMap>());
        ModbusResultMap result = varResultList.value<ModbusResultMap >();
        QCOMPARE(result.size(), 1);

        QVERIFY(result[ModbusDataUnit(40001)].isValid() == false);
    }
}

void TestModbusMaster::singleRequestInvalidAddressOnce()
{
    auto conn = new ModbusConnection(); // ModbusMaster takes ownership
    ModbusMaster modbusMaster(conn, _pSettingsModel, ConnectionTypes::ID_1);
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    auto registerList = QList<ModbusDataUnit>()
                        << ModbusDataUnit(40001) << ModbusDataUnit(40002) << ModbusDataUnit(40003);

    _testDeviceMap[Device::cFirstDeviceId]->configureHoldingRegister(0, false, 0);
    _testDeviceMap[Device::cFirstDeviceId]->configureHoldingRegister(1, true, 1);
    _testDeviceMap[Device::cFirstDeviceId]->configureHoldingRegister(2, true, 2);

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

        QVERIFY(result[ModbusDataUnit(40001)].isValid() == false);

        QVERIFY(result[ModbusDataUnit(40002)].isValid());
        QCOMPARE(result[ModbusDataUnit(40002)].value(), static_cast<quint16>(1));

        QVERIFY(result[ModbusDataUnit(40003)].isValid());
        QCOMPARE(result[ModbusDataUnit(40003)].value(), static_cast<quint16>(2));
    }
}

void TestModbusMaster::singleRequestInvalidAddressPersistent()
{
    _testDeviceMap[Device::cFirstDeviceId]->setException(QModbusPdu::IllegalDataAddress, true);

    auto conn = new ModbusConnection(); // ModbusMaster takes ownership
    ModbusMaster modbusMaster(conn, _pSettingsModel, ConnectionTypes::ID_1);
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    auto registerList = QList<ModbusDataUnit>() << ModbusDataUnit(40001);

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

        QVERIFY(result[ModbusDataUnit(40001)].isValid() == false);
    }
}

void TestModbusMaster::multiRequestSuccess()
{
    _testDeviceMap[Device::cFirstDeviceId]->configureHoldingRegister(0, true, 0);
    _testDeviceMap[Device::cFirstDeviceId]->configureHoldingRegister(1, true, 1);
    _testDeviceMap[Device::cFirstDeviceId]->configureHoldingRegister(3, true, 3);

    auto conn = new ModbusConnection(); // ModbusMaster takes ownership
    ModbusMaster modbusMaster(conn, _pSettingsModel, ConnectionTypes::ID_1);

    auto registerList = QList<ModbusDataUnit>()
                        << ModbusDataUnit(40001) << ModbusDataUnit(40002) << ModbusDataUnit(40004);
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    for (uint i = 0; i < _cReadCount; i++)
    {
        modbusMaster.readRegisterList(registerList, 128);

        auto timeout = _pSettingsModel->connectionSettings(ConnectionTypes::ID_1)->timeout();
        QVERIFY(spyModbusPollDone.wait(static_cast<int>(timeout)));
        QCOMPARE(spyModbusPollDone.count(), 1);

        QList<QVariant> arguments = spyModbusPollDone.takeFirst();
        QVERIFY(arguments.count() > 0);

        QVariant varResultList = arguments.first();
        QVERIFY(varResultList.canConvert<ModbusResultMap>());
        ModbusResultMap result = varResultList.value<ModbusResultMap >();
        QCOMPARE(result.size(), 3);

        QVERIFY(result[ModbusDataUnit(40001)].isValid());
        QCOMPARE(result[ModbusDataUnit(40001)].value(), static_cast<quint16>(0));

        QVERIFY(result[ModbusDataUnit(40002)].isValid());
        QCOMPARE(result[ModbusDataUnit(40002)].value(), static_cast<quint16>(1));

        QVERIFY(result[ModbusDataUnit(40004)].isValid());
        QCOMPARE(result[ModbusDataUnit(40004)].value(), static_cast<quint16>(3));
    }
}

void TestModbusMaster::multiRequestGatewayNotAvailable()
{
    _testDeviceMap[Device::cFirstDeviceId]->setException(QModbusPdu::IllegalDataAddress, true);

    _testDeviceMap[Device::cFirstDeviceId]->configureHoldingRegister(0, true, 0);
    _testDeviceMap[Device::cFirstDeviceId]->configureHoldingRegister(1, true, 1);
    _testDeviceMap[Device::cFirstDeviceId]->configureHoldingRegister(3, true, 3);

    auto conn = new ModbusConnection(); // ModbusMaster takes ownership
    ModbusMaster modbusMaster(conn, _pSettingsModel, ConnectionTypes::ID_1);

    auto registerList = QList<ModbusDataUnit>()
                        << ModbusDataUnit(40001) << ModbusDataUnit(40002) << ModbusDataUnit(40004);
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

        QVERIFY(result[ModbusDataUnit(40001)].isValid() == false);
        QVERIFY(result[ModbusDataUnit(40002)].isValid() == false);
        QVERIFY(result[ModbusDataUnit(40004)].isValid() == false);
    }
}

void TestModbusMaster::multiRequestNoResponse()
{
    _testDeviceMap[Device::cFirstDeviceId]->disconnect();

    _testDeviceMap[Device::cFirstDeviceId]->configureHoldingRegister(0, true, 0);
    _testDeviceMap[Device::cFirstDeviceId]->configureHoldingRegister(1, true, 1);
    _testDeviceMap[Device::cFirstDeviceId]->configureHoldingRegister(3, true, 3);

    auto conn = new ModbusConnection(); // ModbusMaster takes ownership
    ModbusMaster modbusMaster(conn, _pSettingsModel, ConnectionTypes::ID_1);

    auto registerList = QList<ModbusDataUnit>()
                        << ModbusDataUnit(40001) << ModbusDataUnit(40002) << ModbusDataUnit(40004);
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    for (uint i = 0; i < _cReadCount; i++)
    {
        modbusMaster.readRegisterList(registerList, 128);

        auto timeout = _pSettingsModel->connectionSettings(ConnectionTypes::ID_1)->timeout();
        spyModbusPollDone.wait(static_cast<int>(timeout) + 100);
        QCOMPARE(spyModbusPollDone.count(), 1);

        QList<QVariant> arguments = spyModbusPollDone.takeFirst();
        QVERIFY(arguments.count() > 0);

        QVariant varResultList = arguments.first();
        QVERIFY(varResultList.canConvert<ModbusResultMap>());
        ModbusResultMap result = varResultList.value<ModbusResultMap >();
        QCOMPARE(result.size(), 3);

        QVERIFY(result[ModbusDataUnit(40001)].isValid() == false);
        QVERIFY(result[ModbusDataUnit(40002)].isValid() == false);
        QVERIFY(result[ModbusDataUnit(40004)].isValid() == false);
    }
}

void TestModbusMaster::multiRequestInvalidAddress()
{
    _testDeviceMap[Device::cFirstDeviceId]->setException(QModbusPdu::IllegalDataAddress, true);

    _testDeviceMap[Device::cFirstDeviceId]->configureHoldingRegister(0, true, 0);
    _testDeviceMap[Device::cFirstDeviceId]->configureHoldingRegister(1, true, 1);
    _testDeviceMap[Device::cFirstDeviceId]->configureHoldingRegister(3, true, 3);

    auto conn = new ModbusConnection(); // ModbusMaster takes ownership
    ModbusMaster modbusMaster(conn, _pSettingsModel, ConnectionTypes::ID_1);

    auto registerList = QList<ModbusDataUnit>()
                        << ModbusDataUnit(40001) << ModbusDataUnit(40002) << ModbusDataUnit(40004);
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    for (uint i = 0; i < _cReadCount; i++)
    {
        modbusMaster.readRegisterList(registerList, 128);

        auto timeout = _pSettingsModel->connectionSettings(ConnectionTypes::ID_1)->timeout();
        QVERIFY(spyModbusPollDone.wait(static_cast<int>(timeout) + 100));
        QCOMPARE(spyModbusPollDone.count(), 1);

        QList<QVariant> arguments = spyModbusPollDone.takeFirst();
        QVERIFY(arguments.count() > 0);

        QVariant varResultList = arguments.first();
        QVERIFY(varResultList.canConvert<ModbusResultMap>());
        ModbusResultMap result = varResultList.value<ModbusResultMap >();
        QCOMPARE(result.size(), 3);

        QVERIFY(result[ModbusDataUnit(40001)].isValid() == false);
        QVERIFY(result[ModbusDataUnit(40002)].isValid() == false);
        QVERIFY(result[ModbusDataUnit(40004)].isValid() == false);
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
