
#include <QTest>

#include "tst_readregisters.h"

#include "communication/readregisters.h"

using ObjectType = ModbusDataUnit::ObjectType;

void TestReadRegisters::init()
{

}

void TestReadRegisters::cleanup()
{

}

void TestReadRegisters::verifyAndAddErrorResult(ReadRegisters& readRegister, ModbusDataUnit addr, quint16 cnt)
{
    QVERIFY(readRegister.hasNext());

    auto object = readRegister.next();
    QCOMPARE(object.address(), addr);
    QCOMPARE(object.count(), cnt);

    readRegister.addError();
}

void TestReadRegisters::resetRead_1()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusDataUnit>() << ModbusDataUnit(0);

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(readRegister, ModbusDataUnit(0), 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::resetRead_2()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusDataUnit>()
                        << ModbusDataUnit(0) << ModbusDataUnit(1) << ModbusDataUnit(2) << ModbusDataUnit(3);

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(readRegister, ModbusDataUnit(0), 4);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::resetReadSplit_1()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusDataUnit>() << ModbusDataUnit(0) << ModbusDataUnit(1) << ModbusDataUnit(3)
                                                << ModbusDataUnit(5) << ModbusDataUnit(6) << ModbusDataUnit(8);

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(readRegister, ModbusDataUnit(0), 2);
    verifyAndAddErrorResult(readRegister, ModbusDataUnit(3), 1);
    verifyAndAddErrorResult(readRegister, ModbusDataUnit(5), 2);
    verifyAndAddErrorResult(readRegister, ModbusDataUnit(8), 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::resetReadSplit_2()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusDataUnit>() << ModbusDataUnit(0) << ModbusDataUnit(3);

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(readRegister, ModbusDataUnit(0), 1);
    verifyAndAddErrorResult(readRegister, ModbusDataUnit(3), 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::resetReadDifferentObjectTypes_1()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusDataUnit>()
                        << ModbusDataUnit(0, ObjectType::COIL) << ModbusDataUnit(1, ObjectType::COIL)
                        << ModbusDataUnit(0, ObjectType::HOLDING_REGISTER)
                        << ModbusDataUnit(2, ObjectType::HOLDING_REGISTER);

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(readRegister, ModbusDataUnit(0, ObjectType::COIL), 2);
    verifyAndAddErrorResult(readRegister, ModbusDataUnit(0, ObjectType::HOLDING_REGISTER), 1);
    verifyAndAddErrorResult(readRegister, ModbusDataUnit(2, ObjectType::HOLDING_REGISTER), 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::resetReadDifferentObjectTypes_2()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusDataUnit>()
                        << ModbusDataUnit(10000) << ModbusDataUnit(10001) << ModbusDataUnit(10002)
                        << ModbusDataUnit(40000) << ModbusDataUnit(40001) << ModbusDataUnit(40002);

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(readRegister, ModbusDataUnit(10000), 1);
    verifyAndAddErrorResult(readRegister, ModbusDataUnit(10001), 2);
    verifyAndAddErrorResult(readRegister, ModbusDataUnit(40000), 1);
    verifyAndAddErrorResult(readRegister, ModbusDataUnit(40001), 2);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::consecutive_1()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusDataUnit>() << ModbusDataUnit(0) << ModbusDataUnit(1) << ModbusDataUnit(2)
                                                << ModbusDataUnit(3) << ModbusDataUnit(4) << ModbusDataUnit(5);

    readRegister.resetRead(registerList, 3);

    verifyAndAddErrorResult(readRegister, ModbusDataUnit(0), 3);
    verifyAndAddErrorResult(readRegister, ModbusDataUnit(3), 3);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::consecutive_2()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusDataUnit>() << ModbusDataUnit(0) << ModbusDataUnit(1);

    readRegister.resetRead(registerList, 5);

    verifyAndAddErrorResult(readRegister, ModbusDataUnit(0), 2);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::consecutive_3()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusDataUnit>() << ModbusDataUnit(0) << ModbusDataUnit(1) << ModbusDataUnit(2);

    readRegister.resetRead(registerList, 2);

    verifyAndAddErrorResult(readRegister, ModbusDataUnit(0), 2);
    verifyAndAddErrorResult(readRegister, ModbusDataUnit(2), 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::splitNextToSingleReads_1()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusDataUnit>() << ModbusDataUnit(0);

    readRegister.resetRead(registerList, 2);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), ModbusDataUnit(0));
    QCOMPARE(readRegister.next().count(), 1);

    readRegister.splitNextToSingleReads();

    verifyAndAddErrorResult(readRegister, ModbusDataUnit(0), 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::splitNextToSingleReads_2()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusDataUnit>() << ModbusDataUnit(0) << ModbusDataUnit(1) << ModbusDataUnit(2);

    readRegister.resetRead(registerList, 100);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), ModbusDataUnit(0));
    QCOMPARE(readRegister.next().count(), 3);

    readRegister.splitNextToSingleReads();

    verifyAndAddErrorResult(readRegister, ModbusDataUnit(0), 1);
    verifyAndAddErrorResult(readRegister, ModbusDataUnit(1), 1);
    verifyAndAddErrorResult(readRegister, ModbusDataUnit(2), 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::splitNextToSingleReads_3()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusDataUnit>() << ModbusDataUnit(0) << ModbusDataUnit(1) << ModbusDataUnit(2)
                                                << ModbusDataUnit(5) << ModbusDataUnit(6);

    readRegister.resetRead(registerList, 100);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), ModbusDataUnit(0));
    QCOMPARE(readRegister.next().count(), 3);

    readRegister.splitNextToSingleReads();

    verifyAndAddErrorResult(readRegister, ModbusDataUnit(0), 1);
    verifyAndAddErrorResult(readRegister, ModbusDataUnit(1), 1);
    verifyAndAddErrorResult(readRegister, ModbusDataUnit(2), 1);

    verifyAndAddErrorResult(readRegister, ModbusDataUnit(5), 2);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::addAllErrors()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusDataUnit>() << ModbusDataUnit(0) << ModbusDataUnit(1) << ModbusDataUnit(2)
                                                << ModbusDataUnit(5) << ModbusDataUnit(6) << ModbusDataUnit(7);

    readRegister.resetRead(registerList, 100);

    readRegister.addAllErrors();

    QVERIFY(!readRegister.hasNext());

    auto resultMap = readRegister.resultMap();

    QCOMPARE(resultMap.size(), registerList.size());

    for(quint16 idx = 0; idx < static_cast<quint16>(registerList.size()); idx++)
    {
        QCOMPARE(resultMap.value(registerList[idx]).value(), 0);
        QVERIFY(!resultMap.value(registerList[idx]).isValid());
    }
}

