
#include <QtTest/QtTest>

#include "connectiontypes.h"
#include "modbusregister.h"
#include "tst_modbusregister.h"

void TestModbusRegister::init()
{

}

void TestModbusRegister::cleanup()
{

}

void TestModbusRegister::constructor()
{
    ModbusRegister reg(40001, Connection::ID_1, "16b");

    QCOMPARE(reg.address(), 40001);
    QCOMPARE(reg.connectionId(), Connection::ID_1);
    QCOMPARE(reg.is32Bit(), false);
    QCOMPARE(reg.isUnsigned(), true);
    QCOMPARE(reg.isFloat(), false);
}

void TestModbusRegister::constructor_s32()
{
    ModbusRegister reg(40001, Connection::ID_1, "s32b");

    QCOMPARE(reg.address(), 40001);
    QCOMPARE(reg.connectionId(), Connection::ID_1);
    QCOMPARE(reg.is32Bit(), true);
    QCOMPARE(reg.isUnsigned(), false);
    QCOMPARE(reg.isFloat(), false);
}

void TestModbusRegister::constructor_float32()
{
    ModbusRegister reg(40001, Connection::ID_1, "float32");

    QCOMPARE(reg.address(), 40001);
    QCOMPARE(reg.connectionId(), Connection::ID_1);
    QCOMPARE(reg.is32Bit(), true);
    QCOMPARE(reg.isUnsigned(), false);
    QCOMPARE(reg.isFloat(), true);
}

void TestModbusRegister::comparison()
{
    ModbusRegister reg_1(40001, Connection::ID_1, "16b");
    ModbusRegister reg_2(40001, Connection::ID_1, "16b");

    QVERIFY(reg_1 == reg_2);

    reg_1.setAddress(40002);

    QVERIFY(!(reg_1 == reg_2));
}

void TestModbusRegister::copy()
{
    ModbusRegister reg_1(40001, Connection::ID_2, "s32b");
    ModbusRegister reg_2;

    QVERIFY(!(reg_1 == reg_2));

    reg_2 = reg_1;

    QCOMPARE(reg_1.address(), 40001);
    QCOMPARE(reg_1.connectionId(), Connection::ID_2);
    QCOMPARE(reg_1.is32Bit(), true);
    QCOMPARE(reg_1.isUnsigned(), false);

    QVERIFY(reg_1 == reg_2);
}

void TestModbusRegister::description()
{
    ModbusRegister reg_1(40001, Connection::ID_1, "16b");
    ModbusRegister reg_2(40002, Connection::ID_2, "s32b");
    ModbusRegister reg_3(40003, Connection::ID_2, "float32");

    QCOMPARE(reg_1.description(), "40001, unsigned, 16 bit, conn 1");
    QCOMPARE(reg_2.description(), "40002, signed, 32 bit, conn 2");
    QCOMPARE(reg_3.description(), "40003, float32, conn 2");
}

QTEST_GUILESS_MAIN(TestModbusRegister)
