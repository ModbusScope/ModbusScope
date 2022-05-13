
#include <QtTest/QtTest>

#include "expressionparser.h"
#include "settingsmodel.h"
#include "tst_expressionparser.h"


void TestExpressionParser::init()
{

}

void TestExpressionParser::cleanup()
{
}

void TestExpressionParser::singleRegister()
{
    auto input = QStringList() <<           "${45332}";
    auto expExpressions = QStringList() <<  "r(0    )";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, Connection::ID_1, false, true);


    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::singleRegisterConn()
{
    auto input = QStringList() <<           "${45332@2}";
    auto expExpressions = QStringList() <<  "r(0      )";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, Connection::ID_2, false, true);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::singleRegisterSigned()
{
    auto input = QStringList() <<           "${45332: s16b}";
    auto expExpressions = QStringList() <<  "r(0          )";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, Connection::ID_1, false, false);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::singleRegisterSigned32()
{
    auto input = QStringList() <<           "${45332: s32b}";
    auto expExpressions = QStringList() <<  "r(0          )";

    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, Connection::ID_1, true, false);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::singleRegisterConnType()
{
    auto input = QStringList() <<           "${45332@2: s32b}";
    auto expExpressions = QStringList() <<  "r(0            )";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, Connection::ID_2, true, false);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::multiRegisters()
{
    auto input = QStringList() <<           "${45332} + ${45333}" << "${45334}";
    auto expExpressions = QStringList() <<  "r(0    ) + r(1    )" << "r(2    )";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, Connection::ID_1, false, true)
                                                      << ModbusRegister(45333, Connection::ID_1, false, true)
                                                      << ModbusRegister(45334, Connection::ID_1, false, true);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::multiRegistersDuplicate()
{
    auto input = QStringList() <<           "${45332} + ${45333}" << "${45332}";
    auto expExpressions = QStringList() <<  "r(0    ) + r(1    )" << "r(0    )";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, Connection::ID_1, false, true)
                                                      << ModbusRegister(45333, Connection::ID_1, false, true);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::failure()
{
    auto input = QStringList() << "${}";
    auto expModbusRegisters = QList<ModbusRegister>();
    auto expExpressions = QStringList() << "${}";

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::failureMulti()
{
    auto input = QStringList() <<           "${}" << "${45331}";
    auto expExpressions = QStringList() <<  "${}" << "r(0    )";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45331, Connection::ID_1, false, true);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::combinations()
{
    auto input = QStringList() <<           "${45332@2: s32b} + ${45330} + 2";
    auto expExpressions = QStringList() <<  "r(0            ) + r(1    ) + 2";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, Connection::ID_2, true, false)
                                                      << ModbusRegister(45330, Connection::ID_1, false, true);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::explicitDefaults()
{
    auto input = QStringList() <<           "${40001@1} + ${40002:16b}";
    auto expExpressions = QStringList() <<  "r(0      ) + r(1        )";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_1, false, true)
                                                      << ModbusRegister(40002, Connection::ID_1, false, true);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::spaces()
{
    auto input = QStringList() <<           "${45332   @2: 32b   } + ${  45330  }";
    auto expExpressions = QStringList() <<  "r(0                 ) + r(1        )";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, Connection::ID_2, true, true)
                                                      << ModbusRegister(45330, Connection::ID_1, false, true);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::newlines()
{
    auto input = QStringList() <<           "${45332@2:32b} \n + 1 + \n + ${45330}";
    auto expExpressions = QStringList() <<  "r(0          ) \n + 1 + \n + r(1    )";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(45332, Connection::ID_2, true, true)
                                                      << ModbusRegister(45330, Connection::ID_1, false, true);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::constant()
{
    auto input = QStringList() << "2";
    auto expModbusRegisters = QList<ModbusRegister>();
    auto expExpressions = QStringList() << "2";

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::manyRegisters()
{
    auto input = QStringList() << "${0}" << "${1}"
                                << "${2}" << "${3}"
                                << "${4}" << "${5}"
                                << "${6}" << "${7}"
                                << "${8}" << "${9}"
                                << "${10}" << "${11}";
    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(0, Connection::ID_1, false, true)
                                                        << ModbusRegister(1, Connection::ID_1, false, true)
                                                        << ModbusRegister(2, Connection::ID_1, false, true)
                                                        << ModbusRegister(3, Connection::ID_1, false, true)
                                                        << ModbusRegister(4, Connection::ID_1, false, true)
                                                        << ModbusRegister(5, Connection::ID_1, false, true)
                                                        << ModbusRegister(6, Connection::ID_1, false, true)
                                                        << ModbusRegister(7, Connection::ID_1, false, true)
                                                        << ModbusRegister(8, Connection::ID_1, false, true)
                                                        << ModbusRegister(9, Connection::ID_1, false, true)
                                                        << ModbusRegister(10, Connection::ID_1, false, true)
                                                        << ModbusRegister(11, Connection::ID_1, false, true);
    auto expExpressions = QStringList() << "r(0)" << "r(1)"
                                        << "r(2)" << "r(3)"
                                        << "r(4)" << "r(5)"
                                        << "r(6)" << "r(7)"
                                        << "r(8)" << "r(9)"
                                        << "r(10)" << "r(11)";

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
