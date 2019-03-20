
#include <QtTest/QtTest>
#include <QMap>

#include "modbusmaster.h"
#include "testslavedata.h"
#include "testslavemodbus.h"

#include "testmodbusmaster.h"

#include <QMetaType>

typedef QMap<quint16,ModbusResult> ModbusResultMap;
Q_DECLARE_METATYPE(ModbusResultMap);
Q_DECLARE_METATYPE(ModbusResult);

void TestModbusMaster::init()
{
    qRegisterMetaType<ModbusResult>("ModbusResult");
    qRegisterMetaType<QMap<quint16, ModbusResult> >("QMap<quint16, ModbusResult>");

    _settingsModel.setIpAddress("127.0.0.1");
    _settingsModel.setPort(5020);
    _settingsModel.setTimeout(500);
    _settingsModel.setSlaveId(1);

    _serverConnectionData.setPort(_settingsModel.port());
    _serverConnectionData.setHost(_settingsModel.ipAddress());

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

    QVERIFY(_pTestSlaveModbus->connect(_serverConnectionData, _settingsModel.slaveId()));

}

void TestModbusMaster::cleanup()
{
    _pTestSlaveModbus->disconnectDevice();

    delete _pTestSlaveData;
    delete _pTestSlaveModbus;
}

void TestModbusMaster::singleRequestSuccess()
{
    _pTestSlaveData->setRegisterState(0, true);

    ModbusMaster modbusMaster(&_settingsModel, 0); // TODO

    QList<quint16> registerList = QList<quint16>() << 40001;
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    for (uint i = 0; i < _cReadCount; i++)
    {
        modbusMaster.readRegisterList(registerList);

        QVERIFY(spyModbusPollDone.wait(100));
        QCOMPARE(spyModbusPollDone.count(), 1);

        QList<QVariant> arguments = spyModbusPollDone.takeFirst(); // take the first signal
        QCOMPARE(arguments.count(), 2);

        QVariant varResultList = arguments.first();
        QVERIFY((varResultList.canConvert<QMap<quint16,ModbusResult> >()));
        QMap<quint16, ModbusResult> result = varResultList.value<QMap<quint16, ModbusResult> >();
        QCOMPARE(result.keys().count(), 1);

        QVERIFY(result[40001].isSuccess());
        QCOMPARE(result[40001].value(), static_cast<quint16>(0));
    }
}

void TestModbusMaster::singleRequestGatewayNotAvailable()
{
    _pTestSlaveModbus->setException(QModbusPdu::GatewayTargetDeviceFailedToRespond, true);

    ModbusMaster modbusMaster(&_settingsModel, 0); // TODO: FIX this
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);
    QList<quint16> registerList = QList<quint16>() << 40001;

    for (uint i = 0; i < _cReadCount; i++)
    {
        modbusMaster.readRegisterList(registerList);

        QVERIFY(spyModbusPollDone.wait(100));
        QCOMPARE(spyModbusPollDone.count(), 1);

        QList<QVariant> arguments = spyModbusPollDone.takeFirst(); // take the first signal
        QCOMPARE(arguments.count(), 2);

        QVariant varResultList = arguments.first();
        QVERIFY((varResultList.canConvert<QMap<quint16,ModbusResult> >()));
        QMap<quint16, ModbusResult> result = varResultList.value<QMap<quint16, ModbusResult> >();
        QCOMPARE(result.keys().count(), 1);

        QVERIFY(result[40001].isSuccess() == false);
    }
}

void TestModbusMaster::singleRequestNoResponse()
{
    _pTestSlaveModbus->disconnectDevice();

    ModbusMaster modbusMaster(&_settingsModel, 0); // TODO: FIX this

    QList<quint16> registerList = QList<quint16>() << 40001;
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    for (uint i = 0; i < _cReadCount; i++)
    {
        modbusMaster.readRegisterList(registerList);

        spyModbusPollDone.wait(static_cast<int>(100));
        QCOMPARE(spyModbusPollDone.count(), 1);

        QList<QVariant> arguments = spyModbusPollDone.takeFirst(); // take the first signal
        QCOMPARE(arguments.count(), 2);

        QVariant varResultList = arguments.first();
        QVERIFY((varResultList.canConvert<QMap<quint16,ModbusResult> >()));
        QMap<quint16, ModbusResult> result = varResultList.value<QMap<quint16, ModbusResult> >();
        QCOMPARE(result.keys().count(), 1);

        QVERIFY(result[40001].isSuccess() == false);
    }
}

