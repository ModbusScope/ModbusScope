
#include "tst_expressionparser.h"

#include "datahandling/expressionparser.h"

#include <QTest>

void TestExpressionParser::init()
{
}

void TestExpressionParser::cleanup()
{
}

void TestExpressionParser::singleRegister()
{
    auto input = QStringList() << "${45332}";
    auto expExpressions = QStringList() << "r(0    )";
    auto expDataPoints = QList<DataPoint>() << DataPoint("${45332}", Device::cFirstDeviceId);

    verifyParsing(input, expDataPoints, expExpressions);
}

void TestExpressionParser::singleRegisterConn()
{
    auto input = QStringList() << "${45332@2}";
    auto expExpressions = QStringList() << "r(0      )";
    auto expDataPoints = QList<DataPoint>() << DataPoint("${45332@2}", Device::cFirstDeviceId + 1);

    verifyParsing(input, expDataPoints, expExpressions);
}

void TestExpressionParser::singleRegisterSigned()
{
    auto input = QStringList() << "${45332: s16b}";
    auto expExpressions = QStringList() << "r(0          )";
    auto expDataPoints = QList<DataPoint>() << DataPoint("${45332: s16b}", Device::cFirstDeviceId);

    verifyParsing(input, expDataPoints, expExpressions);
}

void TestExpressionParser::singleRegisterSigned32()
{
    auto input = QStringList() << "${45332: s32b}";
    auto expExpressions = QStringList() << "r(0          )";

    auto expDataPoints = QList<DataPoint>() << DataPoint("${45332: s32b}", Device::cFirstDeviceId);

    verifyParsing(input, expDataPoints, expExpressions);
}

void TestExpressionParser::singleRegisterFloat32()
{
    auto input = QStringList() << "${45332: f32b}";
    auto expExpressions = QStringList() << "r(0          )";

    auto expDataPoints = QList<DataPoint>() << DataPoint("${45332: f32b}", Device::cFirstDeviceId);

    verifyParsing(input, expDataPoints, expExpressions);
}

void TestExpressionParser::singleRegisterExplicitCoil()
{
    auto input = QStringList() << "${c50000} + ${c100@2}";
    auto expExpressions = QStringList() << "r(0     ) + r(1     )";

    auto expDataPoints = QList<DataPoint>() << DataPoint("${c50000}", Device::cFirstDeviceId)
                                            << DataPoint("${c100@2}", Device::cFirstDeviceId + 1);

    verifyParsing(input, expDataPoints, expExpressions);
}

void TestExpressionParser::singleRegisterExplicitDiscreteInput()
{
    auto input = QStringList() << "${d50000@2} + ${d100}";
    auto expExpressions = QStringList() << "r(0       ) + r(1   )";

    auto expDataPoints = QList<DataPoint>() << DataPoint("${d50000@2}", Device::cFirstDeviceId + 1)
                                            << DataPoint("${d100}", Device::cFirstDeviceId);

    verifyParsing(input, expDataPoints, expExpressions);
}

void TestExpressionParser::singleRegisterExplicitHolding()
{
    auto input = QStringList() << "${h50000: f32b} + ${h100@2}";
    auto expExpressions = QStringList() << "r(0           ) + r(1     )";

    auto expDataPoints = QList<DataPoint>() << DataPoint("${h50000: f32b}", Device::cFirstDeviceId)
                                            << DataPoint("${h100@2}", Device::cFirstDeviceId + 1);

    verifyParsing(input, expDataPoints, expExpressions);
}

void TestExpressionParser::singleRegisterExplicitInput()
{
    auto input = QStringList() << "${i50000@2} + ${i100}";
    auto expExpressions = QStringList() << "r(0       ) + r(1   )";

    auto expDataPoints = QList<DataPoint>() << DataPoint("${i50000@2}", Device::cFirstDeviceId + 1)
                                            << DataPoint("${i100}", Device::cFirstDeviceId);

    verifyParsing(input, expDataPoints, expExpressions);
}

void TestExpressionParser::singleRegisterConnType()
{
    auto input = QStringList() << "${45332@2: s32b}";
    auto expExpressions = QStringList() << "r(0            )";
    auto expDataPoints = QList<DataPoint>() << DataPoint("${45332@2: s32b}", Device::cFirstDeviceId + 1);

    verifyParsing(input, expDataPoints, expExpressions);
}

