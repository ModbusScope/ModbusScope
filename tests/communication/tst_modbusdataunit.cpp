
#include "tst_modbusdataunit.h"

#include "communication/modbusdataunit.h"

#include <QTest>

void TestModbusDataUnit::init()
{
}

void TestModbusDataUnit::cleanup()
{
}

void TestModbusDataUnit::constructor_params()
{
    ModbusDataUnit unit(123, ModbusAddress::ObjectType::COIL, 42);
    QCOMPARE(unit.protocolAddress(), static_cast<quint16>(123));
    QCOMPARE(unit.objectType(), ModbusAddress::ObjectType::COIL);
    QCOMPARE(unit.slaveId(), static_cast<ModbusDataUnit::slaveId_t>(42));
}

void TestModbusDataUnit::constructor_from_address()
{
    ModbusAddress addr(40001);
    ModbusDataUnit unit(addr, 7);
    QCOMPARE(unit.protocolAddress(), addr.protocolAddress());
    QCOMPARE(unit.objectType(), addr.objectType());
    QCOMPARE(unit.slaveId(), static_cast<ModbusDataUnit::slaveId_t>(7));
}

void TestModbusDataUnit::slave_getter_setter()
{
    ModbusDataUnit unit(10, ModbusAddress::ObjectType::COIL, 5);
    QCOMPARE(unit.slaveId(), static_cast<ModbusDataUnit::slaveId_t>(5));

    unit.setSlaveId(99);
    QCOMPARE(unit.slaveId(), static_cast<ModbusDataUnit::slaveId_t>(99));
}

void TestModbusDataUnit::operator_equality()
{
    ModbusDataUnit unit1(10, ModbusAddress::ObjectType::COIL, 5);
    ModbusDataUnit unit2(10, ModbusAddress::ObjectType::COIL, 5);
    ModbusDataUnit unit3(10, ModbusAddress::ObjectType::COIL, 6);
    ModbusDataUnit unit4(11, ModbusAddress::ObjectType::COIL, 5);
    ModbusDataUnit unit5(10, ModbusAddress::ObjectType::INPUT_REGISTER, 5);

    QVERIFY(unit1 == unit2);
    QVERIFY(!(unit1 == unit3));
    QVERIFY(!(unit1 == unit4));
    QVERIFY(!(unit1 == unit5));
}

void TestModbusDataUnit::operator_less_than()
{
    ModbusDataUnit unit1(10, ModbusAddress::ObjectType::COIL, 5);
    ModbusDataUnit unit2(11, ModbusAddress::ObjectType::COIL, 5);
    ModbusDataUnit unit3(10, ModbusAddress::ObjectType::DISCRETE_INPUT, 5);
    ModbusDataUnit unit4(10, ModbusAddress::ObjectType::COIL, 6); // same addr+type, different slaveId

    QVERIFY(unit1 < unit2); // protocolAddress comparison
    QVERIFY(unit1 < unit3); // type comparison
    QVERIFY(!(unit2 < unit1));
    QVERIFY(!(unit3 < unit1));
    // slaveId comparison (regression for multi-slave bug)
    QVERIFY(unit1 < unit4); // slaveId 5 < slaveId 6, same address+type
    QVERIFY(!(unit4 < unit1));
}

void TestModbusDataUnit::qmap_different_slave_ids_are_distinct_keys()
{
    ModbusDataUnit unit1(40001, ModbusAddress::ObjectType::HOLDING_REGISTER, 1);
    ModbusDataUnit unit2(40001, ModbusAddress::ObjectType::HOLDING_REGISTER, 2);

    QMap<ModbusDataUnit, int> map;
    map.insert(unit1, 100);
    map.insert(unit2, 200);

    QCOMPARE(map.size(), 2);
    QCOMPARE(map.value(unit1), 100);
    QCOMPARE(map.value(unit2), 200);
}

void TestModbusDataUnit::toString()
{
    ModbusDataUnit unit(123, ModbusAddress::ObjectType::INPUT_REGISTER, 42);
    QString expected = "input register, 123, slave id 42";
    QCOMPARE(unit.toString(), expected);
}

QTEST_GUILESS_MAIN(TestModbusDataUnit)
