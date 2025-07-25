
#include "tst_expressionparser.h"

#include "models/connectiontypes.h"
#include "util/expressionparser.h"

#include <QTest>

using Type = ModbusDataType::Type;

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
    auto expModbusRegisters = QList<ModbusRegister>()
                              << ModbusRegister(ModbusAddress(45332), ConnectionId::ID_1, Type::UNSIGNED_16);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::singleRegisterConn()
{
    auto input = QStringList() <<           "${45332@2}";
    auto expExpressions = QStringList() <<  "r(0      )";
    auto expModbusRegisters = QList<ModbusRegister>()
                              << ModbusRegister(ModbusAddress(45332), ConnectionId::ID_2, Type::UNSIGNED_16);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::singleRegisterSigned()
{
    auto input = QStringList() <<           "${45332: s16b}";
    auto expExpressions = QStringList() <<  "r(0          )";
    auto expModbusRegisters = QList<ModbusRegister>()
                              << ModbusRegister(ModbusAddress(45332), ConnectionId::ID_1, Type::SIGNED_16);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::singleRegisterSigned32()
{
    auto input = QStringList() <<           "${45332: s32b}";
    auto expExpressions = QStringList() <<  "r(0          )";

    auto expModbusRegisters = QList<ModbusRegister>()
                              << ModbusRegister(ModbusAddress(45332), ConnectionId::ID_1, Type::SIGNED_32);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::singleRegisterFloat32()
{
    auto input = QStringList() <<           "${45332: f32b}";
    auto expExpressions = QStringList() <<  "r(0          )";

    auto expModbusRegisters = QList<ModbusRegister>()
                              << ModbusRegister(ModbusAddress(45332), ConnectionId::ID_1, Type::FLOAT_32);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::singleRegisterExplicitCoil()
{
    auto input = QStringList() <<           "${c50000} + ${c100@2}";
    auto expExpressions = QStringList() <<  "r(0     ) + r(1     )";

    auto expModbusRegisters = QList<ModbusRegister>()
                              << ModbusRegister(ModbusAddress("c50000"), ConnectionId::ID_1, Type::UNSIGNED_16)
                              << ModbusRegister(ModbusAddress("c100"), ConnectionId::ID_2, Type::UNSIGNED_16);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::singleRegisterExplicitDiscreteInput()
{
    auto input = QStringList() <<           "${d50000@2} + ${d100}";
    auto expExpressions = QStringList() <<  "r(0       ) + r(1   )";

    auto expModbusRegisters = QList<ModbusRegister>()
                              << ModbusRegister(ModbusAddress("d50000"), ConnectionId::ID_2, Type::UNSIGNED_16)
                              << ModbusRegister(ModbusAddress("d100"), ConnectionId::ID_1, Type::UNSIGNED_16);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::singleRegisterExplicitHolding()
{
    auto input = QStringList() <<           "${h50000: f32b} + ${h100@2}";
    auto expExpressions = QStringList() <<  "r(0           ) + r(1     )";

    auto expModbusRegisters = QList<ModbusRegister>()
                              << ModbusRegister(ModbusAddress("h50000"), ConnectionId::ID_1, Type::FLOAT_32)
                              << ModbusRegister(ModbusAddress("h100"), ConnectionId::ID_2, Type::UNSIGNED_16);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::singleRegisterExplicitInput()
{
    auto input = QStringList() <<           "${i50000@2} + ${i100}";
    auto expExpressions = QStringList() <<  "r(0       ) + r(1   )";

    auto expModbusRegisters = QList<ModbusRegister>()
                              << ModbusRegister(ModbusAddress("i50000"), ConnectionId::ID_2, Type::UNSIGNED_16)
                              << ModbusRegister(ModbusAddress("i100"), ConnectionId::ID_1, Type::UNSIGNED_16);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::singleRegisterConnType()
{
    auto input = QStringList() <<           "${45332@2: s32b}";
    auto expExpressions = QStringList() <<  "r(0            )";
    auto expModbusRegisters = QList<ModbusRegister>()
                              << ModbusRegister(ModbusAddress(45332), ConnectionId::ID_2, Type::SIGNED_32);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::multiRegisters()
{
    auto input = QStringList() <<           "${45332} + ${45333}" << "${45334}";
    auto expExpressions = QStringList() <<  "r(0    ) + r(1    )" << "r(2    )";
    auto expModbusRegisters = QList<ModbusRegister>()
                              << ModbusRegister(ModbusAddress(45332), ConnectionId::ID_1, Type::UNSIGNED_16)
                              << ModbusRegister(ModbusAddress(45333), ConnectionId::ID_1, Type::UNSIGNED_16)
                              << ModbusRegister(ModbusAddress(45334), ConnectionId::ID_1, Type::UNSIGNED_16);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::multiRegistersDuplicate()
{
    auto input = QStringList() <<           "${45332} + ${45333}" << "${45332}";
    auto expExpressions = QStringList() <<  "r(0    ) + r(1    )" << "r(0    )";
    auto expModbusRegisters = QList<ModbusRegister>()
                              << ModbusRegister(ModbusAddress(45332), ConnectionId::ID_1, Type::UNSIGNED_16)
                              << ModbusRegister(ModbusAddress(45333), ConnectionId::ID_1, Type::UNSIGNED_16);

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
    auto expModbusRegisters = QList<ModbusRegister>()
                              << ModbusRegister(ModbusAddress(45331), ConnectionId::ID_1, Type::UNSIGNED_16);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::combinations()
{
    auto input = QStringList() <<           "${45332@2: s32b} + ${45330} + 2";
    auto expExpressions = QStringList() <<  "r(0            ) + r(1    ) + 2";
    auto expModbusRegisters = QList<ModbusRegister>()
                              << ModbusRegister(ModbusAddress(45332), ConnectionId::ID_2, Type::SIGNED_32)
                              << ModbusRegister(ModbusAddress(45330), ConnectionId::ID_1, Type::UNSIGNED_16);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::explicitDefaults()
{
    auto input = QStringList() <<           "${40001@1} + ${40002:16b}";
    auto expExpressions = QStringList() <<  "r(0      ) + r(1        )";
    auto expModbusRegisters = QList<ModbusRegister>()
                              << ModbusRegister(ModbusAddress(40001), ConnectionId::ID_1, Type::UNSIGNED_16)
                              << ModbusRegister(ModbusAddress(40002), ConnectionId::ID_1, Type::UNSIGNED_16);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::sameRegisterDifferentType()
{
    auto input = QStringList() <<           "${40001@1} + ${40001:s16b}";
    auto expExpressions = QStringList() <<  "r(0      ) + r(1         )";
    auto expModbusRegisters = QList<ModbusRegister>()
                              << ModbusRegister(ModbusAddress(40001), ConnectionId::ID_1, Type::UNSIGNED_16)
                              << ModbusRegister(ModbusAddress(40001), ConnectionId::ID_1, Type::SIGNED_16);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::spaces()
{
    auto input = QStringList() <<           "${45332   @2: 32b   } + ${  45330  }";
    auto expExpressions = QStringList() <<  "r(0                 ) + r(1        )";
    auto expModbusRegisters = QList<ModbusRegister>()
                              << ModbusRegister(ModbusAddress(45332), ConnectionId::ID_2, Type::UNSIGNED_32)
                              << ModbusRegister(ModbusAddress(45330), ConnectionId::ID_1, Type::UNSIGNED_16);

    verifyParsing(input, expModbusRegisters, expExpressions);
}

void TestExpressionParser::newlines()
{
    auto input = QStringList() <<           "${45332@2:32b} \n + 1 + \n + ${45330}";
    auto expExpressions = QStringList() <<  "r(0          ) \n + 1 + \n + r(1    )";
    auto expModbusRegisters = QList<ModbusRegister>()
                              << ModbusRegister(ModbusAddress(45332), ConnectionId::ID_2, Type::UNSIGNED_32)
                              << ModbusRegister(ModbusAddress(45330), ConnectionId::ID_1, Type::UNSIGNED_16);

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
    auto expModbusRegisters = QList<ModbusRegister>()
                              << ModbusRegister(ModbusAddress(0), ConnectionId::ID_1, Type::UNSIGNED_16)
                              << ModbusRegister(ModbusAddress(1), ConnectionId::ID_1, Type::UNSIGNED_16)
                              << ModbusRegister(ModbusAddress(2), ConnectionId::ID_1, Type::UNSIGNED_16)
                              << ModbusRegister(ModbusAddress(3), ConnectionId::ID_1, Type::UNSIGNED_16)
                              << ModbusRegister(ModbusAddress(4), ConnectionId::ID_1, Type::UNSIGNED_16)
                              << ModbusRegister(ModbusAddress(5), ConnectionId::ID_1, Type::UNSIGNED_16)
                              << ModbusRegister(ModbusAddress(6), ConnectionId::ID_1, Type::UNSIGNED_16)
                              << ModbusRegister(ModbusAddress(7), ConnectionId::ID_1, Type::UNSIGNED_16)
                              << ModbusRegister(ModbusAddress(8), ConnectionId::ID_1, Type::UNSIGNED_16)
                              << ModbusRegister(ModbusAddress(9), ConnectionId::ID_1, Type::UNSIGNED_16)
                              << ModbusRegister(ModbusAddress(10), ConnectionId::ID_1, Type::UNSIGNED_16)
                              << ModbusRegister(ModbusAddress(11), ConnectionId::ID_1, Type::UNSIGNED_16);
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