void TestExpressionParser::multiRegisters()
{
    auto input = QStringList() << "${45332} + ${45333}" << "${45334}";
    auto expExpressions = QStringList() << "r(0    ) + r(1    )" << "r(2    )";
    auto expDataPoints = QList<DataPoint>() << DataPoint("${45332}", Device::cFirstDeviceId)
                                            << DataPoint("${45333}", Device::cFirstDeviceId)
                                            << DataPoint("${45334}", Device::cFirstDeviceId);

    verifyParsing(input, expDataPoints, expExpressions);
}

void TestExpressionParser::multiRegistersDuplicate()
{
    auto input = QStringList() << "${45332} + ${45333}" << "${45332}";
    auto expExpressions = QStringList() << "r(0    ) + r(1    )" << "r(0    )";
    auto expDataPoints = QList<DataPoint>() << DataPoint("${45332}", Device::cFirstDeviceId)
                                            << DataPoint("${45333}", Device::cFirstDeviceId);

    verifyParsing(input, expDataPoints, expExpressions);
}

void TestExpressionParser::singleExpressionDuplicate()
{
    auto input = QStringList() << "${45332} + ${45332}";
    auto expExpressions = QStringList() << "r(0    ) + r(0    )";
    auto expDataPoints = QList<DataPoint>() << DataPoint("${45332}", Device::cFirstDeviceId);

    verifyParsing(input, expDataPoints, expExpressions);
}

void TestExpressionParser::failure()
{
    auto input = QStringList() << "${}";
    auto expDataPoints = QList<DataPoint>();
    auto expExpressions = QStringList() << "${}";

    verifyParsing(input, expDataPoints, expExpressions);
}

void TestExpressionParser::failureMulti()
{
    auto input = QStringList() << "${}" << "${45331}";
    auto expExpressions = QStringList() << "${}" << "r(0    )";
    auto expDataPoints = QList<DataPoint>() << DataPoint("${45331}", Device::cFirstDeviceId);

    verifyParsing(input, expDataPoints, expExpressions);
}

void TestExpressionParser::combinations()
{
    auto input = QStringList() << "${45332@2: s32b} + ${45330} + 2";
    auto expExpressions = QStringList() << "r(0            ) + r(1    ) + 2";
    auto expDataPoints = QList<DataPoint>() << DataPoint("${45332@2: s32b}", Device::cFirstDeviceId + 1)
                                            << DataPoint("${45330}", Device::cFirstDeviceId);

    verifyParsing(input, expDataPoints, expExpressions);
}

void TestExpressionParser::explicitDefaults()
{
    auto input = QStringList() << "${40001@1} + ${40002:16b}";
    auto expExpressions = QStringList() << "r(0      ) + r(1        )";
    auto expDataPoints = QList<DataPoint>() << DataPoint("${40001@1}", Device::cFirstDeviceId)
                                            << DataPoint("${40002:16b}", Device::cFirstDeviceId);

    verifyParsing(input, expDataPoints, expExpressions);
}

void TestExpressionParser::sameRegisterDifferentType()
{
    auto input = QStringList() << "${40001@1} + ${40001:s16b}";
    auto expExpressions = QStringList() << "r(0      ) + r(1         )";
    auto expDataPoints = QList<DataPoint>() << DataPoint("${40001@1}", Device::cFirstDeviceId)
                                            << DataPoint("${40001:s16b}", Device::cFirstDeviceId);

    verifyParsing(input, expDataPoints, expExpressions);
}

void TestExpressionParser::spaces()
{
    auto input = QStringList() << "${45332   @2: 32b   } + ${  45330  }";
    auto expExpressions = QStringList() << "r(0                 ) + r(1        )";
    auto expDataPoints = QList<DataPoint>() << DataPoint("${45332   @2: 32b   }", Device::cFirstDeviceId + 1)
                                            << DataPoint("${  45330  }", Device::cFirstDeviceId);

    verifyParsing(input, expDataPoints, expExpressions);
}

void TestExpressionParser::newlines()
{
    auto input = QStringList() << "${45332@2:32b} \n + 1 + \n + ${45330}";
    auto expExpressions = QStringList() << "r(0          ) \n + 1 + \n + r(1    )";
    auto expDataPoints = QList<DataPoint>() << DataPoint("${45332@2:32b}", Device::cFirstDeviceId + 1)
                                            << DataPoint("${45330}", Device::cFirstDeviceId);

    verifyParsing(input, expDataPoints, expExpressions);
}

