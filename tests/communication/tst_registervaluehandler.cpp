
#include <QtTest/QtTest>

#include "registervaluehandler.h"
#include "tst_registervaluehandler.h"

#include "settingsmodel.h"

Q_DECLARE_METATYPE(ResultDouble);

using Type = ModbusDataType::Type;
using State = ResultState::State;

void TestRegisterValueHandler::init()
{
    qRegisterMetaType<ResultDouble>("ResultDouble");
    qRegisterMetaType<ResultDoubleList >("ResultDoubleList");

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
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, Type::UNSIGNED_16);
    auto expRegisterList = QList<ModbusAddress>() << 40001;

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<ModbusAddress> actualRegisterList;
    regHandler.registerAddresList(actualRegisterList, Connection::ID_1);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressList_16_multiple()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, Type::UNSIGNED_16)
                                                   << ModbusRegister(40002, Connection::ID_1, Type::UNSIGNED_16);
    auto expRegisterList = QList<ModbusAddress>() << 40001 << 40002;

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<ModbusAddress> actualRegisterList;
    regHandler.registerAddresList(actualRegisterList, Connection::ID_1);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressList_32()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, Type::UNSIGNED_32);
    auto expRegisterList = QList<ModbusAddress>() << 40001 << 40002;

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<ModbusAddress> actualRegisterList;
    regHandler.registerAddresList(actualRegisterList, Connection::ID_1);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressList_32_multiple()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, Type::UNSIGNED_32)
                                                   << ModbusRegister(40005, Connection::ID_1, Type::UNSIGNED_32);
    auto expRegisterList = QList<ModbusAddress>() << 40001 << 40002 << 40005 << 40006;

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<ModbusAddress> actualRegisterList;
    regHandler.registerAddresList(actualRegisterList, Connection::ID_1);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressList_float32()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, Type::FLOAT_32);
    auto expRegisterList = QList<ModbusAddress>() << 40001 << 40002;

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<ModbusAddress> actualRegisterList;
    regHandler.registerAddresList(actualRegisterList, Connection::ID_1);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressList_float32_multiple()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, Type::FLOAT_32)
                                                   << ModbusRegister(40005, Connection::ID_1, Type::FLOAT_32);
    auto expRegisterList = QList<ModbusAddress>() << 40001 << 40002 << 40005 << 40006;

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<ModbusAddress> actualRegisterList;
    regHandler.registerAddresList(actualRegisterList, Connection::ID_1);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressListCombinations()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, Type::UNSIGNED_16)
                                                   << ModbusRegister(40005, Connection::ID_1, Type::UNSIGNED_16)
                                                   << ModbusRegister(40005, Connection::ID_1, Type::UNSIGNED_32)
                                                   << ModbusRegister(40008, Connection::ID_1, Type::UNSIGNED_32);
    auto expRegisterList = QList<ModbusAddress>() << 40001 << 40005 << 40006 << 40008 << 40009;

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<ModbusAddress> actualRegisterList;
    regHandler.registerAddresList(actualRegisterList, Connection::ID_1);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressListMultipleConnections()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, Type::UNSIGNED_32)
                                                   << ModbusRegister(40005, Connection::ID_2, Type::UNSIGNED_32)
                                                   << ModbusRegister(40010, Connection::ID_1, Type::UNSIGNED_32);
    auto expRegisterList0 = QList<ModbusAddress>() << 40001 << 40002 << 40010 << 40011;
    auto expRegisterList1 = QList<ModbusAddress>() << 40005 << 40006;

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<ModbusAddress> actualRegisterList0;
    QList<ModbusAddress> actualRegisterList1;
    regHandler.registerAddresList(actualRegisterList0, Connection::ID_1);
    regHandler.registerAddresList(actualRegisterList1, Connection::ID_2);

    QVERIFY(actualRegisterList0 == expRegisterList0);
    QVERIFY(actualRegisterList1 == expRegisterList1);
}

void TestRegisterValueHandler::addressListMixedObjects()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(0, Connection::ID_1, Type::UNSIGNED_16)
                                                   << ModbusRegister(30002, Connection::ID_1, Type::UNSIGNED_16)
                                                   << ModbusRegister(40002, Connection::ID_1, Type::UNSIGNED_16)   ;
    auto expRegisterList = QList<ModbusAddress>() << 0 << 30002 << 40002;

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<ModbusAddress> actualRegisterList;
    regHandler.registerAddresList(actualRegisterList, Connection::ID_1);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::read_16()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, Type::UNSIGNED_16)
                                                   << ModbusRegister(40002, Connection::ID_1, Type::SIGNED_16);
    ModbusResultMap partialResultMap;
    addToResultMap(partialResultMap, 40001, false, 256, State::SUCCESS);
    addToResultMap(partialResultMap, 40002, false, -100, State::SUCCESS);

    auto expResults = ResultDoubleList() << ResultDouble(256, State::SUCCESS)
                                         << ResultDouble(-100, State::SUCCESS);

    verifyRegisterResult(modbusRegisters, partialResultMap, expResults);
}