void TestReadRegisters::addSuccess()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusDataUnit>() << ModbusDataUnit(0) << ModbusDataUnit(1) << ModbusDataUnit(2)
                                                << ModbusDataUnit(5) << ModbusDataUnit(6) << ModbusDataUnit(8);

    readRegister.resetRead(registerList, 100);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), ModbusDataUnit(0));
    QCOMPARE(readRegister.next().count(), 3);

    readRegister.addSuccess(ModbusDataUnit(0), QList<quint16>() << 1000 << 1001 << 1002);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), ModbusDataUnit(5));
    QCOMPARE(readRegister.next().count(), 2);

    readRegister.addSuccess(ModbusDataUnit(5), QList<quint16>() << 1005 << 1006);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), ModbusDataUnit(8));
    QCOMPARE(readRegister.next().count(), 1);

    readRegister.addSuccess(ModbusDataUnit(8), QList<quint16>() << 1008);

    QVERIFY(!readRegister.hasNext());

    auto resultMap = readRegister.resultMap();

    QCOMPARE(resultMap.size(), registerList.size());

    for(quint16 idx = 0; idx < static_cast<quint16>(registerList.size()); idx++)
    {
        QCOMPARE(resultMap.value(registerList[idx]).value(), registerList[idx].protocolAddress() + 1000);
        QVERIFY(resultMap.value(registerList[idx]).isValid());
    }
}

void TestReadRegisters::addSuccessAndErrors()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusDataUnit>()
                        << ModbusDataUnit(0) << ModbusDataUnit(1) << ModbusDataUnit(5) << ModbusDataUnit(8);

    readRegister.resetRead(registerList, 100);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), ModbusDataUnit(0));
    QCOMPARE(readRegister.next().count(), 2);

    readRegister.addSuccess(ModbusDataUnit(0), QList<quint16>() << 1000 << 1001);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), ModbusDataUnit(5));
    QCOMPARE(readRegister.next().count(), 1);

    readRegister.addError();

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), ModbusDataUnit(8));
    QCOMPARE(readRegister.next().count(), 1);

    readRegister.addSuccess(ModbusDataUnit(8), QList<quint16>() << 1008);

    QVERIFY(!readRegister.hasNext());

    auto resultMap = readRegister.resultMap();

    QCOMPARE(resultMap.size(), registerList.size());

    QCOMPARE(resultMap.value(ModbusDataUnit(0)).value(), 1000);
    QVERIFY(resultMap.value(ModbusDataUnit(0)).isValid());

    QCOMPARE(resultMap.value(ModbusDataUnit(1)).value(), 1001);
    QVERIFY(resultMap.value(ModbusDataUnit(1)).isValid());

    QCOMPARE(resultMap.value(ModbusDataUnit(5)).value(), 0);
    QVERIFY(!resultMap.value(ModbusDataUnit(5)).isValid());

    QCOMPARE(resultMap.value(ModbusDataUnit(8)).value(), 1008);
    QVERIFY(resultMap.value(ModbusDataUnit(8)).isValid());
}

QTEST_GUILESS_MAIN(TestReadRegisters)