void TestExpressionParser::constant()
{
    auto input = QStringList() << "2";
    auto expDataPoints = QList<DataPoint>();
    auto expExpressions = QStringList() << "2";

    verifyParsing(input, expDataPoints, expExpressions);
}

void TestExpressionParser::manyRegisters()
{
    auto input = QStringList() << "${0}" << "${1}"
                               << "${2}" << "${3}"
                               << "${4}" << "${5}"
                               << "${6}" << "${7}"
                               << "${8}" << "${9}"
                               << "${10}" << "${11}";
    auto expDataPoints = QList<DataPoint>()
                         << DataPoint("${0}", Device::cFirstDeviceId) << DataPoint("${1}", Device::cFirstDeviceId)
                         << DataPoint("${2}", Device::cFirstDeviceId) << DataPoint("${3}", Device::cFirstDeviceId)
                         << DataPoint("${4}", Device::cFirstDeviceId) << DataPoint("${5}", Device::cFirstDeviceId)
                         << DataPoint("${6}", Device::cFirstDeviceId) << DataPoint("${7}", Device::cFirstDeviceId)
                         << DataPoint("${8}", Device::cFirstDeviceId) << DataPoint("${9}", Device::cFirstDeviceId)
                         << DataPoint("${10}", Device::cFirstDeviceId) << DataPoint("${11}", Device::cFirstDeviceId);
    auto expExpressions = QStringList() << "r(0)" << "r(1)"
                                        << "r(2)" << "r(3)"
                                        << "r(4)" << "r(5)"
                                        << "r(6)" << "r(7)"
                                        << "r(8)" << "r(9)"
                                        << "r(10)" << "r(11)";

    verifyParsing(input, expDataPoints, expExpressions);
}

void TestExpressionParser::manyRegistersHighIndex()
{
    /* Verify qMax(0,...) guard: ${0} is 4 chars, but assigned index 10 (after 10 prior registers).
     * Unguarded: spacesCount = 4-3-2 = -1. Guarded: clamped to 0, producing r(10) instead of garbage. */
    auto input = QStringList() << "${1}" << "${2}" << "${3}" << "${4}" << "${5}"
                               << "${6}" << "${7}" << "${8}" << "${9}" << "${10}" << "${0}";
    auto expDataPoints = QList<DataPoint>()
                         << DataPoint("${1}", Device::cFirstDeviceId) << DataPoint("${2}", Device::cFirstDeviceId)
                         << DataPoint("${3}", Device::cFirstDeviceId) << DataPoint("${4}", Device::cFirstDeviceId)
                         << DataPoint("${5}", Device::cFirstDeviceId) << DataPoint("${6}", Device::cFirstDeviceId)
                         << DataPoint("${7}", Device::cFirstDeviceId) << DataPoint("${8}", Device::cFirstDeviceId)
                         << DataPoint("${9}", Device::cFirstDeviceId) << DataPoint("${10}", Device::cFirstDeviceId)
                         << DataPoint("${0}", Device::cFirstDeviceId);
    /* ${10} at idx=9: size=5, spacesCount=5-3-1=1 → r(9 )
     * ${0}  at idx=10: size=4, spacesCount=qMax(0,4-3-2)=0 → r(10) */
    auto expExpressions = QStringList() << "r(0)" << "r(1)" << "r(2)" << "r(3)" << "r(4)"
                                        << "r(5)" << "r(6)" << "r(7)" << "r(8)" << "r(9 )" << "r(10)";

    verifyParsing(input, expDataPoints, expExpressions);
}

void TestExpressionParser::verifyParsing(const QStringList& exprList,
                                         const QList<DataPoint>& expectedDataPoints,
                                         const QStringList& expectedExpression)
{
    ExpressionParser parser(exprList);
    const QList<DataPoint> actualDataPoints = parser.dataPoints();
    const QStringList actualExpressionList = parser.processedExpressions();

    QVERIFY(actualDataPoints == expectedDataPoints);
    QVERIFY(actualExpressionList == expectedExpression);
}

QTEST_GUILESS_MAIN(TestExpressionParser)
