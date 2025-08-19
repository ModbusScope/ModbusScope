
#include <QTest>

#include "tst_modbusregister.h"

#include "communication/modbusregister.h"

using Type = ModbusDataType::Type;

#define ADD_TEST_16(name, registerValue, value)      QTest::newRow(name) << static_cast<uint16_t>(registerValue)       \
                                                                        << static_cast<double>(value)

#define ADD_TEST_32(name, upperRegister, lowerRegister, value)      QTest::newRow(name) << static_cast<uint16_t>(upperRegister)       \
                                                                                     << static_cast<uint16_t>(lowerRegister)       \
                                                                                     << static_cast<double>(value)

void TestModbusRegister::init()
{

}

void TestModbusRegister::cleanup()
{

}

void TestModbusRegister::constructor()
{
    ModbusRegister reg(ModbusAddress(40001), 2, Type::SIGNED_16);

    QCOMPARE(reg.address(), ModbusAddress(40001));
    QCOMPARE(reg.deviceId(), 2);
    QCOMPARE(reg.type(), Type::SIGNED_16);
}

void TestModbusRegister::comparison()
{
    ModbusRegister reg_1(ModbusAddress(40001), 1, Type::SIGNED_16);
    ModbusRegister reg_2(ModbusAddress(40001), 1, Type::SIGNED_16);

    QVERIFY(reg_1 == reg_2);

    reg_1.setAddress(ModbusAddress(40002));

    QVERIFY(!(reg_1 == reg_2));
}

void TestModbusRegister::copy()
{
    ModbusRegister reg_1(ModbusAddress(40001), 2, Type::SIGNED_32);
    ModbusRegister reg_2;

    QVERIFY(!(reg_1 == reg_2));

    reg_2 = reg_1;

    QCOMPARE(reg_1.address(), ModbusAddress(40001));
    QCOMPARE(reg_1.deviceId(), 2);
    QCOMPARE(reg_1.type(), Type::SIGNED_32);

    QVERIFY(reg_1 == reg_2);
}

void TestModbusRegister::description()
{
    ModbusRegister reg_1(ModbusAddress(40001), Device::cFirstDeviceId, Type::UNSIGNED_16);
    ModbusRegister reg_2(ModbusAddress(40002), Device::cFirstDeviceId + 1, Type::SIGNED_32);
    ModbusRegister reg_3(ModbusAddress(40003), Device::cFirstDeviceId + 1, Type::FLOAT_32);

    QCOMPARE(reg_1.description(), "holding register, 0, slave id 1, unsigned 16-bit, device 1");
    QCOMPARE(reg_2.description(), "holding register, 1, slave id 1, signed 32-bit, device 2");
    QCOMPARE(reg_3.description(), "holding register, 2, slave id 1, 32-bit float, device 2");
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

    ModbusRegister reg(ModbusAddress(40001), 1, Type::UNSIGNED_16);

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

    ModbusRegister reg(ModbusAddress(40001), 1, Type::SIGNED_16);

    double actValue = reg.processValue(registerValue, 0, false);
    QCOMPARE(actValue, expValue);
}

void TestModbusRegister::processValue_32b_data()
{
    QTest::addColumn<uint16_t>("upperRegister");
    QTest::addColumn<uint16_t>("lowerRegister");
    QTest::addColumn<double>("expValue");

    ADD_TEST_32("32b_1", 0, 0, 0);
    ADD_TEST_32("32b_2", 0, 1, 1);
    ADD_TEST_32("32b_3", 1, 0, 65536);
    ADD_TEST_32("32b_4", 0xFFFF, 0xFFFF, UINT32_MAX);
}

void TestModbusRegister::processValue_32b()
{
    QFETCH(uint16_t, upperRegister);
    QFETCH(uint16_t, lowerRegister);

    QFETCH(double, expValue);

    ModbusRegister reg(ModbusAddress(40001), 1, Type::UNSIGNED_32);

    double actValue = reg.processValue(upperRegister, lowerRegister, false);
    QCOMPARE(actValue, expValue);

    double actValue2 = reg.processValue(lowerRegister, upperRegister, true);
    QCOMPARE(actValue2, expValue);
}

