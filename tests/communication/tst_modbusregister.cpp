
#include <QtTest/QtTest>

#include "connectiontypes.h"
#include "modbusregister.h"
#include "tst_modbusregister.h"

using Type = ModbusDataType::Type;

#define ADD_TEST_16(name, registerValue, value)      QTest::newRow(name) << static_cast<uint16_t>(registerValue)       \
                                                                        << static_cast<double>(value)

#define ADD_TEST_32(name, lowerRegister, upperRegister, value)      QTest::newRow(name) << static_cast<uint16_t>(lowerRegister)       \
                                                                                     << static_cast<uint16_t>(upperRegister)       \
                                                                                     << static_cast<double>(value)

void TestModbusRegister::init()
{

}

void TestModbusRegister::cleanup()
{

}

void TestModbusRegister::constructor()
{
    ModbusRegister reg(40001, Connection::ID_2, Type::SIGNED_16);

    QCOMPARE(reg.address(), 40001);
    QCOMPARE(reg.connectionId(), Connection::ID_2);
    QCOMPARE(reg.type(), Type::SIGNED_16);
}

void TestModbusRegister::comparison()
{
    ModbusRegister reg_1(40001, Connection::ID_1, Type::SIGNED_16);
    ModbusRegister reg_2(40001, Connection::ID_1, Type::SIGNED_16);

    QVERIFY(reg_1 == reg_2);

    reg_1.setAddress(40002);

    QVERIFY(!(reg_1 == reg_2));
}

void TestModbusRegister::copy()
{
    ModbusRegister reg_1(40001, Connection::ID_2, Type::SIGNED_32);
    ModbusRegister reg_2;

    QVERIFY(!(reg_1 == reg_2));

    reg_2 = reg_1;

    QCOMPARE(reg_1.address(), 40001);
    QCOMPARE(reg_1.connectionId(), Connection::ID_2);
    QCOMPARE(reg_1.type(), Type::SIGNED_32);

    QVERIFY(reg_1 == reg_2);
}

void TestModbusRegister::description()
{
    ModbusRegister reg_1(40001, Connection::ID_1, Type::UNSIGNED_16);
    ModbusRegister reg_2(40002, Connection::ID_2, Type::SIGNED_32);
    ModbusRegister reg_3(40003, Connection::ID_2, Type::FLOAT_32);

    QCOMPARE(reg_1.description(), "40001, unsigned, 16 bit, conn 1");
    QCOMPARE(reg_2.description(), "40002, signed, 32 bit, conn 2");
    QCOMPARE(reg_3.description(), "40003, float32, conn 2");
}

void TestModbusRegister::processValue_16b_data()
{
    QTest::addColumn<uint16_t>("registerValue");
    QTest::addColumn<double>("expValue");

    ADD_TEST_16("16b_1", 0, 0);
    ADD_TEST_16("16b_2", 0xFFFF, UINT16_MAX);
}

void TestModbusRegister::processValue_16b()
{
    QFETCH(uint16_t, registerValue);
    QFETCH(double, expValue);

    ModbusRegister reg(40001, Connection::ID_1, Type::UNSIGNED_16);

    double actValue = reg.processValue(registerValue, 0, false);
    QCOMPARE(actValue, expValue);
}

void TestModbusRegister::processValue_s16b_data()
{
    QTest::addColumn<uint16_t>("registerValue");
    QTest::addColumn<double>("expValue");

    ADD_TEST_16("s16b_1", 0, 0);
    ADD_TEST_16("s16b_2", 0xFFFF, -1);
}

void TestModbusRegister::processValue_s16b()
{
    QFETCH(uint16_t, registerValue);
    QFETCH(double, expValue);

    ModbusRegister reg(40001, Connection::ID_1, Type::SIGNED_16);

    double actValue = reg.processValue(registerValue, 0, false);
    QCOMPARE(actValue, expValue);
}

void TestModbusRegister::processValue_32b_data()
{
    QTest::addColumn<uint16_t>("lowerRegister");
    QTest::addColumn<uint16_t>("upperRegister");
    QTest::addColumn<double>("expValue");

    ADD_TEST_32("32b_1", 0, 0, 0);
    ADD_TEST_32("32b_2", 0xFFFF, 0xFFFF, UINT32_MAX);
}

void TestModbusRegister::processValue_32b()
{
    QFETCH(uint16_t, lowerRegister);
    QFETCH(uint16_t, upperRegister);
    QFETCH(double, expValue);

    ModbusRegister reg(40001, Connection::ID_1, Type::UNSIGNED_32);

    double actValue = reg.processValue(lowerRegister, upperRegister, false);
    QCOMPARE(actValue, expValue);

    double actValue2 = reg.processValue(upperRegister, lowerRegister, true);
    QCOMPARE(actValue2, expValue);
}

void TestModbusRegister::processValue_s32b_data()
{
    QTest::addColumn<uint16_t>("lowerRegister");
    QTest::addColumn<uint16_t>("upperRegister");
    QTest::addColumn<double>("expValue");

    ADD_TEST_32("s32b_1", 0, 0, 0);
    ADD_TEST_32("s32b_2", 0xFFFF, 0xFFFF, -1);
}

void TestModbusRegister::processValue_s32b()
{
    QFETCH(uint16_t, lowerRegister);
    QFETCH(uint16_t, upperRegister);
    QFETCH(double, expValue);

    ModbusRegister reg(40001, Connection::ID_1, Type::SIGNED_32);

    double actValue = reg.processValue(lowerRegister, upperRegister, false);
    QCOMPARE(actValue, expValue);

    double actValue2 = reg.processValue(upperRegister, lowerRegister, true);
    QCOMPARE(actValue2, expValue);
}

QTEST_GUILESS_MAIN(TestModbusRegister)
