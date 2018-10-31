#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "src/communication/readregisters.h"

using namespace testing;

void verifyAndAddErrorResult(ReadRegisters * pReadRegister, quint16 addr, quint16 cnt)
{
    EXPECT_TRUE(pReadRegister->hasNext());
    EXPECT_EQ(pReadRegister->next().address(), addr);
    EXPECT_EQ(pReadRegister->next().count(), cnt);

    pReadRegister->addError();
}

TEST(ReadRegisters, resetRead_1)
{
    ReadRegisters readRegister;
    QList<quint16> registerList = QList<quint16>() << 0;

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(&readRegister, 0, 1);

    EXPECT_FALSE(readRegister.hasNext());
}

TEST(ReadRegisters, resetRead_2)
{
    ReadRegisters readRegister;
    QList<quint16> registerList = QList<quint16>() << 0 << 1 << 2 << 3;

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(&readRegister, 0, 4);

    EXPECT_FALSE(readRegister.hasNext());
}

TEST(ReadRegisters, resetReadSplit_1)
{
    ReadRegisters readRegister;
    QList<quint16> registerList = QList<quint16>() << 0 << 1 << 3 << 5 << 6 << 8;

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(&readRegister, 0, 2);
    verifyAndAddErrorResult(&readRegister, 3, 1);
    verifyAndAddErrorResult(&readRegister, 5, 2);
    verifyAndAddErrorResult(&readRegister, 8, 1);

    EXPECT_FALSE(readRegister.hasNext());
}

TEST(ReadRegisters, resetReadSplit_2)
{
    ReadRegisters readRegister;
    QList<quint16> registerList = QList<quint16>() << 0 << 3;

    readRegister.resetRead(registerList, 255);

    verifyAndAddErrorResult(&readRegister, 0, 1);
    verifyAndAddErrorResult(&readRegister, 3, 1);

    EXPECT_FALSE(readRegister.hasNext());
}


TEST(ReadRegisters, consecutive_1)
{
    ReadRegisters readRegister;
    QList<quint16> registerList = QList<quint16>() << 0 << 1 << 2 << 3 << 4 << 5;

    readRegister.resetRead(registerList, 3);

    verifyAndAddErrorResult(&readRegister, 0, 3);
    verifyAndAddErrorResult(&readRegister, 3, 3);

    EXPECT_FALSE(readRegister.hasNext());
}

TEST(ReadRegisters, consecutive_2)
{
    ReadRegisters readRegister;
    QList<quint16> registerList = QList<quint16>() << 0 << 1;

    readRegister.resetRead(registerList, 5);

    verifyAndAddErrorResult(&readRegister, 0, 2);

    EXPECT_FALSE(readRegister.hasNext());
}

TEST(ReadRegisters, consecutive_3)
{
    ReadRegisters readRegister;
    QList<quint16> registerList = QList<quint16>() << 0 << 1 << 2;

    readRegister.resetRead(registerList, 2);

    verifyAndAddErrorResult(&readRegister, 0, 2);
    verifyAndAddErrorResult(&readRegister, 2, 1);

    EXPECT_FALSE(readRegister.hasNext());
}

TEST(ReadRegisters, splitNextToSingleReads_1)
{
    ReadRegisters readRegister;
    QList<quint16> registerList = QList<quint16>() << 0;

    readRegister.resetRead(registerList, 2);

    EXPECT_TRUE(readRegister.hasNext());
    EXPECT_EQ(readRegister.next().address(), 0);
    EXPECT_EQ(readRegister.next().count(), 1);

    readRegister.splitNextToSingleReads();

    verifyAndAddErrorResult(&readRegister, 0, 1);

    EXPECT_FALSE(readRegister.hasNext());
}

TEST(ReadRegisters, splitNextToSingleReads_2)
{
    ReadRegisters readRegister;
    QList<quint16> registerList = QList<quint16>() << 0 << 1 << 2;

    readRegister.resetRead(registerList, 100);

    EXPECT_TRUE(readRegister.hasNext());
    EXPECT_EQ(readRegister.next().address(), 0);
    EXPECT_EQ(readRegister.next().count(), 3);

    readRegister.splitNextToSingleReads();

    verifyAndAddErrorResult(&readRegister, 0, 1);
    verifyAndAddErrorResult(&readRegister, 1, 1);
    verifyAndAddErrorResult(&readRegister, 2, 1);

    EXPECT_FALSE(readRegister.hasNext());
}

