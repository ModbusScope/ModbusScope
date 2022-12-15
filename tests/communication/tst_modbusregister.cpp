
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
    ModbusRegister reg(40001, Connection::ID_2, ModbusDataType::SIGNED_16);

    QCOMPARE(reg.address(), 40001);
    QCOMPARE(reg.connectionId(), Connection::ID_2);
    QCOMPARE(reg.type(), ModbusDataType::SIGNED_16);
}

void TestModbusRegister::comparison()
{
    ModbusRegister reg_1(40001, Connection::ID_1, ModbusDataType::SIGNED_16);
    ModbusRegister reg_2(40001, Connection::ID_1, ModbusDataType::SIGNED_16);

    QVERIFY(reg_1 == reg_2);

    reg_1.setAddress(40002);

    QVERIFY(!(reg_1 == reg_2));
}

void TestModbusRegister::copy()
{
    ModbusRegister reg_1(40001, Connection::ID_2, ModbusDataType::SIGNED_32);
    ModbusRegister reg_2;

    QVERIFY(!(reg_1 == reg_2));

    reg_2 = reg_1;

    QCOMPARE(reg_1.address(), 40001);
    QCOMPARE(reg_1.connectionId(), Connection::ID_2);
    QCOMPARE(reg_1.type(), ModbusDataType::SIGNED_32);

    QVERIFY(reg_1 == reg_2);
}

void TestModbusRegister::description()
{
    ModbusRegister reg_1(40001, Connection::ID_1, ModbusDataType::UNSIGNED_16);
    ModbusRegister reg_2(40002, Connection::ID_2, ModbusDataType::SIGNED_32);
    ModbusRegister reg_3(40003, Connection::ID_2, ModbusDataType::FLOAT_32);

    QCOMPARE(reg_1.description(), "40001, unsigned, 16 bit, conn 1");
    QCOMPARE(reg_2.description(), "40002, signed, 32 bit, conn 2");
    QCOMPARE(reg_3.description(), "40003, float32, conn 2");
}

QTEST_GUILESS_MAIN(TestModbusRegister)
