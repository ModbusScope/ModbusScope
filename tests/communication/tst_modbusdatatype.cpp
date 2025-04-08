
#include "tst_modbusdatatype.h"

#include "util/modbusdatatype.h"

#include <QtTest/QtTest>

using Type = ModbusDataType::Type;

#define ADD_TYPE_TEST(strType, type)        QTest::newRow(strType) << strType << type
#define ADD_STRING_TEST(type, strType)      QTest::newRow(strType) << type << strType;

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
    QTest::addColumn<QString>("strType");
    QTest::addColumn<ModbusDataType::Type>("type");

    ADD_TYPE_TEST("16b", Type::UNSIGNED_16);
    ADD_TYPE_TEST("s16b", Type::SIGNED_16);
    ADD_TYPE_TEST("32b", Type::UNSIGNED_32);
    ADD_TYPE_TEST("s32b", Type::SIGNED_32);
    ADD_TYPE_TEST("f32b", Type::FLOAT_32);
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

void TestModbusDataType::convertMbcString_data()
{
    QTest::addColumn<QString>("strType");
    QTest::addColumn<ModbusDataType::Type>("type");

    ADD_TYPE_TEST("uint16", Type::UNSIGNED_16);
    ADD_TYPE_TEST("bin16", Type::UNSIGNED_16);
    ADD_TYPE_TEST("hex16", Type::UNSIGNED_16);
    ADD_TYPE_TEST("ascii16", Type::UNSIGNED_16);
    ADD_TYPE_TEST("int16", Type::SIGNED_16);
    ADD_TYPE_TEST("uint32", Type::UNSIGNED_32);
    ADD_TYPE_TEST("bin32", Type::UNSIGNED_32);
    ADD_TYPE_TEST("hex32", Type::UNSIGNED_32);
    ADD_TYPE_TEST("ascii32", Type::UNSIGNED_32);
    ADD_TYPE_TEST("int32", Type::SIGNED_32);
    ADD_TYPE_TEST("float32", Type::FLOAT_32);
}

void TestModbusDataType::convertMbcString()
{
    QFETCH(QString, strType);
    QFETCH(ModbusDataType::Type, type);

    bool bOk;
    ModbusDataType::Type actType = ModbusDataType::convertMbcString(strType, bOk);

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

void TestModbusDataType::typeString_data()
{
    QTest::addColumn<ModbusDataType::Type>("type");
    QTest::addColumn<QString>("strType");

    ADD_STRING_TEST(Type::UNSIGNED_16, "16b");
    ADD_STRING_TEST(Type::SIGNED_16, "s16b");
    ADD_STRING_TEST(Type::UNSIGNED_32, "32b");
    ADD_STRING_TEST(Type::SIGNED_32, "s32b");
    ADD_STRING_TEST(Type::FLOAT_32, "f32b");
    ADD_STRING_TEST(static_cast<ModbusDataType::Type>(255), "16b");
}

void TestModbusDataType::typeString()
{
    QFETCH(ModbusDataType::Type, type);
    QFETCH(QString, strType);

    QString actTypeString = ModbusDataType::typeString(type);

    QCOMPARE(strType, actTypeString);
}

void TestModbusDataType::description_data()
{
    QTest::addColumn<ModbusDataType::Type>("type");
    QTest::addColumn<QString>("strType");

    ADD_STRING_TEST(Type::UNSIGNED_16, "unsigned 16-bit");
    ADD_STRING_TEST(Type::SIGNED_16, "signed 16-bit");
    ADD_STRING_TEST(Type::UNSIGNED_32, "unsigned 32-bit");
    ADD_STRING_TEST(Type::SIGNED_32, "signed 32-bit");
    ADD_STRING_TEST(Type::FLOAT_32, "32-bit float");
    ADD_STRING_TEST(static_cast<ModbusDataType::Type>(255), "unsigned 16-bit");
}

void TestModbusDataType::description()
{
    QFETCH(ModbusDataType::Type, type);
    QFETCH(QString, strType);

    QString actDescriptionString = ModbusDataType::description(type);

    QCOMPARE(strType, actDescriptionString);
}

void TestModbusDataType::convertSettings_data()
{
    QTest::addColumn<bool>("is32bit");
    QTest::addColumn<bool>("bUnsigned");
    QTest::addColumn<bool>("bFloat");
    QTest::addColumn<ModbusDataType::Type>("type");

    /*                    32bit    unsigned float  */
    QTest::newRow("0") << false << true  << true  << ModbusDataType::Type::FLOAT_32;
    QTest::newRow("1") << false << true  << false << ModbusDataType::Type::UNSIGNED_16;
    QTest::newRow("2") << false << false << true  << ModbusDataType::Type::FLOAT_32;
    QTest::newRow("3") << false << false << false << ModbusDataType::Type::SIGNED_16;

    QTest::newRow("4") << true  << true  << true  << ModbusDataType::Type::FLOAT_32;
    QTest::newRow("5") << true  << true  << false << ModbusDataType::Type::UNSIGNED_32;
    QTest::newRow("6") << true  << false << true  << ModbusDataType::Type::FLOAT_32;
    QTest::newRow("7") << true  << false << false << ModbusDataType::Type::SIGNED_32;
}

void TestModbusDataType::convertSettings()
{
    QFETCH(ModbusDataType::Type, type);
    QFETCH(bool, is32bit);
    QFETCH(bool, bUnsigned);
    QFETCH(bool, bFloat);

    ModbusDataType::Type actType = ModbusDataType::convertSettings(is32bit, bUnsigned, bFloat);

    QCOMPARE(actType, type);
}

QTEST_GUILESS_MAIN(TestModbusDataType)
