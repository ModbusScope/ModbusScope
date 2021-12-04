
#include <QtTest/QtTest>

#include "testregistervaluehandler.h"

#include "settingsmodel.h"

Q_DECLARE_METATYPE(ModbusResult);


void TestRegisterValueHandler::init()
{
    qRegisterMetaType<ModbusResult>("ModbusResult");
    qRegisterMetaType<QList<ModbusResult> >("QList<ModbusResult>");

    _pSettingsModel = new SettingsModel();

    _pSettingsModel->setInt32LittleEndian(SettingsModel::CONNECTION_ID_0, true);
    _pSettingsModel->setInt32LittleEndian(SettingsModel::CONNECTION_ID_1, true);
}

void TestRegisterValueHandler::cleanup()
{
    delete _pSettingsModel;
}

void TestRegisterValueHandler::addressList_16()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, SettingsModel::CONNECTION_ID_0, false, true);
    auto expRegisterList = QList<uint16_t>() << 40001;

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<quint16> actualRegisterList;
    regHandler.registerAddresList(actualRegisterList, SettingsModel::CONNECTION_ID_0);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressList_16_multiple()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, SettingsModel::CONNECTION_ID_0, false, true)
                                                   << ModbusRegister(40002, SettingsModel::CONNECTION_ID_0, false, true);
    auto expRegisterList = QList<uint16_t>() << 40001 << 40002;

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<quint16> actualRegisterList;
    regHandler.registerAddresList(actualRegisterList, SettingsModel::CONNECTION_ID_0);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressList_32()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, SettingsModel::CONNECTION_ID_0, true, true);
    auto expRegisterList = QList<uint16_t>() << 40001 << 40002;

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<quint16> actualRegisterList;
    regHandler.registerAddresList(actualRegisterList, SettingsModel::CONNECTION_ID_0);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressList_32_multiple()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, SettingsModel::CONNECTION_ID_0, true, true)
                                                   << ModbusRegister(40005, SettingsModel::CONNECTION_ID_0, true, true);
    auto expRegisterList = QList<uint16_t>() << 40001 << 40002 << 40005 << 40006;

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<quint16> actualRegisterList;
    regHandler.registerAddresList(actualRegisterList, SettingsModel::CONNECTION_ID_0);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressListCombinations()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, SettingsModel::CONNECTION_ID_0, false, true)
                                                   << ModbusRegister(40005, SettingsModel::CONNECTION_ID_0, false, true)
                                                   << ModbusRegister(40005, SettingsModel::CONNECTION_ID_0, true, true)
                                                   << ModbusRegister(40008, SettingsModel::CONNECTION_ID_0, true, true);
    auto expRegisterList = QList<uint16_t>() << 40001 << 40005 << 40006 << 40008 << 40009;

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<quint16> actualRegisterList;
    regHandler.registerAddresList(actualRegisterList, SettingsModel::CONNECTION_ID_0);

    QVERIFY(actualRegisterList == expRegisterList);
}

void TestRegisterValueHandler::addressListMultipleConnections()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, SettingsModel::CONNECTION_ID_0, true, true)
                                                   << ModbusRegister(40005, SettingsModel::CONNECTION_ID_1, true, true)
                                                   << ModbusRegister(40010, SettingsModel::CONNECTION_ID_0, true, true);
    auto expRegisterList0 = QList<uint16_t>() << 40001 << 40002 << 40010 << 40011;
    auto expRegisterList1 = QList<uint16_t>() << 40005 << 40006;

    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QList<quint16> actualRegisterList0;
    QList<quint16> actualRegisterList1;
    regHandler.registerAddresList(actualRegisterList0, SettingsModel::CONNECTION_ID_0);
    regHandler.registerAddresList(actualRegisterList1, SettingsModel::CONNECTION_ID_1);

    QVERIFY(actualRegisterList0 == expRegisterList0);
    QVERIFY(actualRegisterList1 == expRegisterList1);
}

void TestRegisterValueHandler::read_16()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, SettingsModel::CONNECTION_ID_0, false, true)
                                                   << ModbusRegister(40002, SettingsModel::CONNECTION_ID_0, false, false);
    QMap<quint16, ModbusResult> partialResultMap;
    addToResultMap(partialResultMap, 40001, false, 256, true);
    addToResultMap(partialResultMap, 40002, false, -100, true);

    auto expResults = QList<ModbusResult>() << ModbusResult(256, true)
                                            << ModbusResult(-100, true);

    verifyRegisterResult(modbusRegisters, partialResultMap, expResults);
}

void TestRegisterValueHandler::read_32()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, SettingsModel::CONNECTION_ID_0, true, true)
                                                   << ModbusRegister(40005, SettingsModel::CONNECTION_ID_0, true, false);
    QMap<quint16, ModbusResult> partialResultMap;
    addToResultMap(partialResultMap, 40001, true, 1000000, true);
    addToResultMap(partialResultMap, 40005, true, -100000, true);

    auto expResults = QList<ModbusResult>() << ModbusResult(1000000, true)
                                            << ModbusResult(-100000, true);

    verifyRegisterResult(modbusRegisters, partialResultMap, expResults);
}

