
#include "tst_registervaluehandler.h"

#include "communication/registervaluehandler.h"
#include "models/settingsmodel.h"

#include <QSignalSpy>
#include <QTest>

Q_DECLARE_METATYPE(ResultDouble);

using Type = ModbusDataType::Type;
using State = ResultState::State;

void TestRegisterValueHandler::init()
{
    qRegisterMetaType<ResultDouble>("ResultDouble");
    qRegisterMetaType<ResultDoubleList >("ResultDoubleList");

    _pSettingsModel = new SettingsModel();

    _pSettingsModel->deviceSettings(Device::cFirstDeviceId)->setInt32LittleEndian(true);

    deviceId_t devId = Device::cFirstDeviceId + 1;
    _pSettingsModel->addDevice(devId);
    _pSettingsModel->deviceSettings(devId)->setConnectionId(ConnectionTypes::ID_2);
    _pSettingsModel->deviceSettings(devId)->setInt32LittleEndian(true);
    _pSettingsModel->deviceSettings(devId)->setSlaveId(2);
}

void TestRegisterValueHandler::cleanup()
{
    delete _pSettingsModel;
}

void TestRegisterValueHandler::addressList_16()
{
    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::UNSIGNED_16);
    auto expRegisterList = QList<ModbusDataUnit>() << ModbusDataUnit(40001);

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<ModbusDataUnit> actualRegisterList;
    regHandler.registerAddresListForConnection(actualRegisterList, ConnectionTypes::ID_1);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressList_16_multiple()
{
    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::UNSIGNED_16)
                           << ModbusRegister(ModbusAddress(40002), Device::cFirstDeviceId, Type::UNSIGNED_16);
    auto expRegisterList = QList<ModbusDataUnit>() << ModbusDataUnit(40001) << ModbusDataUnit(40002);

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<ModbusDataUnit> actualRegisterList;
    regHandler.registerAddresListForConnection(actualRegisterList, ConnectionTypes::ID_1);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressList_32()
{
    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::UNSIGNED_32);
    auto expRegisterList = QList<ModbusDataUnit>() << ModbusDataUnit(40001) << ModbusDataUnit(40002);

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<ModbusDataUnit> actualRegisterList;
    regHandler.registerAddresListForConnection(actualRegisterList, ConnectionTypes::ID_1);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressList_32_multiple()
{
    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::UNSIGNED_32)
                           << ModbusRegister(ModbusAddress(40005), Device::cFirstDeviceId, Type::UNSIGNED_32);
    auto expRegisterList = QList<ModbusDataUnit>() << ModbusDataUnit(40001) << ModbusDataUnit(40002)
                                                   << ModbusDataUnit(40005) << ModbusDataUnit(40006);

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<ModbusDataUnit> actualRegisterList;
    regHandler.registerAddresListForConnection(actualRegisterList, ConnectionTypes::ID_1);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressList_float32()
{
    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::FLOAT_32);
    auto expRegisterList = QList<ModbusDataUnit>() << ModbusDataUnit(40001) << ModbusDataUnit(40002);

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<ModbusDataUnit> actualRegisterList;
    regHandler.registerAddresListForConnection(actualRegisterList, ConnectionTypes::ID_1);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressList_float32_multiple()
{
    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::FLOAT_32)
                           << ModbusRegister(ModbusAddress(40005), Device::cFirstDeviceId, Type::FLOAT_32);
    auto expRegisterList = QList<ModbusDataUnit>() << ModbusDataUnit(40001) << ModbusDataUnit(40002)
                                                   << ModbusDataUnit(40005) << ModbusDataUnit(40006);

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<ModbusDataUnit> actualRegisterList;
    regHandler.registerAddresListForConnection(actualRegisterList, ConnectionTypes::ID_1);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressListCombinations()
{
    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::UNSIGNED_16)
                           << ModbusRegister(ModbusAddress(40005), Device::cFirstDeviceId, Type::UNSIGNED_16)
                           << ModbusRegister(ModbusAddress(40005), Device::cFirstDeviceId, Type::UNSIGNED_32)
                           << ModbusRegister(ModbusAddress(40008), Device::cFirstDeviceId, Type::UNSIGNED_32);
    auto expRegisterList = QList<ModbusDataUnit>()
                           << ModbusDataUnit(40001) << ModbusDataUnit(40005) << ModbusDataUnit(40006)
                           << ModbusDataUnit(40008) << ModbusDataUnit(40009);

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<ModbusDataUnit> actualRegisterList;
    regHandler.registerAddresListForConnection(actualRegisterList, ConnectionTypes::ID_1);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressListMultipleConnections()
{
    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::UNSIGNED_32)
                           << ModbusRegister(ModbusAddress(40005), Device::cFirstDeviceId + 1, Type::UNSIGNED_32)
                           << ModbusRegister(ModbusAddress(40010), Device::cFirstDeviceId, Type::UNSIGNED_32);
    auto expRegisterList0 = QList<ModbusDataUnit>() << ModbusDataUnit(40001) << ModbusDataUnit(40002)
                                                    << ModbusDataUnit(40010) << ModbusDataUnit(40011);
    auto expRegisterList1 = QList<ModbusDataUnit>()
                            << ModbusDataUnit(40005 - 40001, ModbusDataUnit::ObjectType::HOLDING_REGISTER, 2)
                            << ModbusDataUnit(40006 - 40001, ModbusDataUnit::ObjectType::HOLDING_REGISTER, 2);

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<ModbusDataUnit> actualRegisterList0;
    QList<ModbusDataUnit> actualRegisterList1;
    regHandler.registerAddresListForConnection(actualRegisterList0, ConnectionTypes::ID_1);
    regHandler.registerAddresListForConnection(actualRegisterList1, ConnectionTypes::ID_2);

    QVERIFY(actualRegisterList0 == expRegisterList0);
    QVERIFY(actualRegisterList1 == expRegisterList1);
}

