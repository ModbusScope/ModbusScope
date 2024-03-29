
#include <QtTest/QtTest>

#include "modbusaddress.h"

#include "tst_modbusaddress.h"

using ObjectType = ModbusAddress::ObjectType;
using Offset = ModbusAddress::Offset;

void TestModbusAddress::init()
{

}

void TestModbusAddress::cleanup()
{

}

void TestModbusAddress::constructor_default()
{
    ModbusAddress addr;

    QCOMPARE(addr.address(Offset::WITH_OFFSET), 40001);
    QCOMPARE(addr.objectType(), ObjectType::HOLDING_REGISTER);
}

void TestModbusAddress::constructor_address()
{
    ModbusAddress addr(40001);

    QCOMPARE(addr.address(Offset::WITH_OFFSET), 40001);
    QCOMPARE(addr.objectType(), ObjectType::HOLDING_REGISTER);
}

void TestModbusAddress::constructor_type()
{
    ModbusAddress addr(1);

    QCOMPARE(addr.address(Offset::WITHOUT_OFFSET), 1);
    QCOMPARE(addr.objectType(), ObjectType::COIL);
}

void TestModbusAddress::constructor_assignment()
{
    ModbusAddress addrOriginal(40001);
    ModbusAddress addr;

    addr = addrOriginal;

    QCOMPARE(addr.address(Offset::WITH_OFFSET), 40001);
    QCOMPARE(addr.objectType(), ObjectType::HOLDING_REGISTER);
}

void TestModbusAddress::constructor_copy()
{
    ModbusAddress addrOriginal(40001);
    ModbusAddress addr(addrOriginal);

    QCOMPARE(addr.address(Offset::WITH_OFFSET), 40001);
    QCOMPARE(addr.objectType(), ObjectType::HOLDING_REGISTER);
}

void TestModbusAddress::get_address()
{
    ModbusAddress addr_1(0, ObjectType::COIL);
    ModbusAddress addr_2(0, ObjectType::DISCRETE_INPUT);
    ModbusAddress addr_3(0, ObjectType::INPUT_REGISTER);
    ModbusAddress addr_4(0, ObjectType::HOLDING_REGISTER);

    QCOMPARE(addr_1.address(Offset::WITH_OFFSET), 0);
    QCOMPARE(addr_1.address(Offset::WITHOUT_OFFSET), 0);

    QCOMPARE(addr_2.address(Offset::WITH_OFFSET), 10001);
    QCOMPARE(addr_2.address(Offset::WITHOUT_OFFSET), 0);

    QCOMPARE(addr_3.address(Offset::WITH_OFFSET), 30001);
    QCOMPARE(addr_3.address(Offset::WITHOUT_OFFSET), 0);

    QCOMPARE(addr_4.address(Offset::WITH_OFFSET), 40001);
    QCOMPARE(addr_4.address(Offset::WITHOUT_OFFSET), 0);
}

void TestModbusAddress::large_address()
{
    ModbusAddress addr(40001 + 30000);

    QCOMPARE(addr.address(Offset::WITH_OFFSET), 70001);
    QCOMPARE(addr.address(Offset::WITHOUT_OFFSET), 30000);
}

void TestModbusAddress::to_string()
{
    ModbusAddress addr(40011);

    QCOMPARE(addr.toString(), "holding register, 10");
}

void TestModbusAddress::to_string_coil()
{
    ModbusAddress addr(1);

    QCOMPARE(addr.toString(), "coil, 1");
}

void TestModbusAddress::next_and_compare()
{
    ModbusAddress addr(40001);

    QCOMPARE(addr.next(), ModbusAddress(40002));
    QCOMPARE(addr.next(1), ModbusAddress(40002));
    QCOMPARE(addr.next(2), ModbusAddress(40003));
}

void TestModbusAddress::greater()
{
    ModbusAddress addr_1(0, ObjectType::COIL);
    ModbusAddress addr_2(1, ObjectType::COIL);
    ModbusAddress addr_3(0, ObjectType::HOLDING_REGISTER);
    ModbusAddress addr_4(2, ObjectType::HOLDING_REGISTER);

    QVERIFY(addr_2 > addr_1);
    QVERIFY(!(addr_1 > addr_2));

    QVERIFY(addr_3 > addr_1);

    QVERIFY(addr_4 > addr_3);
    QVERIFY(!(addr_3 > addr_4));
}

void TestModbusAddress::smaller()
{
    ModbusAddress addr_1(0, ObjectType::COIL);
    ModbusAddress addr_2(1, ObjectType::COIL);
    ModbusAddress addr_3(0, ObjectType::HOLDING_REGISTER);
    ModbusAddress addr_4(2, ObjectType::HOLDING_REGISTER);

    QVERIFY(addr_1 < addr_2);
    QVERIFY(!(addr_2 < addr_1));

    QVERIFY(addr_1 < addr_3);

    QVERIFY(addr_3 < addr_4);
    QVERIFY(!(addr_4 < addr_3));
}

void TestModbusAddress::sort_large_object_address()
{
    auto act_addrList = QList<ModbusAddress>() << ModbusAddress(0, ObjectType::HOLDING_REGISTER)
                                               << ModbusAddress(40200, ObjectType::COIL); // Large coil address


    auto exp_addrList = QList<ModbusAddress>() << ModbusAddress(40200, ObjectType::COIL) // Coils always come first
                                               << ModbusAddress(0, ObjectType::HOLDING_REGISTER);


    std::sort(act_addrList.begin(), act_addrList.end(), std::less<ModbusAddress>());

    QCOMPARE(act_addrList, exp_addrList);
}

QTEST_GUILESS_MAIN(TestModbusAddress)
