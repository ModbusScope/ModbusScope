#include "tst_modbusmasterfake.h"

#include "communication/modbusmaster.h"
#include "modbusconnectionfake.h"

#include <QSignalSpy>
#include <QTest>

void TestModbusMasterFake::init()
{
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

    devId = Device::cFirstDeviceId + 1;
    _pSettingsModel->addDevice(devId);
    _pSettingsModel->deviceSettings(devId)->setConnectionId(ConnectionTypes::ID_1);
    _pSettingsModel->deviceSettings(devId)->setSlaveId(devId);

    const auto deviceList = _pSettingsModel->deviceList();
    for (deviceId_t devId : std::as_const(deviceList))
    {
        _testDeviceMap[devId] = new TestDevice();
    }
}

void TestModbusMasterFake::cleanup()
{
    for (auto& testDevice : _testDeviceMap)
    {
        testDevice->disconnect();
        delete testDevice;
    }

    delete _pSettingsModel;
}

void TestModbusMasterFake::singleRequestSuccess()
{
    _testDeviceMap[Device::cFirstDeviceId]->configureHoldingRegister(0, true, 0);

    ModbusConnectionFake* conn = new ModbusConnectionFake(); // ModbusMaster takes ownership
    conn->addSlaveDevice(Device::cFirstDeviceId, _testDeviceMap[Device::cFirstDeviceId]);
    ModbusMaster modbusMaster(conn, _pSettingsModel, ConnectionTypes::ID_1);

    auto registerList = QList<ModbusDataUnit>() << ModbusDataUnit(40001);
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

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

void TestModbusMasterFake::requestToDevicesOnSameConnection()
{
    _testDeviceMap[Device::cFirstDeviceId]->configureHoldingRegister(0, true, 0);
    _testDeviceMap[Device::cFirstDeviceId + 1]->configureHoldingRegister(1, true, 5);

    ModbusConnectionFake* conn = new ModbusConnectionFake(); // ModbusMaster takes ownership
    conn->addSlaveDevice(Device::cFirstDeviceId, _testDeviceMap[Device::cFirstDeviceId]);
    conn->addSlaveDevice(Device::cFirstDeviceId + 1, _testDeviceMap[Device::cFirstDeviceId + 1]);
    ModbusMaster modbusMaster(conn, _pSettingsModel, ConnectionTypes::ID_1);

    ModbusDataUnit dataUnit1(0, ModbusAddress::ObjectType::HOLDING_REGISTER, Device::cFirstDeviceId);
    ModbusDataUnit dataUnit2(1, ModbusAddress::ObjectType::HOLDING_REGISTER, Device::cFirstDeviceId + 1);
    auto registerList = QList<ModbusDataUnit>() << dataUnit1 << dataUnit2;
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    modbusMaster.readRegisterList(registerList, 128);

    QVERIFY(spyModbusPollDone.wait(100));
    QCOMPARE(spyModbusPollDone.count(), 1);

    QList<QVariant> arguments = spyModbusPollDone.takeFirst();
    QVERIFY(arguments.count() > 0);

    QVariant varResultList = arguments.first();
    QVERIFY(varResultList.canConvert<ModbusResultMap>());
    ModbusResultMap result = varResultList.value<ModbusResultMap >();
    QCOMPARE(result.size(), 2);

    QVERIFY(result[dataUnit1].isValid());
    QCOMPARE(result[dataUnit1].value(), static_cast<quint32>(0));

    QVERIFY(result[dataUnit2].isValid());
    QCOMPARE(result[dataUnit2].value(), static_cast<quint32>(5));
}

QTEST_GUILESS_MAIN(TestModbusMasterFake)