void TestModbusRegister::processValue_s32b_data()
{
    QTest::addColumn<uint16_t>("upperRegister");
    QTest::addColumn<uint16_t>("lowerRegister");
    QTest::addColumn<double>("expValue");

    ADD_TEST_32("s32b_1", 0, 0, 0);
    ADD_TEST_32("s32b_2", 0, 1, 1);
    ADD_TEST_32("s32b_3", 1, 0, 65536);
    ADD_TEST_32("s32b_4", 0xFFFF, 0xFFFF, -1);
}

void TestModbusRegister::processValue_s32b()
{
    QFETCH(uint16_t, upperRegister);
    QFETCH(uint16_t, lowerRegister);
    QFETCH(double, expValue);

    ModbusRegister reg(ModbusAddress(40001), 1, Type::SIGNED_32);

    double actValue = reg.processValue(upperRegister, lowerRegister, false);
    QCOMPARE(actValue, expValue);

    double actValue2 = reg.processValue(lowerRegister, upperRegister, true);
    QCOMPARE(actValue2, expValue);
}

void TestModbusRegister::processValue_f32b_data()
{
    QTest::addColumn<uint16_t>("upperRegister");
    QTest::addColumn<uint16_t>("lowerRegister");
    QTest::addColumn<double>("expValue");

    ADD_TEST_32("f32b_1", 0x0000, 0x0001, 1.4012984643e-45f);       // smallest positive subnormal number
    ADD_TEST_32("f32b_2", 0x007f, 0xffff, 1.1754942107e-38f);       // largest subnormal number
    ADD_TEST_32("f32b_3", 0x0080, 0x0000, 1.1754943508e-38f);       // smallest positive normal number
    ADD_TEST_32("f32b_4", 0x7f7f, 0xffff, 3.40282346639e38f);       // largest normal number
    ADD_TEST_32("f32b_5", 0x3f7f, 0xffff, 0.999999940395355225f);   // largest number less than one
    ADD_TEST_32("f32b_6", 0x3f80, 0x0000, 1);
    ADD_TEST_32("f32b_7", 0x3f80, 0x0001, 1.000000119209289505f);   // smallest number larger than one
    ADD_TEST_32("f32b_8", 0xc000, 0x0000, -2);
    ADD_TEST_32("f32b_9", 0x0000, 0x0000, 0);
    ADD_TEST_32("f32b_10", 0x8000, 0x0000, 0);                      // -0
    ADD_TEST_32("f32b_11", 0x7f80, 0x0000, 0);                      // infinity
    ADD_TEST_32("f32b_12", 0xff80, 0x0000, 0);                      // −infinity
    ADD_TEST_32("f32b_13", 0x4049, 0x0fdb, 3.14159274101257324f);   // pi
    ADD_TEST_32("f32b_14", 0x3eaa, 0xaaab, 0.333333343267440796f);  // 1/3
    ADD_TEST_32("f32b_15", 0xffc0, 0x0001, 0);                      // qNaN (on x86 and ARM processors)
    ADD_TEST_32("f32b_16", 0xff80, 0x0001, 0);                      // sNaN (on x86 and ARM processors)
}

void TestModbusRegister::processValue_f32b()
{
    QFETCH(uint16_t, upperRegister);
    QFETCH(uint16_t, lowerRegister);
    QFETCH(double, expValue);

    ModbusRegister reg(ModbusAddress(40001), 1, ModbusDataType::Type::FLOAT_32);

    double actValue = reg.processValue(upperRegister, lowerRegister, false);
    QCOMPARE(actValue, expValue);

    double actValue2 = reg.processValue(lowerRegister, upperRegister, true);
    QCOMPARE(actValue2, expValue);
}

QTEST_GUILESS_MAIN(TestModbusRegister)
