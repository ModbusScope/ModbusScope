
#include "tst_modbusaddress.h"

#include "util/modbusaddress.h"

#include <QTest>

using ObjectType = ModbusAddress::ObjectType;

void TestModbusAddress::init()
{

}

void TestModbusAddress::cleanup()
{

}

void TestModbusAddress::constructor_default()
{
    ModbusAddress addr;

    QCOMPARE(addr.fullAddress(), "40001");
    QCOMPARE(addr.objectType(), ObjectType::HOLDING_REGISTER);
}

void TestModbusAddress::constructor_address()
{
    ModbusAddress addr(40001);

    QCOMPARE(addr.fullAddress(), "40001");
    QCOMPARE(addr.objectType(), ObjectType::HOLDING_REGISTER);
}

void TestModbusAddress::constructor_string_address()
{
    ModbusAddress addr_1("h20000");
    QCOMPARE(addr_1.protocolAddress(), 20000);
    QCOMPARE(addr_1.objectType(), ObjectType::HOLDING_REGISTER);

    ModbusAddress addr_2("d20000");
    QCOMPARE(addr_2.protocolAddress(), 20000);
    QCOMPARE(addr_2.objectType(), ObjectType::DISCRETE_INPUT);

    ModbusAddress addr_3("c20000");
    QCOMPARE(addr_3.protocolAddress(), 20000);
    QCOMPARE(addr_3.objectType(), ObjectType::COIL);

    ModbusAddress addr_4("i20000");
    QCOMPARE(addr_4.protocolAddress(), 20000);
    QCOMPARE(addr_4.objectType(), ObjectType::INPUT_REGISTER);

    ModbusAddress addr_5("40001");
    QCOMPARE(addr_5.protocolAddress(), 0);
    QCOMPARE(addr_5.objectType(), ObjectType::HOLDING_REGISTER);
}

void TestModbusAddress::constructor_type()
{
    ModbusAddress addr(1);

    QCOMPARE(addr.protocolAddress(), 1);
    QCOMPARE(addr.objectType(), ObjectType::COIL);
}

void TestModbusAddress::constructor_assignment()
{
    ModbusAddress addrOriginal(40001);
    ModbusAddress addr;

    addr = addrOriginal;

    QCOMPARE(addr.fullAddress(), "40001");
    QCOMPARE(addr.objectType(), ObjectType::HOLDING_REGISTER);
}

void TestModbusAddress::constructor_copy()
{
    ModbusAddress addrOriginal(40001);
    ModbusAddress addr(addrOriginal);

    QCOMPARE(addr.fullAddress(), "40001");
    QCOMPARE(addr.objectType(), ObjectType::HOLDING_REGISTER);
}

void TestModbusAddress::addressFunctions_data()
{
    QTest::addColumn<ModbusAddress>("modbusAddr");
    QTest::addColumn<QString>("fullAddress");
    QTest::addColumn<quint16>("protocolAddress");

    /*                                                                              Full addr    Protocol address      */
    QTest::newRow("Test 01") << ModbusAddress(0, ObjectType::COIL)                  << "0"      << static_cast<quint16>(0);
    QTest::newRow("Test 02") << ModbusAddress(0, ObjectType::DISCRETE_INPUT)        << "10001"  << static_cast<quint16>(0);
    QTest::newRow("Test 03") << ModbusAddress(0, ObjectType::INPUT_REGISTER)        << "30001"  << static_cast<quint16>(0);
    QTest::newRow("Test 04") << ModbusAddress(0, ObjectType::HOLDING_REGISTER)      << "40001"  << static_cast<quint16>(0);

    /* Last possible values with 5 digits notation */
    QTest::newRow("Test 05") << ModbusAddress(9999, ObjectType::COIL)               << "9999"   << static_cast<quint16>(9999);
    QTest::newRow("Test 06") << ModbusAddress(9999, ObjectType::DISCRETE_INPUT)     << "20000"  << static_cast<quint16>(9999);
    QTest::newRow("Test 07") << ModbusAddress(9999, ObjectType::INPUT_REGISTER)     << "40000"  << static_cast<quint16>(9999);
    QTest::newRow("Test 08") << ModbusAddress(9999, ObjectType::HOLDING_REGISTER)   << "50000"  << static_cast<quint16>(9999);

    /* Last possible address with 5 digits notation */
    QTest::newRow("Test 09") << ModbusAddress(10000, ObjectType::COIL)              << "c10000"     << static_cast<quint16>(10000);
    QTest::newRow("Test 10") << ModbusAddress(10000, ObjectType::DISCRETE_INPUT)    << "d10000"     << static_cast<quint16>(10000);
    QTest::newRow("Test 11") << ModbusAddress(10000, ObjectType::INPUT_REGISTER)    << "i10000"     << static_cast<quint16>(10000);
    QTest::newRow("Test 12") << ModbusAddress(10000, ObjectType::HOLDING_REGISTER)  << "h10000"     << static_cast<quint16>(10000);

    /* Largest possible address */
    QTest::newRow("Test 13") << ModbusAddress(65535, ObjectType::COIL)              << "c65535"     << static_cast<quint16>(65535);
    QTest::newRow("Test 14") << ModbusAddress(65535, ObjectType::DISCRETE_INPUT)    << "d65535"     << static_cast<quint16>(65535);
    QTest::newRow("Test 15") << ModbusAddress(65535, ObjectType::INPUT_REGISTER)    << "i65535"     << static_cast<quint16>(65535);
    QTest::newRow("Test 16") << ModbusAddress(65535, ObjectType::HOLDING_REGISTER)  << "h65535"     << static_cast<quint16>(65535);

}

void TestModbusAddress::addressFunctions()
{
    QFETCH(ModbusAddress, modbusAddr);
    QFETCH(QString, fullAddress);
    QFETCH(quint16, protocolAddress);

    QCOMPARE(modbusAddr.fullAddress(), fullAddress);
    QCOMPARE(modbusAddr.protocolAddress(), protocolAddress);
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