void TestModbusMaster::singleRequestInvalidAddressOnce()
{
    ModbusMaster modbusMaster(&_settingsModel, 0); // TODO: FIX this
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    QList<quint16> registerList = QList<quint16>() << 40001 << 40002 << 40003;

    _pTestSlaveData->setRegisterState(0, false);
    _pTestSlaveData->setRegisterState(1, true);
    _pTestSlaveData->setRegisterState(2, true);

    _pTestSlaveData->setRegisterValue(1, 1);
    _pTestSlaveData->setRegisterValue(2, 2);

    for (uint i = 0; i < _cReadCount; i++)
    {
        modbusMaster.readRegisterList(registerList);

        QVERIFY(spyModbusPollDone.wait(100));
        QCOMPARE(spyModbusPollDone.count(), 1);

        QList<QVariant> arguments = spyModbusPollDone.takeFirst(); // take the first signal
        QCOMPARE(arguments.count(), 2);

        QVariant varResultList = arguments.first();
        QVERIFY((varResultList.canConvert<QMap<quint16,ModbusResult> >()));
        QMap<quint16, ModbusResult> result = varResultList.value<QMap<quint16, ModbusResult> >();
        QCOMPARE(result.keys().count(), 3);

        QVERIFY(result[40001].isSuccess() == false);

        QVERIFY(result[40002].isSuccess());
        QCOMPARE(result[40002].value(), static_cast<quint16>(1));

        QVERIFY(result[40003].isSuccess());
        QCOMPARE(result[40003].value(), static_cast<quint16>(2));

    }
}

void TestModbusMaster::singleRequestInvalidAddressPersistent()
{
    _pTestSlaveModbus->setException(QModbusPdu::IllegalDataAddress, true);

    ModbusMaster modbusMaster(&_settingsModel, 0); // TODO: FIX this
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    QList<quint16> registerList = QList<quint16>() << 40001;

    for (uint i = 0; i < _cReadCount; i++)
    {
        modbusMaster.readRegisterList(registerList);

        QVERIFY(spyModbusPollDone.wait(100));
        QCOMPARE(spyModbusPollDone.count(), 1);

        QList<QVariant> arguments = spyModbusPollDone.takeFirst(); // take the first signal
        QCOMPARE(arguments.count(), 2);

        QVariant varResultList = arguments.first();
        QVERIFY((varResultList.canConvert<QMap<quint16,ModbusResult> >()));
        QMap<quint16, ModbusResult> result = varResultList.value<QMap<quint16, ModbusResult> >();
        QCOMPARE(result.keys().count(), 1);

        QVERIFY(result[40001].isSuccess() == false);


    }
}

void TestModbusMaster::multiRequestSuccess()
{
    _pTestSlaveData->setRegisterState(0, true);
    _pTestSlaveData->setRegisterState(1, true);
    _pTestSlaveData->setRegisterState(3, true);

    _pTestSlaveData->setRegisterValue(0, 0);
    _pTestSlaveData->setRegisterValue(1, 1);
    _pTestSlaveData->setRegisterValue(3, 3);

    ModbusMaster modbusMaster(&_settingsModel, 0); // TODO: FIX this

    QList<quint16> registerList = QList<quint16>() << 40001 << 40002 << 40004;
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    for (uint i = 0; i < _cReadCount; i++)
    {
        modbusMaster.readRegisterList(registerList);

        QVERIFY(spyModbusPollDone.wait(static_cast<int>(_settingsModel.timeout())));
        QCOMPARE(spyModbusPollDone.count(), 1);

        QList<QVariant> arguments = spyModbusPollDone.takeFirst(); // take the first signal
        QCOMPARE(arguments.count(), 2);

        QVariant varResultList = arguments.first();
        QVERIFY((varResultList.canConvert<QMap<quint16,ModbusResult> >()));
        QMap<quint16, ModbusResult> result = varResultList.value<QMap<quint16, ModbusResult> >();
        QCOMPARE(result.keys().count(), 3);

        QVERIFY(result[40001].isSuccess());
        QCOMPARE(result[40001].value(), static_cast<quint16>(0));

        QVERIFY(result[40002].isSuccess());
        QCOMPARE(result[40002].value(), static_cast<quint16>(1));

        QVERIFY(result[40004].isSuccess());
        QCOMPARE(result[40004].value(), static_cast<quint16>(3));
    }
}

