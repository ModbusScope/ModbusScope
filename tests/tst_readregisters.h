#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "src/communication/readregisters.h"

using namespace testing;

void verifyAndAddErrorResult(ReadRegisters * pReadRegister, quint16 addr, quint16 cnt)
{
    EXPECT_TRUE(pReadRegister->hasNext());
    EXPECT_EQ(pReadRegister->next().address(), addr);
    EXPECT_EQ(pReadRegister->next().count(), cnt);

    pReadRegister->addError(addr, cnt);
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


/* TODO: */
/* AddSuccess */
/* AddError */
/* AddAllErrors */
/* splitNextToSingleReads */
/* resultMap */

