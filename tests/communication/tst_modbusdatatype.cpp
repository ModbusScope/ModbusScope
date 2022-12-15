
#include <QtTest/QtTest>

#include "modbusdatatype.h"
#include "tst_modbusdatatype.h"

void TestModbusDataType::init()
{

}

void TestModbusDataType::cleanup()
{

}

void TestModbusDataType::is32bit()
{
    QCOMPARE(ModbusDataType::is32Bit(ModbusDataType::UNSIGNED_16), false);
    QCOMPARE(ModbusDataType::is32Bit(ModbusDataType::SIGNED_16), false);
    QCOMPARE(ModbusDataType::is32Bit(ModbusDataType::UNSIGNED_32), true);
    QCOMPARE(ModbusDataType::is32Bit(ModbusDataType::SIGNED_32), true);
    QCOMPARE(ModbusDataType::is32Bit(ModbusDataType::FLOAT_32), true);
}

void TestModbusDataType::isUnsigned()
{
    QCOMPARE(ModbusDataType::isUnsigned(ModbusDataType::UNSIGNED_16), true);
    QCOMPARE(ModbusDataType::isUnsigned(ModbusDataType::SIGNED_16), false);
    QCOMPARE(ModbusDataType::isUnsigned(ModbusDataType::UNSIGNED_32), true);
    QCOMPARE(ModbusDataType::isUnsigned(ModbusDataType::SIGNED_32), false);
    QCOMPARE(ModbusDataType::isUnsigned(ModbusDataType::FLOAT_32), false);
}

void TestModbusDataType::isFloat()
{
    QCOMPARE(ModbusDataType::isFloat(ModbusDataType::UNSIGNED_16), false);
    QCOMPARE(ModbusDataType::isFloat(ModbusDataType::SIGNED_16), false);
    QCOMPARE(ModbusDataType::isFloat(ModbusDataType::UNSIGNED_32), false);
    QCOMPARE(ModbusDataType::isFloat(ModbusDataType::SIGNED_32), false);
    QCOMPARE(ModbusDataType::isFloat(ModbusDataType::FLOAT_32), true);
}

void TestModbusDataType::convertString_data()
{
    #define ADD_TEST(strType, type)      QTest::newRow(strType) << QString(strType) << static_cast<ModbusDataType::Type>(type)

    QTest::addColumn<QString>("strType");
    QTest::addColumn<ModbusDataType::Type>("type");

    ADD_TEST("16b", ModbusDataType::UNSIGNED_16);
    ADD_TEST("s16b", ModbusDataType::SIGNED_16);
    ADD_TEST("32b", ModbusDataType::UNSIGNED_32);
    ADD_TEST("s32b", ModbusDataType::SIGNED_32);
    ADD_TEST("float32", ModbusDataType::FLOAT_32);
}

void TestModbusDataType::convertString()
{
    QFETCH(QString, strType);
    QFETCH(ModbusDataType::Type, type);

    bool bOk;
    ModbusDataType::Type actType = ModbusDataType::convertString(strType, bOk);

    QCOMPARE(type, actType);
    QVERIFY(bOk);
}

void TestModbusDataType::convertStringUnknown()
{
    bool bOk;
    ModbusDataType::Type actType = ModbusDataType::convertString("noType", bOk);

    QVERIFY(!bOk);
    QCOMPARE(actType, ModbusDataType::UNSIGNED_16);
}

//convertString

QTEST_GUILESS_MAIN(TestModbusDataType)
