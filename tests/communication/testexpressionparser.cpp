
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
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, Connection::ID_1, false, true);
    auto expExpressions = QStringList() << "regval(0)";

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::singleRegisterConn()
{
    auto input = QStringList() << "${45332@2}";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, Connection::ID_2, false, true);
    auto expExpressions = QStringList() << "regval(0)";

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::singleRegisterSigned()
{
    auto input = QStringList() << "${45332: s16b}";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, Connection::ID_1, false, false);
    auto expExpressions = QStringList() << "regval(0)";

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::singleRegisterSigned32()
{
    auto input = QStringList() << "${45332: s32b}";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, Connection::ID_1, true, false);
    auto expExpressions = QStringList() << "regval(0)";

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::singleRegisterConnType()
{
    auto input = QStringList() << "${45332@2: s32b}";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, Connection::ID_2, true, false);
    auto expExpressions = QStringList() << "regval(0)";

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::multiRegisters()
{
    auto input = QStringList() << "${45332} + ${45333}" << "${45334}";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, Connection::ID_1, false, true)
                                                      << ModbusRegister(45333, Connection::ID_1, false, true)
                                                      << ModbusRegister(45334, Connection::ID_1, false, true);
    auto expExpressions = QStringList() << "regval(0)+regval(1)" << "regval(2)";

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::multiRegistersDuplicate()
{
    auto input = QStringList() << "${45332} + ${45333}" << "${45332}";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, Connection::ID_1, false, true)
                                                      << ModbusRegister(45333, Connection::ID_1, false, true);
    auto expExpressions = QStringList() << "regval(0)+regval(1)" << "regval(0)";

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::failure()
{
    auto input = QStringList() << "${}";
    auto expModbusRegisters = QList<ModbusRegister>();
    auto expExpressions = QStringList() << "";

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::failureMulti()
{
    auto input = QStringList() << "${}" << "${45331}";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45331, Connection::ID_1, false, true);
    auto expExpressions = QStringList() << "" << "regval(0)";

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::combinations()
{
    auto input = QStringList() << "${45332@2: s32b} + ${45330} + 2";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, Connection::ID_2, true, false)
                                                      << ModbusRegister(45330, Connection::ID_1, false, true);
    auto expExpressions = QStringList() << "regval(0)+regval(1)+2";

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::explicitDefaults()
{
    auto input = QStringList() << "${40001@1} + ${40002:16b}";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, false, true)
                                                      << ModbusRegister(40002, Connection::ID_1, false, true);
    auto expExpressions = QStringList() << "regval(0)+regval(1)";

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::spaces()
{
    auto input = QStringList() << "${45332   @2: 32b   } + ${  45330  }";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, Connection::ID_2, true, true)
                                                      << ModbusRegister(45330, Connection::ID_1, false, true);
    auto expExpressions = QStringList() << "regval(0)+regval(1)";

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::constant()
{
    auto input = QStringList() << "2";
    auto expModbusRegisters = QList<ModbusRegister>();
    auto expExpressions = QStringList() << "2";

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::verifyParsing(QStringList exprList, QList<ModbusRegister> &expectedRegisters, QStringList &expectedExpression)
{
    QList<ModbusRegister> actualModbusRegisters;
    QStringList actualExpressionList;

    ExpressionParser parser(exprList);
    parser.modbusRegisters(actualModbusRegisters);
    parser.processedExpressions(actualExpressionList);

    QVERIFY(actualModbusRegisters == expectedRegisters);
    QVERIFY(actualExpressionList == expectedExpression);
}

QTEST_GUILESS_MAIN(TestExpressionParser)
