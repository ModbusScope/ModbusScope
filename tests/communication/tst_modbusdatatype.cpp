
#include <QtTest/QtTest>

#include "modbusdatatype.h"
#include "tst_modbusdatatype.h"

using Type = ModbusDataType::Type;

void TestModbusDataType::init()
{

}

void TestModbusDataType::cleanup()
{

}

void TestModbusDataType::is32bit()
{
    QCOMPARE(ModbusDataType::is32Bit(Type::UNSIGNED_16), false);
    QCOMPARE(ModbusDataType::is32Bit(Type::SIGNED_16), false);
    QCOMPARE(ModbusDataType::is32Bit(Type::UNSIGNED_32), true);
    QCOMPARE(ModbusDataType::is32Bit(Type::SIGNED_32), true);
    QCOMPARE(ModbusDataType::is32Bit(Type::FLOAT_32), true);
}

void TestModbusDataType::isUnsigned()
{
    QCOMPARE(ModbusDataType::isUnsigned(Type::UNSIGNED_16), true);
    QCOMPARE(ModbusDataType::isUnsigned(Type::SIGNED_16), false);
    QCOMPARE(ModbusDataType::isUnsigned(Type::UNSIGNED_32), true);
    QCOMPARE(ModbusDataType::isUnsigned(Type::SIGNED_32), false);
    QCOMPARE(ModbusDataType::isUnsigned(Type::FLOAT_32), false);
}

void TestModbusDataType::isFloat()
{
    QCOMPARE(ModbusDataType::isFloat(Type::UNSIGNED_16), false);
    QCOMPARE(ModbusDataType::isFloat(Type::SIGNED_16), false);
    QCOMPARE(ModbusDataType::isFloat(Type::UNSIGNED_32), false);
    QCOMPARE(ModbusDataType::isFloat(Type::SIGNED_32), false);
    QCOMPARE(ModbusDataType::isFloat(Type::FLOAT_32), true);
}

void TestModbusDataType::convertString_data()
{
    #define ADD_TEST(strType, type)      QTest::newRow(strType) << QString(strType) << static_cast<ModbusDataType::Type>(type)

    QTest::addColumn<QString>("strType");
    QTest::addColumn<ModbusDataType::Type>("type");

    ADD_TEST("16b", Type::UNSIGNED_16);
    ADD_TEST("s16b", Type::SIGNED_16);
    ADD_TEST("32b", Type::UNSIGNED_32);
    ADD_TEST("s32b", Type::SIGNED_32);
    ADD_TEST("f32b", Type::FLOAT_32);
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
    QCOMPARE(actType, Type::UNSIGNED_16);
}

//convertString

QTEST_GUILESS_MAIN(TestModbusDataType)
