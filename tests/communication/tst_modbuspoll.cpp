
#include "tst_modbuspoll.h"

#include "communication/modbuspoll.h"
#include "communicationhelpers.h"
#include "testslavemodbus.h"
#include "util/modbusdatatype.h"

#include <QSignalSpy>
#include <QTest>

Q_DECLARE_METATYPE(ResultDouble);

using Type = ModbusDataType::Type;
using State = ResultState::State;

void TestModbusPoll::init()
{
    qRegisterMetaType<ResultDouble>("ResultDouble");
    qRegisterMetaType<ResultDoubleList >("ResultDoubleList");

    _pSettingsModel = new SettingsModel;

    auto connData = _pSettingsModel->connectionSettings(ConnectionTypes::ID_1);
    connData->setIpAddress("127.0.0.1");
    connData->setPort(5020);
    connData->setTimeout(500);

    _pSettingsModel->setConnectionState(ConnectionTypes::ID_2, true);
    connData = _pSettingsModel->connectionSettings(ConnectionTypes::ID_2);
    connData->setIpAddress("127.0.0.1");
    connData->setPort(5021);
    connData->setTimeout(500);

    _pSettingsModel->setConnectionState(ConnectionTypes::ID_3, true);
    connData = _pSettingsModel->connectionSettings(ConnectionTypes::ID_3);
    connData->setIpAddress("127.0.0.1");
    connData->setPort(5022);
    connData->setTimeout(500);

    deviceId_t devId = Device::cFirstDeviceId;
    _pSettingsModel->addDevice(devId);
    _pSettingsModel->deviceSettings(devId)->setConnectionId(ConnectionTypes::ID_1);
    _pSettingsModel->deviceSettings(devId)->setSlaveId(devId);

    devId++;
    _pSettingsModel->addDevice(devId);
    _pSettingsModel->deviceSettings(devId)->setConnectionId(ConnectionTypes::ID_2);
    _pSettingsModel->deviceSettings(devId)->setSlaveId(devId);

    devId++;
    _pSettingsModel->addDevice(devId);
    _pSettingsModel->deviceSettings(devId)->setConnectionId(ConnectionTypes::ID_3);
    _pSettingsModel->deviceSettings(devId)->setSlaveId(devId);

    _pSettingsModel->setPollTime(100);

    const auto deviceList = _pSettingsModel->deviceList();
    for (deviceId_t devId : std::as_const(deviceList))
    {
        auto device = _pSettingsModel->deviceSettings(devId);

        connData = _pSettingsModel->connectionSettings(device->connectionId());

        _testSlaveMap[devId] = new TestSlaveModbus();
        auto& testSlave = _testSlaveMap[devId];

        QVERIFY(testSlave->connect(connData->ipAddress(), connData->port(), device->slaveId()));
    }
}

void TestModbusPoll::cleanup()
{
    for (auto& testSlave : _testSlaveMap)
    {
        testSlave->disconnect();
        delete testSlave;
    }

    delete _pSettingsModel;
}