void TestRegisterValueHandler::addressListMixedObjects()
{
    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(0), Device::cFirstDeviceId, Type::UNSIGNED_16)
                           << ModbusRegister(ModbusAddress(30002), Device::cFirstDeviceId, Type::UNSIGNED_16)
                           << ModbusRegister(ModbusAddress(40002), Device::cFirstDeviceId, Type::UNSIGNED_16);
    auto expRegisterList = QList<ModbusDataUnit>()
                           << ModbusDataUnit(0) << ModbusDataUnit(30002) << ModbusDataUnit(40002);

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<ModbusDataUnit> actualRegisterList;
    regHandler.registerAddresListForConnection(actualRegisterList, ConnectionTypes::ID_1);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressListSameRegisterDifferentType()
{
    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(0), Device::cFirstDeviceId, Type::UNSIGNED_16)
                           << ModbusRegister(ModbusAddress(0), Device::cFirstDeviceId, Type::SIGNED_16);
    auto expRegisterList = QList<ModbusDataUnit>() << ModbusDataUnit(0);

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<ModbusDataUnit> actualRegisterList;
    regHandler.registerAddresListForConnection(actualRegisterList, ConnectionTypes::ID_1);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::read_16()
{
    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::UNSIGNED_16)
                           << ModbusRegister(ModbusAddress(40002), Device::cFirstDeviceId, Type::SIGNED_16);
    ModbusResultMap partialResultMap;
    addToResultMap(partialResultMap, 40001, false, 256, State::SUCCESS);
    addToResultMap(partialResultMap, 40002, false, -100, State::SUCCESS);

    auto expResults = ResultDoubleList() << ResultDouble(256, State::SUCCESS)
                                         << ResultDouble(-100, State::SUCCESS);

    verifyRegisterResult(modbusRegisters, partialResultMap, expResults);
}

