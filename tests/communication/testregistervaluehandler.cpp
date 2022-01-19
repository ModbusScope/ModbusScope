
#include <QtTest/QtTest>

#include "testregistervaluehandler.h"

#include "settingsmodel.h"

Q_DECLARE_METATYPE(Result);


void TestRegisterValueHandler::init()
{
    qRegisterMetaType<Result>("Result");
    qRegisterMetaType<QList<Result> >("QList<Result>");

    _pSettingsModel = new SettingsModel();

    _pSettingsModel->setInt32LittleEndian(Connection::ID_1, true);
    _pSettingsModel->setInt32LittleEndian(Connection::ID_2, true);
}

void TestRegisterValueHandler::cleanup()
{
    delete _pSettingsModel;
}

void TestRegisterValueHandler::addressList_16()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, false, true);
    auto expRegisterList = QList<uint16_t>() << 40001;

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<quint16> actualRegisterList;
    regHandler.registerAddresList(actualRegisterList, Connection::ID_1);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressList_16_multiple()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, false, true)
                                                   << ModbusRegister(40002, Connection::ID_1, false, true);
    auto expRegisterList = QList<uint16_t>() << 40001 << 40002;

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<quint16> actualRegisterList;
    regHandler.registerAddresList(actualRegisterList, Connection::ID_1);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressList_32()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, true, true);
    auto expRegisterList = QList<uint16_t>() << 40001 << 40002;

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<quint16> actualRegisterList;
    regHandler.registerAddresList(actualRegisterList, Connection::ID_1);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressList_32_multiple()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, true, true)
                                                   << ModbusRegister(40005, Connection::ID_1, true, true);
    auto expRegisterList = QList<uint16_t>() << 40001 << 40002 << 40005 << 40006;

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<quint16> actualRegisterList;
    regHandler.registerAddresList(actualRegisterList, Connection::ID_1);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressListCombinations()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, false, true)
                                                   << ModbusRegister(40005, Connection::ID_1, false, true)
                                                   << ModbusRegister(40005, Connection::ID_1, true, true)
                                                   << ModbusRegister(40008, Connection::ID_1, true, true);
    auto expRegisterList = QList<uint16_t>() << 40001 << 40005 << 40006 << 40008 << 40009;

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<quint16> actualRegisterList;
    regHandler.registerAddresList(actualRegisterList, Connection::ID_1);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressListMultipleConnections()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, true, true)
                                                   << ModbusRegister(40005, Connection::ID_2, true, true)
                                                   << ModbusRegister(40010, Connection::ID_1, true, true);
    auto expRegisterList0 = QList<uint16_t>() << 40001 << 40002 << 40010 << 40011;
    auto expRegisterList1 = QList<uint16_t>() << 40005 << 40006;

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<quint16> actualRegisterList0;
    QList<quint16> actualRegisterList1;
    regHandler.registerAddresList(actualRegisterList0, Connection::ID_1);
    regHandler.registerAddresList(actualRegisterList1, Connection::ID_2);

    QVERIFY(actualRegisterList0 == expRegisterList0);
    QVERIFY(actualRegisterList1 == expRegisterList1);
}

void TestRegisterValueHandler::read_16()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, false, true)
                                                   << ModbusRegister(40002, Connection::ID_1, false, false);
    QMap<quint16, Result> partialResultMap;
    addToResultMap(partialResultMap, 40001, false, 256, true);
    addToResultMap(partialResultMap, 40002, false, -100, true);

    auto expResults = QList<Result>() << Result(256, true)
                                            << Result(-100, true);

    verifyRegisterResult(modbusRegisters, partialResultMap, expResults);
}

void TestRegisterValueHandler::read_32()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, true, true)
                                                   << ModbusRegister(40005, Connection::ID_1, true, false);
    QMap<quint16, Result> partialResultMap;
    addToResultMap(partialResultMap, 40001, true, 1000000, true);
    addToResultMap(partialResultMap, 40005, true, -100000, true);

    auto expResults = QList<Result>() << Result(1000000, true)
                                            << Result(-100000, true);

    verifyRegisterResult(modbusRegisters, partialResultMap, expResults);
}

