
#include <QtTest/QtTest>
#include <QMap>

#include "testslavedata.h"
#include "testslavemodbus.h"
#include "communicationhelpers.h"

#include "tst_modbuspoll.h"

Q_DECLARE_METATYPE(ResultDouble);

using Type = ModbusDataType::Type;
using State = ResultState::State;

void TestModbusPoll::init()
{
    qRegisterMetaType<ResultDouble>("ResultDouble");
    qRegisterMetaType<ResultDoubleList >("ResultDoubleList");

    _pSettingsModel = new SettingsModel;

    _pSettingsModel->setIpAddress(Connection::ID_1, "127.0.0.1");
    _pSettingsModel->setPort(Connection::ID_1, 5020);
    _pSettingsModel->setTimeout(Connection::ID_1, 500);
    _pSettingsModel->setSlaveId(Connection::ID_1, 1);

    _pSettingsModel->setConnectionState(Connection::ID_2, true);
    _pSettingsModel->setIpAddress(Connection::ID_2, "127.0.0.1");
    _pSettingsModel->setPort(Connection::ID_2, 5021);
    _pSettingsModel->setTimeout(Connection::ID_2, 500);
    _pSettingsModel->setSlaveId(Connection::ID_2, 2);

    _pSettingsModel->setConnectionState(Connection::ID_3, true);
    _pSettingsModel->setIpAddress(Connection::ID_3, "127.0.0.1");
    _pSettingsModel->setPort(Connection::ID_3, 5022);
    _pSettingsModel->setTimeout(Connection::ID_3, 500);
    _pSettingsModel->setSlaveId(Connection::ID_3, 3);

    _pSettingsModel->setPollTime(100);

    for (quint8 idx = 0; idx < Connection::ID_CNT; idx++)
    {
        _serverConnectionDataList.append(QUrl());
        _serverConnectionDataList.last().setPort(_pSettingsModel->port(idx));
        _serverConnectionDataList.last().setHost(_pSettingsModel->ipAddress(idx));

        _testSlaveDataList.append(new TestSlaveData());
        _testSlaveModbusList.append(new TestSlaveModbus(_testSlaveDataList.last()));

        QVERIFY(_testSlaveModbusList.last()->connect(_serverConnectionDataList.last(), _pSettingsModel->slaveId(idx)));
    }
}

void TestModbusPoll::cleanup()
{
    delete _pSettingsModel;

    for (int idx = 0; idx < Connection::ID_CNT; idx++)
    {
        _testSlaveModbusList[idx]->disconnectDevice();
    }

    qDeleteAll(_testSlaveDataList);
    qDeleteAll(_testSlaveModbusList);

    _serverConnectionDataList.clear();
    _testSlaveDataList.clear();
    _testSlaveModbusList.clear();
}

void TestModbusPoll::singleSlaveSuccess()
{
    _testSlaveDataList[Connection::ID_1]->setRegisterState(0, true);
    _testSlaveDataList[Connection::ID_1]->setRegisterValue(0, 5);

    _testSlaveDataList[Connection::ID_1]->setRegisterState(1, true);
    _testSlaveDataList[Connection::ID_1]->setRegisterValue(1, 65000);

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, Type::UNSIGNED_16)
                                                   << ModbusRegister(40002, Connection::ID_1, Type::UNSIGNED_16);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(50));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    auto expResults = ResultDoubleList() << ResultDouble(5, State::SUCCESS)
                                            << ResultDouble(65000, State::SUCCESS);

    /* Verify arguments of signal */
    CommunicationHelpers::verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::singleSlaveFail()
{
    for (quint8 idx = 0; idx < Connection::ID_CNT; idx++)
    {
        _testSlaveModbusList[idx]->disconnectDevice();
    }

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, Type::UNSIGNED_16)
                                                   << ModbusRegister(40002, Connection::ID_1, Type::UNSIGNED_16);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(static_cast<int>(_pSettingsModel->timeout(Connection::ID_1)) + 100));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();

    auto expResults = ResultDoubleList() << ResultDouble(0, State::ERROR)
                                            << ResultDouble(0, State::ERROR);

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