void TestRegisterValueHandler::read_32()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, Type::UNSIGNED_32)
                                                   << ModbusRegister(40005, Connection::ID_1, Type::SIGNED_32);
    ModbusResultMap partialResultMap;
    addToResultMap(partialResultMap, 40001, true, 1000000, State::SUCCESS);
    addToResultMap(partialResultMap, 40005, true, -100000, State::SUCCESS);

    auto expResults = ResultDoubleList() << ResultDouble(1000000, State::SUCCESS)
                                            << ResultDouble(-100000, State::SUCCESS);

    verifyRegisterResult(modbusRegisters, partialResultMap, expResults);
}

void TestRegisterValueHandler::readBigEndian_32()
{
    _pSettingsModel->setInt32LittleEndian(Connection::ID_1, false);

    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, Type::UNSIGNED_32);
    ModbusResultMap partialResultMap;
    quint32 value = 1000000;

    partialResultMap.insert(40001, Result<quint16>(static_cast<quint32>(value) >> 16, State::SUCCESS));
    partialResultMap.insert(40001 + 1, Result<quint16>(static_cast<quint16>(value), State::SUCCESS));

    auto expResults = ResultDoubleList() << ResultDouble(1000000, State::SUCCESS);

    verifyRegisterResult(modbusRegisters, partialResultMap, expResults);
}

void TestRegisterValueHandler::readBigEndian_s32()
{
    _pSettingsModel->setInt32LittleEndian(Connection::ID_1, false);

    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, Type::SIGNED_32);
    ModbusResultMap partialResultMap;
    quint32 value = -1000000;

    partialResultMap.insert(40001, Result<quint16>(static_cast<quint32>(value) >> 16, State::SUCCESS));
    partialResultMap.insert(40001 + 1, Result<quint16>(static_cast<quint16>(value), State::SUCCESS));

    auto expResults = ResultDoubleList() << ResultDouble(-1000000, State::SUCCESS);

    verifyRegisterResult(modbusRegisters, partialResultMap, expResults);
}

void TestRegisterValueHandler::readConnections()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, Type::UNSIGNED_16)
                                                   << ModbusRegister(40001, Connection::ID_2, Type::SIGNED_16);
    ModbusResultMap partialResultMap1;
    addToResultMap(partialResultMap1, 40001, false, 256, State::SUCCESS);

    ModbusResultMap partialResultMap2;
    addToResultMap(partialResultMap2, 40001, false, 100, State::SUCCESS);

    auto expResults = ResultDoubleList() << ResultDouble(256, State::SUCCESS)
                                            << ResultDouble(100, State::SUCCESS);


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
    QVERIFY(varResultList.canConvert<ResultDoubleList >());
    ResultDoubleList result = varResultList.value<ResultDoubleList >();

    QCOMPARE(result, expResults);
}

void TestRegisterValueHandler::readFail()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, Type::UNSIGNED_16)
                                                   << ModbusRegister(40001, Connection::ID_2, Type::SIGNED_16);

    ModbusResultMap partialResultMap2;
    addToResultMap(partialResultMap2, 40001, false, 100, State::SUCCESS);

    auto expResults = ResultDoubleList() << ResultDouble(0, State::INVALID)
                                            << ResultDouble(100, State::SUCCESS);


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
    QVERIFY(varResultList.canConvert<ResultDoubleList >());
    ResultDoubleList result = varResultList.value<ResultDoubleList >();

    QCOMPARE(result, expResults);
}

void TestRegisterValueHandler::verifyRegisterResult(QList<ModbusRegister>& regList,
                                                    ModbusResultMap &regData,
                                                    ResultDoubleList expResults)
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
    QVERIFY(varResultList.canConvert<ResultDoubleList >());
    ResultDoubleList result = varResultList.value<ResultDoubleList >();

    QCOMPARE(result, expResults);
}

void TestRegisterValueHandler::addToResultMap(ModbusResultMap &resultMap,
        quint32 addr,
        bool b32bit,
        qint64 value,
        State resultState
        )
{
    resultMap.insert(addr, Result<quint16>(static_cast<quint16>(value), resultState));

    if (b32bit)
    {
        resultMap.insert(addr + 1, Result<quint16>(static_cast<quint32>(value) >> 16, resultState));
    }
}

QTEST_GUILESS_MAIN(TestRegisterValueHandler)