void TestRegisterValueHandler::readBigEndian_32()
{
    _pSettingsModel->setInt32LittleEndian(Connection::ID_1, false);

    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, true, true);
    QMap<quint16, Result> partialResultMap;
    quint32 value = 1000000;

    partialResultMap.insert(40001, Result(static_cast<quint32>(value) >> 16, true));
    partialResultMap.insert(40001 + 1, Result(static_cast<quint16>(value), true));

    auto expResults = QList<Result>() << Result(1000000, true);

    verifyRegisterResult(modbusRegisters, partialResultMap, expResults);
}

void TestRegisterValueHandler::readBigEndian_s32()
{
    _pSettingsModel->setInt32LittleEndian(Connection::ID_1, false);

    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, true, true);
    QMap<quint16, Result> partialResultMap;
    quint32 value = -1000000;

    partialResultMap.insert(40001, Result(static_cast<quint32>(value) >> 16, true));
    partialResultMap.insert(40001 + 1, Result(static_cast<quint16>(value), true));

    auto expResults = QList<Result>() << Result(-1000000, true);

    verifyRegisterResult(modbusRegisters, partialResultMap, expResults);
}

void TestRegisterValueHandler::readConnections()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, false, true)
                                                   << ModbusRegister(40001, Connection::ID_2, false, false);
    QMap<quint16, Result> partialResultMap1;
    addToResultMap(partialResultMap1, 40001, false, 256, true);

    QMap<quint16, Result> partialResultMap2;
    addToResultMap(partialResultMap2, 40001, false, 100, true);

    auto expResults = QList<Result>() << Result(256, true)
                                            << Result(100, true);


    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QSignalSpy spyDataReady(&regHandler, &RegisterValueHandler::registerDataReady);

    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap1, Connection::ID_1);
    regHandler.processPartialResult(partialResultMap2, Connection::ID_2);
    regHandler.finishRead();

    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    QVERIFY(arguments.count() > 0);

    QVariant varResultList = arguments.first();
    QVERIFY((varResultList.canConvert<QList<Result> >()));
    QList<Result> result = varResultList.value<QList<Result> >();

    QCOMPARE(result, expResults);
}

void TestRegisterValueHandler::readFail()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, false, true)
                                                   << ModbusRegister(40001, Connection::ID_2, false, false);

    QMap<quint16, Result> partialResultMap2;
    addToResultMap(partialResultMap2, 40001, false, 100, true);

    auto expResults = QList<Result>() << Result(0, false)
                                            << Result(100, true);


    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QSignalSpy spyDataReady(&regHandler, &RegisterValueHandler::registerDataReady);

    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap2, Connection::ID_2);
    regHandler.finishRead();

    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    QVERIFY(arguments.count() > 0);

    QVariant varResultList = arguments.first();
    QVERIFY((varResultList.canConvert<QList<Result> >()));
    QList<Result> result = varResultList.value<QList<Result> >();

    QCOMPARE(result, expResults);
}

void TestRegisterValueHandler::verifyRegisterResult(QList<ModbusRegister>& regList,
                                                    QMap<quint16, Result> &regData,
                                                    QList<Result> expResults)
{
    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(regList);

    QSignalSpy spyDataReady(&regHandler, &RegisterValueHandler::registerDataReady);

    regHandler.startRead();
    regHandler.processPartialResult(regData, Connection::ID_1);
    regHandler.finishRead();

    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    QVERIFY(arguments.count() > 0);

    QVariant varResultList = arguments.first();
    QVERIFY((varResultList.canConvert<QList<Result> >()));
    QList<Result> result = varResultList.value<QList<Result> >();

    QCOMPARE(result, expResults);
}

void TestRegisterValueHandler::addToResultMap(QMap<quint16, Result> &resultMap,
        quint16 addr,
        bool b32bit,
        qint64 value,
        bool result
        )
{
    resultMap.insert(addr, Result(static_cast<quint16>(value), result));

    if (b32bit)
    {
        resultMap.insert(addr + 1, Result(static_cast<quint32>(value) >> 16, result));
    }
}

QTEST_GUILESS_MAIN(TestRegisterValueHandler)