TEST(ReadRegisters, splitNextToSingleReads_3)
{
    ReadRegisters readRegister;
    QList<quint16> registerList = QList<quint16>() << 0 << 1 << 2 << 5 << 6;

    readRegister.resetRead(registerList, 100);

    EXPECT_TRUE(readRegister.hasNext());
    EXPECT_EQ(readRegister.next().address(), 0);
    EXPECT_EQ(readRegister.next().count(), 3);

    readRegister.splitNextToSingleReads();

    verifyAndAddErrorResult(&readRegister, 0, 1);
    verifyAndAddErrorResult(&readRegister, 1, 1);
    verifyAndAddErrorResult(&readRegister, 2, 1);

    verifyAndAddErrorResult(&readRegister, 5, 2);

    EXPECT_FALSE(readRegister.hasNext());
}

TEST(ReadRegisters, addAllErrors)
{
    ReadRegisters readRegister;
    QList<quint16> registerList = QList<quint16>() << 0 << 1 << 2 << 5 << 6 << 7;

    readRegister.resetRead(registerList, 100);

    readRegister.addAllErrors();

    EXPECT_FALSE(readRegister.hasNext());

    QMap<quint16, ModbusResult> resultMap = readRegister.resultMap();

    EXPECT_EQ(resultMap.size(), registerList.size());

    for(quint16 idx = 0; idx < static_cast<quint16>(registerList.size()); idx++)
    {
        EXPECT_EQ(resultMap.value(registerList[idx]).value(), 0);
        EXPECT_FALSE(resultMap.value(registerList[idx]).isSuccess());
    }
}

TEST(ReadRegisters, addSuccess)
{
    ReadRegisters readRegister;
    QList<quint16> registerList = QList<quint16>() << 0 << 1 << 2 << 5 << 6 << 8;

    readRegister.resetRead(registerList, 100);

    EXPECT_TRUE(readRegister.hasNext());
    EXPECT_EQ(readRegister.next().address(), 0);
    EXPECT_EQ(readRegister.next().count(), 3);

    readRegister.addSuccess(0, QList<quint16>() << 1000 << 1001 << 1002);

    EXPECT_TRUE(readRegister.hasNext());
    EXPECT_EQ(readRegister.next().address(), 5);
    EXPECT_EQ(readRegister.next().count(), 2);

    readRegister.addSuccess(5, QList<quint16>() << 1005 << 1006);

    EXPECT_TRUE(readRegister.hasNext());
    EXPECT_EQ(readRegister.next().address(), 8);
    EXPECT_EQ(readRegister.next().count(), 1);

    readRegister.addSuccess(8, QList<quint16>() << 1008);

    EXPECT_FALSE(readRegister.hasNext());

    QMap<quint16, ModbusResult> resultMap = readRegister.resultMap();

    EXPECT_EQ(resultMap.size(), registerList.size());

    for(quint16 idx = 0; idx < static_cast<quint16>(registerList.size()); idx++)
    {
        EXPECT_EQ(resultMap.value(registerList[idx]).value(), registerList[idx] + 1000);
        EXPECT_TRUE(resultMap.value(registerList[idx]).isSuccess());
    }
}


TEST(ReadRegisters, addSuccessAndErrors)
{
    ReadRegisters readRegister;
    QList<quint16> registerList = QList<quint16>() << 0 << 1 << 5 << 8;

    readRegister.resetRead(registerList, 100);

    EXPECT_TRUE(readRegister.hasNext());
    EXPECT_EQ(readRegister.next().address(), 0);
    EXPECT_EQ(readRegister.next().count(), 2);

    readRegister.addSuccess(0, QList<quint16>() << 1000 << 1001);

    EXPECT_TRUE(readRegister.hasNext());
    EXPECT_EQ(readRegister.next().address(), 5);
    EXPECT_EQ(readRegister.next().count(), 1);

    readRegister.addError();

    EXPECT_TRUE(readRegister.hasNext());
    EXPECT_EQ(readRegister.next().address(), 8);
    EXPECT_EQ(readRegister.next().count(), 1);

    readRegister.addSuccess(8, QList<quint16>() << 1008);

    EXPECT_FALSE(readRegister.hasNext());

    QMap<quint16, ModbusResult> resultMap = readRegister.resultMap();

    EXPECT_EQ(resultMap.size(), registerList.size());


    EXPECT_EQ(resultMap.value(0).value(), 1000);
    EXPECT_TRUE(resultMap.value(0).isSuccess());

    EXPECT_EQ(resultMap.value(1).value(), 1001);
    EXPECT_TRUE(resultMap.value(1).isSuccess());

    EXPECT_EQ(resultMap.value(5).value(), 0);
    EXPECT_FALSE(resultMap.value(5).isSuccess());

    EXPECT_EQ(resultMap.value(8).value(), 1008);
    EXPECT_TRUE(resultMap.value(8).isSuccess());
}
