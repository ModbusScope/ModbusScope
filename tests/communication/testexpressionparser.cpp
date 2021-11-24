
#include <QtTest/QtTest>

#include "expressionparser.h"
#include "settingsmodel.h"
#include "testexpressionparser.h"


void TestExpressionParser::init()
{

}

void TestExpressionParser::cleanup()
{
}

void TestExpressionParser::singleRegister()
{
    auto input = QStringList() << "${45332}";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, SettingsModel::CONNECTION_ID_0, false, true);

    verifyParsing(input, expModbusRegisters);
}

void TestExpressionParser::singleRegisterConn()
{
    auto input = QStringList() << "${45332@2}";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, SettingsModel::CONNECTION_ID_1, false, true);

    verifyParsing(input, expModbusRegisters);
}

void TestExpressionParser::singleRegisterSigned()
{
    auto input = QStringList() << "${45332: s16b}";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, SettingsModel::CONNECTION_ID_0, false, false);

    verifyParsing(input, expModbusRegisters);
}

void TestExpressionParser::singleRegisterSigned32()
{
    auto input = QStringList() << "${45332: s32b}";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, SettingsModel::CONNECTION_ID_0, true, false);

    verifyParsing(input, expModbusRegisters);
}

void TestExpressionParser::singleRegisterConnType()
{
    auto input = QStringList() << "${45332@2: s32b}";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, SettingsModel::CONNECTION_ID_1, true, false);

    verifyParsing(input, expModbusRegisters);
}

void TestExpressionParser::multiRegisters()
{
    auto input = QStringList() << "${45332} + ${45333}" << "${45334}";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, SettingsModel::CONNECTION_ID_0, false, true)
                                                      << ModbusRegister(45333, SettingsModel::CONNECTION_ID_0, false, true)
                                                      << ModbusRegister(45334, SettingsModel::CONNECTION_ID_0, false, true);

    verifyParsing(input, expModbusRegisters);
}

void TestExpressionParser::multiRegistersDuplicate()
{
    auto input = QStringList() << "${45332} + ${45333}" << "${45332}";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, SettingsModel::CONNECTION_ID_0, false, true)
                                                      << ModbusRegister(45333, SettingsModel::CONNECTION_ID_0, false, true);

    verifyParsing(input, expModbusRegisters);
}

void TestExpressionParser::failure()
{
    auto input = QStringList() << "${}";
    auto expModbusRegisters = QList<ModbusRegister>();

    verifyParsing(input, expModbusRegisters);
}

void TestExpressionParser::failureMulti()
{
    auto input = QStringList() << "${}" << "${45331}";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45331, SettingsModel::CONNECTION_ID_0, false, true);

    verifyParsing(input, expModbusRegisters);
}

void TestExpressionParser::combinations()
{
    auto input = QStringList() << "${45332@2: s32b} + ${45330} + 2";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, SettingsModel::CONNECTION_ID_1, true, false)
                                                      << ModbusRegister(45330, SettingsModel::CONNECTION_ID_0, false, true);

    verifyParsing(input, expModbusRegisters);
}

void TestExpressionParser::spaces()
{
    auto input = QStringList() << "${45332   @2: 32b   } + ${  45330  }";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, SettingsModel::CONNECTION_ID_1, true, true)
                                                      << ModbusRegister(45330, SettingsModel::CONNECTION_ID_0, false, true);

    verifyParsing(input, expModbusRegisters);
}

void TestExpressionParser::verifyParsing(QStringList exprList, QList<ModbusRegister> &expectedRegisters)
{
    QList<ModbusRegister> actualModbusRegisters;

    ExpressionParser parser(exprList);
    parser.modbusRegisters(actualModbusRegisters);

    QVERIFY(actualModbusRegisters == expectedRegisters);
}

QTEST_GUILESS_MAIN(TestExpressionParser)