void TestModbusPoll::singleSlaveSuccess()
{
    auto device = _testSlaveMap[Device::cFirstDeviceId]->testDevice();
    device->configureHoldingRegister(0, true, 5);
    device->configureHoldingRegister(1, true, 65000);

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::UNSIGNED_16)
                           << ModbusRegister(ModbusAddress(40002), Device::cFirstDeviceId, Type::UNSIGNED_16);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(50));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    auto expResults = ResultDoubleList() << ResultDouble(5, State::SUCCESS) << ResultDouble(65000, State::SUCCESS);

    /* Verify arguments of signal */
    CommunicationHelpers::verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::singleSlaveFail()
{
    for (auto& testSlaveModbus : _testSlaveMap)
    {
        testSlaveModbus->disconnect();
    }

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::UNSIGNED_16)
                           << ModbusRegister(ModbusAddress(40002), Device::cFirstDeviceId, Type::UNSIGNED_16);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    auto timeout = _pSettingsModel->connectionSettings(ConnectionTypes::ID_1)->timeout();
    QVERIFY(spyDataReady.wait(static_cast<int>(timeout) + 100));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();

    auto expResults = ResultDoubleList() << ResultDouble(0, State::INVALID) << ResultDouble(0, State::INVALID);

    /* Verify arguments of signal */
    CommunicationHelpers::verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::singleOnlyConstantDataPoll()
{
    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>(); /* No registers to poll */

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(50));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    auto expResults = ResultDoubleList();

    /* Verify arguments of signal */
    CommunicationHelpers::verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::singleSlaveCoil()
{
    auto device = _testSlaveMap[Device::cFirstDeviceId]->testDevice();
    device->configureCoil(0, true, false);
    device->configureCoil(2, true, true);

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(0), Device::cFirstDeviceId, Type::UNSIGNED_16)
                           << ModbusRegister(ModbusAddress(2), Device::cFirstDeviceId, Type::UNSIGNED_16);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(50));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    auto expResults = ResultDoubleList() << ResultDouble(0, State::SUCCESS) << ResultDouble(1, State::SUCCESS);

    /* Verify arguments of signal */
    CommunicationHelpers::verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::singleSlaveMixedObjects()
{
    auto device = _testSlaveMap[Device::cFirstDeviceId]->testDevice();
    device->configureCoil(0, true, false);
    device->configureDiscreteInput(0, true, true);
    device->configureInputRegister(0, true, 100);
    device->configureHoldingRegister(0, true, 101);

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(0), Device::cFirstDeviceId, Type::UNSIGNED_16)
                           << ModbusRegister(ModbusAddress(10001), Device::cFirstDeviceId, Type::UNSIGNED_16)
                           << ModbusRegister(ModbusAddress(30001), Device::cFirstDeviceId, Type::UNSIGNED_16)
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::UNSIGNED_16);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(50));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    auto expResults = ResultDoubleList() << ResultDouble(0, State::SUCCESS) << ResultDouble(1, State::SUCCESS)
                                         << ResultDouble(100, State::SUCCESS) << ResultDouble(101, State::SUCCESS);

    /* Verify arguments of signal */
    CommunicationHelpers::verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::verifyRestartAfterStop()
{
    auto device = _testSlaveMap[Device::cFirstDeviceId]->testDevice();
    device->configureHoldingRegister(0, true, 5);
    device->configureHoldingRegister(1, true, 65000);

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::UNSIGNED_16)
                           << ModbusRegister(ModbusAddress(40002), Device::cFirstDeviceId, Type::UNSIGNED_16);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(modbusPoll.isActive());

    QVERIFY(spyDataReady.wait(50));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    auto expResults = ResultDoubleList() << ResultDouble(5, State::SUCCESS) << ResultDouble(65000, State::SUCCESS);

    /* Verify arguments of signal */
    CommunicationHelpers::verifyReceivedDataSignal(arguments, expResults);

    /*-- Stop communication --*/
    modbusPoll.stopCommunication();

    QVERIFY(!modbusPoll.isActive());

    /*-- Restart communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(50));
    QCOMPARE(spyDataReady.count(), 1);

    arguments = spyDataReady.takeFirst();

    /* Verify arguments of signal */
    CommunicationHelpers::verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::multiSlaveSuccess()
{
    auto device = _testSlaveMap[Device::cFirstDeviceId]->testDevice();
    auto device_1 = _testSlaveMap[Device::cFirstDeviceId + 1]->testDevice();
    device->configureHoldingRegister(0, true, 5020);
    device_1->configureHoldingRegister(0, true, 5021);

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::UNSIGNED_16)
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId + 1, Type::UNSIGNED_16);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(50));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    auto expResults = ResultDoubleList() << ResultDouble(5020, State::SUCCESS) << ResultDouble(5021, State::SUCCESS);

    /* Verify arguments of signal */
    CommunicationHelpers::verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::multiSlaveSuccess_2()
{
    auto device = _testSlaveMap[Device::cFirstDeviceId]->testDevice();
    auto device_1 = _testSlaveMap[Device::cFirstDeviceId + 1]->testDevice();
    device->configureHoldingRegister(0, true, 5020);
    device_1->configureHoldingRegister(1, true, 5021);

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::UNSIGNED_16)
                           << ModbusRegister(ModbusAddress(40002), Device::cFirstDeviceId + 1, Type::UNSIGNED_16);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(50));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    auto expResults = ResultDoubleList() << ResultDouble(5020, State::SUCCESS) << ResultDouble(5021, State::SUCCESS);

    /* Verify arguments of signal */
    CommunicationHelpers::verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::multiSlaveSuccess_3()
{
    auto device = _testSlaveMap[Device::cFirstDeviceId]->testDevice();
    auto device_1 = _testSlaveMap[Device::cFirstDeviceId + 1]->testDevice();
    device->configureHoldingRegister(0, true, 5020);
    device->configureHoldingRegister(1, true, 5021);
    device_1->configureHoldingRegister(1, true, 5022);

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::UNSIGNED_16)
                           << ModbusRegister(ModbusAddress(40002), Device::cFirstDeviceId + 1, Type::UNSIGNED_16)
                           << ModbusRegister(ModbusAddress(40002), Device::cFirstDeviceId, Type::UNSIGNED_16);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(50));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    auto expResults = ResultDoubleList() << ResultDouble(5020, State::SUCCESS) << ResultDouble(5022, State::SUCCESS)
                                         << ResultDouble(5021, State::SUCCESS);

    /* Verify arguments of signal */
    CommunicationHelpers::verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::multiSlaveSingleFail()
{
    _testSlaveMap[Device::cFirstDeviceId]->disconnect();
    auto device_1 = _testSlaveMap[Device::cFirstDeviceId + 1]->testDevice();
    device_1->configureHoldingRegister(0, true, 5021);

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::UNSIGNED_16)
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId + 1, Type::UNSIGNED_16);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    auto timeout = _pSettingsModel->connectionSettings(ConnectionTypes::ID_1)->timeout();
    QVERIFY(spyDataReady.wait(static_cast<int>(timeout) + 100));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    auto expResults = ResultDoubleList() << ResultDouble(0, State::INVALID) << ResultDouble(5021, State::SUCCESS);

    /* Verify arguments of signal */
    CommunicationHelpers::verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::multiSlaveAllFail()
{
    for (auto& testslaveModbus : _testSlaveMap)
    {
        testslaveModbus->disconnect();
    }

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::UNSIGNED_16)
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId + 1, Type::UNSIGNED_16);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    auto timeout = _pSettingsModel->connectionSettings(ConnectionTypes::ID_1)->timeout();
    QVERIFY(spyDataReady.wait(static_cast<int>(timeout) + 100));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    auto expResults = ResultDoubleList() << ResultDouble(0, State::INVALID) << ResultDouble(0, State::INVALID);

    /* Verify arguments of signal */
    CommunicationHelpers::verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::multiSlaveDisabledConnection()
{
    auto device = _testSlaveMap[Device::cFirstDeviceId]->testDevice();
    auto device_1 = _testSlaveMap[Device::cFirstDeviceId + 1]->testDevice();
    device->configureHoldingRegister(0, true, 5020);
    device_1->configureHoldingRegister(0, true, 5021);

    /* Disable connection */
    _pSettingsModel->setConnectionState(ConnectionTypes::ID_2, false);

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::UNSIGNED_16)
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId + 1, Type::UNSIGNED_16);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(50));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();

    /* Disabled connections return error and zero */
    auto expResults = ResultDoubleList() << ResultDouble(5020, State::SUCCESS) << ResultDouble(0, State::INVALID);

    /* Verify arguments of signal */
    CommunicationHelpers::verifyReceivedDataSignal(arguments, expResults);
}

QTEST_GUILESS_MAIN(TestModbusPoll)
