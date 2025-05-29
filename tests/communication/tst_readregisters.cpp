
#include <QTest>

#include "tst_readregisters.h"

#include "communication/readregisters.h"

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
    auto registerList = QList<ModbusAddress>() << ModbusAddress(0);

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(readRegister, ModbusAddress(0), 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::resetRead_2()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusAddress>() << ModbusAddress(0) << ModbusAddress(1) << ModbusAddress(2) << ModbusAddress(3);

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(readRegister, ModbusAddress(0), 4);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::resetReadSplit_1()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusAddress>() << ModbusAddress(0) << ModbusAddress(1) << ModbusAddress(3) << ModbusAddress(5) << ModbusAddress(6) << ModbusAddress(8);

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(readRegister, ModbusAddress(0), 2);
    verifyAndAddErrorResult(readRegister, ModbusAddress(3), 1);
    verifyAndAddErrorResult(readRegister, ModbusAddress(5), 2);
    verifyAndAddErrorResult(readRegister, ModbusAddress(8), 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::resetReadSplit_2()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusAddress>() << ModbusAddress(0) << ModbusAddress(3);

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(readRegister, ModbusAddress(0), 1);
    verifyAndAddErrorResult(readRegister, ModbusAddress(3), 1);

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
    auto registerList = QList<ModbusAddress>() << ModbusAddress(0) << ModbusAddress(1) << ModbusAddress(2) << ModbusAddress(3) << ModbusAddress(4) << ModbusAddress(5);

    readRegister.resetRead(registerList, 3);

    verifyAndAddErrorResult(readRegister, ModbusAddress(0), 3);
    verifyAndAddErrorResult(readRegister, ModbusAddress(3), 3);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::consecutive_2()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusAddress>() << ModbusAddress(0) << ModbusAddress(1);

    readRegister.resetRead(registerList, 5);

    verifyAndAddErrorResult(readRegister, ModbusAddress(0), 2);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::consecutive_3()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusAddress>() << ModbusAddress(0) << ModbusAddress(1) << ModbusAddress(2);

    readRegister.resetRead(registerList, 2);

    verifyAndAddErrorResult(readRegister, ModbusAddress(0), 2);
    verifyAndAddErrorResult(readRegister, ModbusAddress(2), 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::splitNextToSingleReads_1()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusAddress>() << ModbusAddress(0);

    readRegister.resetRead(registerList, 2);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), ModbusAddress(0));
    QCOMPARE(readRegister.next().count(), 1);

    readRegister.splitNextToSingleReads();

    verifyAndAddErrorResult(readRegister, ModbusAddress(0), 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::splitNextToSingleReads_2()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusAddress>() << ModbusAddress(0) << ModbusAddress(1) << ModbusAddress(2);

    readRegister.resetRead(registerList, 100);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), ModbusAddress(0));
    QCOMPARE(readRegister.next().count(), 3);

    readRegister.splitNextToSingleReads();

    verifyAndAddErrorResult(readRegister, ModbusAddress(0), 1);
    verifyAndAddErrorResult(readRegister, ModbusAddress(1), 1);
    verifyAndAddErrorResult(readRegister, ModbusAddress(2), 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::splitNextToSingleReads_3()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusAddress>() << ModbusAddress(0) << ModbusAddress(1) << ModbusAddress(2) << ModbusAddress(5) << ModbusAddress(6);

    readRegister.resetRead(registerList, 100);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), ModbusAddress(0));
    QCOMPARE(readRegister.next().count(), 3);

    readRegister.splitNextToSingleReads();

    verifyAndAddErrorResult(readRegister, ModbusAddress(0), 1);
    verifyAndAddErrorResult(readRegister, ModbusAddress(1), 1);
    verifyAndAddErrorResult(readRegister, ModbusAddress(2), 1);

    verifyAndAddErrorResult(readRegister, ModbusAddress(5), 2);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::addAllErrors()
{
    ReadRegisters readRegister;
    auto registerList = QList<ModbusAddress>() << ModbusAddress(0) << ModbusAddress(1) << ModbusAddress(2) << ModbusAddress(5) << ModbusAddress(6) << ModbusAddress(7);

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
    auto registerList = QList<ModbusAddress>() << ModbusAddress(0) << ModbusAddress(1) << ModbusAddress(2) << ModbusAddress(5) << ModbusAddress(6) << ModbusAddress(8);

    readRegister.resetRead(registerList, 100);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), ModbusAddress(0));
    QCOMPARE(readRegister.next().count(), 3);

    readRegister.addSuccess(ModbusAddress(0), QList<quint16>() << 1000 << 1001 << 1002);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), ModbusAddress(5));
    QCOMPARE(readRegister.next().count(), 2);

    readRegister.addSuccess(ModbusAddress(5), QList<quint16>() << 1005 << 1006);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), ModbusAddress(8));
    QCOMPARE(readRegister.next().count(), 1);

    readRegister.addSuccess(ModbusAddress(8), QList<quint16>() << 1008);

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
    auto registerList = QList<ModbusAddress>() << ModbusAddress(0) << ModbusAddress(1) << ModbusAddress(5) << ModbusAddress(8);

    readRegister.resetRead(registerList, 100);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), ModbusAddress(0));
    QCOMPARE(readRegister.next().count(), 2);

    readRegister.addSuccess(ModbusAddress(0), QList<quint16>() << 1000 << 1001);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), ModbusAddress(5));
    QCOMPARE(readRegister.next().count(), 1);

    readRegister.addError();

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), ModbusAddress(8));
    QCOMPARE(readRegister.next().count(), 1);

    readRegister.addSuccess(ModbusAddress(8), QList<quint16>() << 1008);

    QVERIFY(!readRegister.hasNext());

    auto resultMap = readRegister.resultMap();

    QCOMPARE(resultMap.size(), registerList.size());


    QCOMPARE(resultMap.value(ModbusAddress(0)).value(), 1000);
    QVERIFY(resultMap.value(ModbusAddress(0)).isValid());

    QCOMPARE(resultMap.value(ModbusAddress(1)).value(), 1001);
    QVERIFY(resultMap.value(ModbusAddress(1)).isValid());

    QCOMPARE(resultMap.value(ModbusAddress(5)).value(), 0);
    QVERIFY(!resultMap.value(ModbusAddress(5)).isValid());

    QCOMPARE(resultMap.value(ModbusAddress(8)).value(), 1008);
    QVERIFY(resultMap.value(ModbusAddress(8)).isValid());
}

QTEST_GUILESS_MAIN(TestReadRegisters)
