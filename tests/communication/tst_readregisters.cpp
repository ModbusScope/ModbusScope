
#include <QtTest/QtTest>

#include "tst_readregisters.h"

#include "readregisters.h"

using ObjectType = ModbusAddress::ObjectType;

void TestReadRegisters::init()
{

}

void TestReadRegisters::cleanup()
{

}

void TestReadRegisters::verifyAndAddErrorResult(ReadRegisters& readRegister, ModbusAddress addr, quint16 cnt)
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
    auto registerList = QList<ModbusAddress>() << 0;

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(readRegister, 0, 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::resetRead_2()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusAddress>() << 0 << 1 << 2 << 3;

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(readRegister, 0, 4);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::resetReadSplit_1()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusAddress>() << 0 << 1 << 3 << 5 << 6 << 8;

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(readRegister, 0, 2);
    verifyAndAddErrorResult(readRegister, 3, 1);
    verifyAndAddErrorResult(readRegister, 5, 2);
    verifyAndAddErrorResult(readRegister, 8, 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::resetReadSplit_2()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusAddress>() << 0 << 3;

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(readRegister, 0, 1);
    verifyAndAddErrorResult(readRegister, 3, 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::resetReadDifferentObjectTypes_1()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusAddress>() << ModbusAddress(0, ObjectType::COIL)
                                               << ModbusAddress(1, ObjectType::COIL)
                                               << ModbusAddress(0, ObjectType::HOLDING_REGISTER)
                                               << ModbusAddress(2, ObjectType::HOLDING_REGISTER);

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(readRegister, ModbusAddress(0, ObjectType::COIL), 2);
    verifyAndAddErrorResult(readRegister, ModbusAddress(0, ObjectType::HOLDING_REGISTER), 1);
    verifyAndAddErrorResult(readRegister, ModbusAddress(2, ObjectType::HOLDING_REGISTER), 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::resetReadDifferentObjectTypes_2()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusAddress>() << ModbusAddress(10000)
                                               << ModbusAddress(10001)
                                               << ModbusAddress(10002)
                                               << ModbusAddress(40000)
                                               << ModbusAddress(40001)
                                               << ModbusAddress(40002)
                                               ;

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(readRegister, ModbusAddress(10000), 1);
    verifyAndAddErrorResult(readRegister, ModbusAddress(10001), 2);
    verifyAndAddErrorResult(readRegister, ModbusAddress(40000), 1);
    verifyAndAddErrorResult(readRegister, ModbusAddress(40001), 2);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::consecutive_1()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusAddress>() << 0 << 1 << 2 << 3 << 4 << 5;

    readRegister.resetRead(registerList, 3);

    verifyAndAddErrorResult(readRegister, 0, 3);
    verifyAndAddErrorResult(readRegister, 3, 3);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::consecutive_2()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusAddress>() << 0 << 1;

    readRegister.resetRead(registerList, 5);

    verifyAndAddErrorResult(readRegister, 0, 2);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::consecutive_3()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusAddress>() << 0 << 1 << 2;

    readRegister.resetRead(registerList, 2);

    verifyAndAddErrorResult(readRegister, 0, 2);
    verifyAndAddErrorResult(readRegister, 2, 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::splitNextToSingleReads_1()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusAddress>() << 0;

    readRegister.resetRead(registerList, 2);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), 0);
    QCOMPARE(readRegister.next().count(), 1);

    readRegister.splitNextToSingleReads();

    verifyAndAddErrorResult(readRegister, 0, 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::splitNextToSingleReads_2()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusAddress>() << 0 << 1 << 2;

    readRegister.resetRead(registerList, 100);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), 0);
    QCOMPARE(readRegister.next().count(), 3);

    readRegister.splitNextToSingleReads();

    verifyAndAddErrorResult(readRegister, 0, 1);
    verifyAndAddErrorResult(readRegister, 1, 1);
    verifyAndAddErrorResult(readRegister, 2, 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::splitNextToSingleReads_3()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusAddress>() << 0 << 1 << 2 << 5 << 6;

    readRegister.resetRead(registerList, 100);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), 0);
    QCOMPARE(readRegister.next().count(), 3);

    readRegister.splitNextToSingleReads();

    verifyAndAddErrorResult(readRegister, 0, 1);
    verifyAndAddErrorResult(readRegister, 1, 1);
    verifyAndAddErrorResult(readRegister, 2, 1);

    verifyAndAddErrorResult(readRegister, 5, 2);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::addAllErrors()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusAddress>() << 0 << 1 << 2 << 5 << 6 << 7;

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
    auto registerList = QList<ModbusAddress>() << 0 << 1 << 2 << 5 << 6 << 8;

    readRegister.resetRead(registerList, 100);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), 0);
    QCOMPARE(readRegister.next().count(), 3);

    readRegister.addSuccess(0, QList<quint16>() << 1000 << 1001 << 1002);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), 5);
    QCOMPARE(readRegister.next().count(), 2);

    readRegister.addSuccess(5, QList<quint16>() << 1005 << 1006);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), 8);
    QCOMPARE(readRegister.next().count(), 1);

    readRegister.addSuccess(8, QList<quint16>() << 1008);

    QVERIFY(!readRegister.hasNext());

    auto resultMap = readRegister.resultMap();

    QCOMPARE(resultMap.size(), registerList.size());

    for(quint16 idx = 0; idx < static_cast<quint16>(registerList.size()); idx++)
    {
        QCOMPARE(resultMap.value(registerList[idx]).value(), registerList[idx].next(1000));
        QVERIFY(resultMap.value(registerList[idx]).isValid());
    }
}

void TestReadRegisters::addSuccessAndErrors()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusAddress>() << 0 << 1 << 5 << 8;

    readRegister.resetRead(registerList, 100);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), 0);
    QCOMPARE(readRegister.next().count(), 2);

    readRegister.addSuccess(0, QList<quint16>() << 1000 << 1001);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), 5);
    QCOMPARE(readRegister.next().count(), 1);

    readRegister.addError();

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), 8);
    QCOMPARE(readRegister.next().count(), 1);

    readRegister.addSuccess(8, QList<quint16>() << 1008);

    QVERIFY(!readRegister.hasNext());

    auto resultMap = readRegister.resultMap();

    QCOMPARE(resultMap.size(), registerList.size());


    QCOMPARE(resultMap.value(0).value(), 1000);
    QVERIFY(resultMap.value(0).isValid());

    QCOMPARE(resultMap.value(1).value(), 1001);
    QVERIFY(resultMap.value(1).isValid());

    QCOMPARE(resultMap.value(5).value(), 0);
    QVERIFY(!resultMap.value(5).isValid());

    QCOMPARE(resultMap.value(8).value(), 1008);
    QVERIFY(resultMap.value(8).isValid());
}

QTEST_GUILESS_MAIN(TestReadRegisters)