void TestModbusMaster::multiRequestGatewayNotAvailable()
{
    _pTestSlaveModbus->setException(QModbusPdu::IllegalDataAddress, true);

    _pTestSlaveData->setRegisterState(0, true);
    _pTestSlaveData->setRegisterState(1, true);
    _pTestSlaveData->setRegisterState(3, true);

    _pTestSlaveData->setRegisterValue(0, 0);
    _pTestSlaveData->setRegisterValue(1, 1);
    _pTestSlaveData->setRegisterValue(3, 3);

    ModbusMaster modbusMaster(&_settingsModel, 0); // TODO: FIX this

    QList<quint16> registerList = QList<quint16>() << 40001 << 40002 << 40004;
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    for (uint i = 0; i < _cReadCount; i++)
    {
        modbusMaster.readRegisterList(registerList);

        QVERIFY(spyModbusPollDone.wait(100));
        QCOMPARE(spyModbusPollDone.count(), 1);

        QList<QVariant> arguments = spyModbusPollDone.takeFirst(); // take the first signal
        QCOMPARE(arguments.count(), 2);

        QVariant varResultList = arguments.first();
        QVERIFY((varResultList.canConvert<QMap<quint16,ModbusResult> >()));
        QMap<quint16, ModbusResult> result = varResultList.value<QMap<quint16, ModbusResult> >();
        QCOMPARE(result.keys().count(), 3);

        QVERIFY(result[40001].isSuccess() == false);
        QVERIFY(result[40002].isSuccess() == false);
        QVERIFY(result[40004].isSuccess() == false);
    }
}

void TestModbusMaster::multiRequestNoResponse()
{

    _pTestSlaveModbus->disconnectDevice();

    _pTestSlaveData->setRegisterState(0, true);
    _pTestSlaveData->setRegisterState(1, true);
    _pTestSlaveData->setRegisterState(3, true);

    _pTestSlaveData->setRegisterValue(0, 0);
    _pTestSlaveData->setRegisterValue(1, 1);
    _pTestSlaveData->setRegisterValue(3, 3);

    ModbusMaster modbusMaster(&_settingsModel, 0); // TODO: FIX this

    QList<quint16> registerList = QList<quint16>() << 40001 << 40002 << 40004;
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    for (uint i = 0; i < _cReadCount; i++)
    {
        modbusMaster.readRegisterList(registerList);

        spyModbusPollDone.wait(static_cast<int>(100));
        QCOMPARE(spyModbusPollDone.count(), 1);

        QList<QVariant> arguments = spyModbusPollDone.takeFirst(); // take the first signal
        QCOMPARE(arguments.count(), 2);

        QVariant varResultList = arguments.first();
        QVERIFY((varResultList.canConvert<QMap<quint16,ModbusResult> >()));
        QMap<quint16, ModbusResult> result = varResultList.value<QMap<quint16, ModbusResult> >();
        QCOMPARE(result.keys().count(), 3);

        QVERIFY(result[40001].isSuccess() == false);
        QVERIFY(result[40002].isSuccess() == false);
        QVERIFY(result[40004].isSuccess() == false);
    }
}

void TestModbusMaster::multiRequestInvalidAddress()
{
    _pTestSlaveModbus->setException(QModbusPdu::IllegalDataAddress, true);

    _pTestSlaveData->setRegisterState(0, true);
    _pTestSlaveData->setRegisterState(1, true);
    _pTestSlaveData->setRegisterState(3, true);

    _pTestSlaveData->setRegisterValue(0, 0);
    _pTestSlaveData->setRegisterValue(1, 1);
    _pTestSlaveData->setRegisterValue(3, 3);

    ModbusMaster modbusMaster(&_settingsModel, 0); // TODO: FIX this

    QList<quint16> registerList = QList<quint16>() << 40001 << 40002 << 40004;
    QSignalSpy spyModbusPollDone(&modbusMaster, &ModbusMaster::modbusPollDone);

    for (uint i = 0; i < _cReadCount; i++)
    {
        modbusMaster.readRegisterList(registerList);

        QVERIFY(spyModbusPollDone.wait(static_cast<int>(_settingsModel.timeout()) + 100));
        QCOMPARE(spyModbusPollDone.count(), 1);

        QList<QVariant> arguments = spyModbusPollDone.takeFirst(); // take the first signal
        QCOMPARE(arguments.count(), 2);

        QVariant varResultList = arguments.first();
        QVERIFY((varResultList.canConvert<QMap<quint16,ModbusResult> >()));
        QMap<quint16, ModbusResult> result = varResultList.value<QMap<quint16, ModbusResult> >();
        QCOMPARE(result.keys().count(), 3);

        QVERIFY(result[40001].isSuccess() == false);
        QVERIFY(result[40002].isSuccess() == false);
        QVERIFY(result[40004].isSuccess() == false);
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


/*
 *

 explicit ModbusMaster(SettingsModel * pSettingsModel);
 *
 *    void readRegisterList(QList<quint16> registerList);

signals:
    void modbusPollDone(QMap<quint16, ModbusResult> modbusResults);
    void modbusAddToStats(quint32 successes, quint32 errors);
    void modbusLogError(QString msg);
    void modbusLogInfo(QString msg);

 *
 */