void TestRegisterValueHandler::read_32()
{
    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::UNSIGNED_32)
                           << ModbusRegister(ModbusAddress(40005), Device::cFirstDeviceId, Type::SIGNED_32);
    ModbusResultMap partialResultMap;
    addToResultMap(partialResultMap, 40001, true, 1000000, State::SUCCESS);
    addToResultMap(partialResultMap, 40005, true, -100000, State::SUCCESS);

    auto expResults = ResultDoubleList() << ResultDouble(1000000, State::SUCCESS)
                                            << ResultDouble(-100000, State::SUCCESS);

    verifyRegisterResult(modbusRegisters, partialResultMap, expResults);
}

void TestRegisterValueHandler::readBigEndian_32()
{
    _pSettingsModel->deviceSettings(Device::cFirstDeviceId)->setInt32LittleEndian(false);

    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::UNSIGNED_32);
    ModbusResultMap partialResultMap;
    quint32 value = 1000000;

    partialResultMap.insert(ModbusDataUnit(40001), Result<quint16>(static_cast<quint32>(value) >> 16, State::SUCCESS));
    partialResultMap.insert(ModbusDataUnit(40001 + 1), Result<quint16>(static_cast<quint16>(value), State::SUCCESS));

    auto expResults = ResultDoubleList() << ResultDouble(1000000, State::SUCCESS);

    verifyRegisterResult(modbusRegisters, partialResultMap, expResults);
}

void TestRegisterValueHandler::readBigEndian_s32()
{
    _pSettingsModel->deviceSettings(Device::cFirstDeviceId)->setInt32LittleEndian(false);

    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::SIGNED_32);
    ModbusResultMap partialResultMap;
    quint32 value = -1000000;

    partialResultMap.insert(ModbusDataUnit(40001), Result<quint16>(static_cast<quint32>(value) >> 16, State::SUCCESS));
    partialResultMap.insert(ModbusDataUnit(40001 + 1), Result<quint16>(static_cast<quint16>(value), State::SUCCESS));

    auto expResults = ResultDoubleList() << ResultDouble(-1000000, State::SUCCESS);

    verifyRegisterResult(modbusRegisters, partialResultMap, expResults);
}

void TestRegisterValueHandler::readSameRegisterDifferentType()
{
    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::UNSIGNED_32)
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::SIGNED_32);

    ModbusResultMap partialResultMap;
    addToResultMap(partialResultMap, 40001, true, 255, State::SUCCESS);

    auto expResults = ResultDoubleList() << ResultDouble(255, State::SUCCESS)
                                         << ResultDouble(255, State::SUCCESS);

    verifyRegisterResult(modbusRegisters, partialResultMap, expResults);
}

void TestRegisterValueHandler::readConnections()
{
    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::UNSIGNED_16)
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId + 1, Type::SIGNED_16);
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
    regHandler.processPartialResult(partialResultMap1, ConnectionTypes::ID_1);
    regHandler.processPartialResult(partialResultMap2, ConnectionTypes::ID_2);
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
    auto modbusRegisters = QList<ModbusRegister>()
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId, Type::UNSIGNED_16)
                           << ModbusRegister(ModbusAddress(40001), Device::cFirstDeviceId + 1, Type::SIGNED_16);

    ModbusResultMap partialResultMap2;
    addToResultMap(partialResultMap2, 40001, false, 100, State::SUCCESS);

    auto expResults = ResultDoubleList() << ResultDouble(0, State::INVALID)
                                            << ResultDouble(100, State::SUCCESS);


    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QSignalSpy spyDataReady(&regHandler, &RegisterValueHandler::registerDataReady);

    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap2, ConnectionTypes::ID_2);
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
    regHandler.processPartialResult(regData, ConnectionTypes::ID_1);
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
    resultMap.insert(ModbusDataUnit(addr), Result<quint16>(static_cast<quint16>(value), resultState));

    if (b32bit)
    {
        resultMap.insert(ModbusDataUnit(addr + 1), Result<quint16>(static_cast<quint32>(value) >> 16, resultState));
    }
}

QTEST_GUILESS_MAIN(TestRegisterValueHandler)