void TestRegisterValueHandler::readBigEndian_32()
{
    _pSettingsModel->setInt32LittleEndian(SettingsModel::CONNECTION_ID_0, false);

    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, SettingsModel::CONNECTION_ID_0, true, true);
    QMap<quint16, ModbusResult> partialResultMap;
    quint32 value = 1000000;

    partialResultMap.insert(40001, ModbusResult(static_cast<quint32>(value) >> 16, true));
    partialResultMap.insert(40001 + 1, ModbusResult(static_cast<quint16>(value), true));

    auto expResults = QList<ModbusResult>() << ModbusResult(1000000, true);

    verifyRegisterResult(modbusRegisters, partialResultMap, expResults);
}

void TestRegisterValueHandler::readBigEndian_s32()
{
    _pSettingsModel->setInt32LittleEndian(SettingsModel::CONNECTION_ID_0, false);

    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, SettingsModel::CONNECTION_ID_0, true, true);
    QMap<quint16, ModbusResult> partialResultMap;
    quint32 value = -1000000;

    partialResultMap.insert(40001, ModbusResult(static_cast<quint32>(value) >> 16, true));
    partialResultMap.insert(40001 + 1, ModbusResult(static_cast<quint16>(value), true));

    auto expResults = QList<ModbusResult>() << ModbusResult(-1000000, true);

    verifyRegisterResult(modbusRegisters, partialResultMap, expResults);
}

void TestRegisterValueHandler::readConnections()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, SettingsModel::CONNECTION_ID_0, false, true)
                                                   << ModbusRegister(40001, SettingsModel::CONNECTION_ID_1, false, false);
    QMap<quint16, ModbusResult> partialResultMap1;
    addToResultMap(partialResultMap1, 40001, false, 256, true);

    QMap<quint16, ModbusResult> partialResultMap2;
    addToResultMap(partialResultMap2, 40001, false, 100, true);

    auto expResults = QList<ModbusResult>() << ModbusResult(256, true)
                                            << ModbusResult(100, true);


    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QSignalSpy spyDataReady(&regHandler, &RegisterValueHandler::registerDataReady);

    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap1, SettingsModel::CONNECTION_ID_0);
    regHandler.processPartialResult(partialResultMap2, SettingsModel::CONNECTION_ID_1);
    regHandler.finishRead();

    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    QVERIFY(arguments.count() > 0);

    QVariant varResultList = arguments.first();
    QVERIFY((varResultList.canConvert<QList<ModbusResult> >()));
    QList<ModbusResult> result = varResultList.value<QList<ModbusResult> >();

    QCOMPARE(result, expResults);
}

void TestRegisterValueHandler::readFail()
{
    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, SettingsModel::CONNECTION_ID_0, false, true)
                                                   << ModbusRegister(40001, SettingsModel::CONNECTION_ID_1, false, false);

    QMap<quint16, ModbusResult> partialResultMap2;
    addToResultMap(partialResultMap2, 40001, false, 100, true);

    auto expResults = QList<ModbusResult>() << ModbusResult(0, false)
                                            << ModbusResult(100, true);


    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(modbusRegisters);

    QSignalSpy spyDataReady(&regHandler, &RegisterValueHandler::registerDataReady);

    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap2, SettingsModel::CONNECTION_ID_1);
    regHandler.finishRead();

    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    QVERIFY(arguments.count() > 0);

    QVariant varResultList = arguments.first();
    QVERIFY((varResultList.canConvert<QList<ModbusResult> >()));
    QList<ModbusResult> result = varResultList.value<QList<ModbusResult> >();

    QCOMPARE(result, expResults);
}

void TestRegisterValueHandler::verifyRegisterResult(QList<ModbusRegister>& regList,
                                                    QMap<quint16, ModbusResult> &regData,
                                                    QList<ModbusResult> expResults)
{
    RegisterValueHandler regHandler(_pSettingsModel);
    regHandler.setRegisters(regList);

    QSignalSpy spyDataReady(&regHandler, &RegisterValueHandler::registerDataReady);

    regHandler.startRead();
    regHandler.processPartialResult(regData, SettingsModel::CONNECTION_ID_0);
    regHandler.finishRead();

    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    QVERIFY(arguments.count() > 0);

    QVariant varResultList = arguments.first();
    QVERIFY((varResultList.canConvert<QList<ModbusResult> >()));
    QList<ModbusResult> result = varResultList.value<QList<ModbusResult> >();

    QCOMPARE(result, expResults);
}

void TestRegisterValueHandler::addToResultMap(QMap<quint16, ModbusResult> &resultMap,
        quint16 addr,
        bool b32bit,
        qint64 value,
        bool result
        )
{
    resultMap.insert(addr, ModbusResult(static_cast<quint16>(value), result));

    if (b32bit)
    {
        resultMap.insert(addr + 1, ModbusResult(static_cast<quint32>(value) >> 16, result));
    }
}

QTEST_GUILESS_MAIN(TestRegisterValueHandler)