void TestModbusPoll::verifyRestartAfterStop()
{
    _testSlaveDataList[Connection::ID_1]->setRegisterState(0, true);
    _testSlaveDataList[Connection::ID_1]->setRegisterValue(0, 5);

    _testSlaveDataList[Connection::ID_1]->setRegisterState(1, true);
    _testSlaveDataList[Connection::ID_1]->setRegisterValue(1, 65000);

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, Type::UNSIGNED_16)
                                                   << ModbusRegister(40002, Connection::ID_1, Type::UNSIGNED_16);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(modbusPoll.isActive());

    QVERIFY(spyDataReady.wait(50));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    auto expResults = ResultDoubleList() << ResultDouble(5, State::SUCCESS)
                                            << ResultDouble(65000, State::SUCCESS);

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
    _testSlaveDataList[Connection::ID_1]->setRegisterState(0, true);
    _testSlaveDataList[Connection::ID_1]->setRegisterValue(0, 5020);

    _testSlaveDataList[Connection::ID_2]->setRegisterState(0, true);
    _testSlaveDataList[Connection::ID_2]->setRegisterValue(0, 5021);

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, Type::UNSIGNED_16)
                                                   << ModbusRegister(40001, Connection::ID_2, Type::UNSIGNED_16);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(50));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    auto expResults = ResultDoubleList() << ResultDouble(5020, State::SUCCESS)
                                            << ResultDouble(5021, State::SUCCESS);

    /* Verify arguments of signal */
    CommunicationHelpers::verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::multiSlaveSuccess_2()
{
    _testSlaveDataList[Connection::ID_1]->setRegisterState(0, true);
    _testSlaveDataList[Connection::ID_1]->setRegisterValue(0, 5020);

    _testSlaveDataList[Connection::ID_2]->setRegisterState(1, true);
    _testSlaveDataList[Connection::ID_2]->setRegisterValue(1, 5021);

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, Type::UNSIGNED_16)
                                                   << ModbusRegister(40002, Connection::ID_2, Type::UNSIGNED_16);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(50));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    auto expResults = ResultDoubleList() << ResultDouble(5020, State::SUCCESS)
                                            << ResultDouble(5021, State::SUCCESS);

    /* Verify arguments of signal */
    CommunicationHelpers::verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::multiSlaveSuccess_3()
{
    _testSlaveDataList[Connection::ID_1]->setRegisterState(0, true);
    _testSlaveDataList[Connection::ID_1]->setRegisterValue(0, 5020);

    _testSlaveDataList[Connection::ID_1]->setRegisterState(1, true);
    _testSlaveDataList[Connection::ID_1]->setRegisterValue(1, 5021);

    _testSlaveDataList[Connection::ID_2]->setRegisterState(1, true);
    _testSlaveDataList[Connection::ID_2]->setRegisterValue(1, 5022);

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, Type::UNSIGNED_16)
                                                   << ModbusRegister(40002, Connection::ID_2, Type::UNSIGNED_16)
                                                   << ModbusRegister(40002, Connection::ID_1, Type::UNSIGNED_16);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(50));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    auto expResults = ResultDoubleList() << ResultDouble(5020, State::SUCCESS)
                                            << ResultDouble(5022, State::SUCCESS)
                                            << ResultDouble(5021, State::SUCCESS);

    /* Verify arguments of signal */
    CommunicationHelpers::verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::multiSlaveSingleFail()
{
    _testSlaveModbusList[Connection::ID_1]->disconnectDevice();

    _testSlaveDataList[Connection::ID_2]->setRegisterState(0, true);
    _testSlaveDataList[Connection::ID_2]->setRegisterValue(0, 5021);

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, Type::UNSIGNED_16)
                                                   << ModbusRegister(40001, Connection::ID_2, Type::UNSIGNED_16);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(static_cast<int>(_pSettingsModel->timeout(Connection::ID_1)) + 100));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    auto expResults = ResultDoubleList() << ResultDouble(0, State::ERROR)
                                            << ResultDouble(5021, State::SUCCESS);

    /* Verify arguments of signal */
    CommunicationHelpers::verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::multiSlaveAllFail()
{
    for (quint8 idx = 0; idx < Connection::ID_CNT; idx++)
    {
        _testSlaveModbusList[idx]->disconnectDevice();
    }

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, Type::UNSIGNED_16)
                                                   << ModbusRegister(40001, Connection::ID_2, Type::UNSIGNED_16);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(static_cast<int>(_pSettingsModel->timeout(Connection::ID_1)) + 100));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    auto expResults = ResultDoubleList() << ResultDouble(0, State::ERROR)
                                            << ResultDouble(0, State::ERROR);

    /* Verify arguments of signal */
    CommunicationHelpers::verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::multiSlaveDisabledConnection()
{
    _testSlaveDataList[Connection::ID_1]->setRegisterState(0, true);
    _testSlaveDataList[Connection::ID_1]->setRegisterValue(0, 5020);

    _testSlaveDataList[Connection::ID_2]->setRegisterState(0, true);
    _testSlaveDataList[Connection::ID_2]->setRegisterValue(0, 5021);

    /* Disable connection */
    _pSettingsModel->setConnectionState(Connection::ID_2, false);

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, Type::UNSIGNED_16)
                                                   << ModbusRegister(40001, Connection::ID_2, Type::UNSIGNED_16);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(50));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();

    /* Disabled connections return error and zero */
    auto expResults = ResultDoubleList() << ResultDouble(5020, State::SUCCESS)
                                            << ResultDouble(0, State::ERROR);

    /* Verify arguments of signal */
    CommunicationHelpers::verifyReceivedDataSignal(arguments, expResults);
}

QTEST_GUILESS_MAIN(TestModbusPoll)
