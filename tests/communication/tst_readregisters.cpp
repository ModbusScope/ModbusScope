
#include <QtTest/QtTest>

#include "tst_readregisters.h"

#include "readregisters.h"

void TestReadRegisters::init()
{

}

void TestReadRegisters::cleanup()
{

}

void TestReadRegisters::verifyAndAddErrorResult(ReadRegisters * pReadRegister, quint32 addr, quint16 cnt)
{
    QVERIFY(pReadRegister->hasNext());
    QCOMPARE(pReadRegister->next().address(), addr);
    QCOMPARE(pReadRegister->next().count(), cnt);

    pReadRegister->addError();
}

void TestReadRegisters::resetRead_1()
{
    ReadRegisters readRegister;
    QList<quint32> registerList = QList<quint32>() << 0;

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(&readRegister, 0, 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::resetRead_2()
{
    ReadRegisters readRegister;
    QList<quint32> registerList = QList<quint32>() << 0 << 1 << 2 << 3;

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(&readRegister, 0, 4);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::resetReadSplit_1()
{
    ReadRegisters readRegister;
    QList<quint32> registerList = QList<quint32>() << 0 << 1 << 3 << 5 << 6 << 8;

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(&readRegister, 0, 2);
    verifyAndAddErrorResult(&readRegister, 3, 1);
    verifyAndAddErrorResult(&readRegister, 5, 2);
    verifyAndAddErrorResult(&readRegister, 8, 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::resetReadSplit_2()
{
    ReadRegisters readRegister;
    QList<quint32> registerList = QList<quint32>() << 0 << 3;

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(&readRegister, 0, 1);
    verifyAndAddErrorResult(&readRegister, 3, 1);

    QVERIFY(!readRegister.hasNext());
}


void TestReadRegisters::consecutive_1()
{
    ReadRegisters readRegister;
    QList<quint32> registerList = QList<quint32>() << 0 << 1 << 2 << 3 << 4 << 5;

    readRegister.resetRead(registerList, 3);

    verifyAndAddErrorResult(&readRegister, 0, 3);
    verifyAndAddErrorResult(&readRegister, 3, 3);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::consecutive_2()
{
    ReadRegisters readRegister;
    QList<quint32> registerList = QList<quint32>() << 0 << 1;

    readRegister.resetRead(registerList, 5);

    verifyAndAddErrorResult(&readRegister, 0, 2);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::consecutive_3()
{
    ReadRegisters readRegister;
    QList<quint32> registerList = QList<quint32>() << 0 << 1 << 2;

    readRegister.resetRead(registerList, 2);

    verifyAndAddErrorResult(&readRegister, 0, 2);
    verifyAndAddErrorResult(&readRegister, 2, 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::splitNextToSingleReads_1()
{
    ReadRegisters readRegister;
    QList<quint32> registerList = QList<quint32>() << 0;

    readRegister.resetRead(registerList, 2);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), 0);
    QCOMPARE(readRegister.next().count(), 1);

    readRegister.splitNextToSingleReads();

    verifyAndAddErrorResult(&readRegister, 0, 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::splitNextToSingleReads_2()
{
    ReadRegisters readRegister;
    QList<quint32> registerList = QList<quint32>() << 0 << 1 << 2;

    readRegister.resetRead(registerList, 100);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), 0);
    QCOMPARE(readRegister.next().count(), 3);

    readRegister.splitNextToSingleReads();

    verifyAndAddErrorResult(&readRegister, 0, 1);
    verifyAndAddErrorResult(&readRegister, 1, 1);
    verifyAndAddErrorResult(&readRegister, 2, 1);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::splitNextToSingleReads_3()
{
    ReadRegisters readRegister;
    QList<quint32> registerList = QList<quint32>() << 0 << 1 << 2 << 5 << 6;

    readRegister.resetRead(registerList, 100);

    QVERIFY(readRegister.hasNext());
    QCOMPARE(readRegister.next().address(), 0);
    QCOMPARE(readRegister.next().count(), 3);

    readRegister.splitNextToSingleReads();

    verifyAndAddErrorResult(&readRegister, 0, 1);
    verifyAndAddErrorResult(&readRegister, 1, 1);
    verifyAndAddErrorResult(&readRegister, 2, 1);

    verifyAndAddErrorResult(&readRegister, 5, 2);

    QVERIFY(!readRegister.hasNext());
}

void TestReadRegisters::addAllErrors()
{
    ReadRegisters readRegister;
    QList<quint32> registerList = QList<quint32>() << 0 << 1 << 2 << 5 << 6 << 7;

    readRegister.resetRead(registerList, 100);

    readRegister.addAllErrors();

    QVERIFY(!readRegister.hasNext());

    QMap<quint32, Result<quint16>> resultMap = readRegister.resultMap();

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
    QList<quint32> registerList = QList<quint32>() << 0 << 1 << 2 << 5 << 6 << 8;

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

    QMap<quint32, Result<quint16>> resultMap = readRegister.resultMap();

    QCOMPARE(resultMap.size(), registerList.size());

    for(quint16 idx = 0; idx < static_cast<quint16>(registerList.size()); idx++)
    {
        QCOMPARE(resultMap.value(registerList[idx]).value(), registerList[idx] + 1000);
        QVERIFY(resultMap.value(registerList[idx]).isValid());
    }
}


void TestReadRegisters::addSuccessAndErrors()
{
    ReadRegisters readRegister;
    QList<quint32> registerList = QList<quint32>() << 0 << 1 << 5 << 8;

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

    QMap<quint32, Result<quint16>> resultMap = readRegister.resultMap();

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
