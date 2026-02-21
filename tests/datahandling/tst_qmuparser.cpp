
#include "tst_qmuparser.h"

#include "datahandling/qmuparser.h"

#include <QTest>

using State = ResultState::State;

#define ADD_TEST(expr, result) QTest::newRow(expr) << QString(expr) << static_cast<double>(result)

#define ADD_REG_TEST(expr, registerValue, result)                                                                      \
    QTest::newRow(expr) << QString(expr) << static_cast<double>(registerValue) << static_cast<double>(result)

void TestQMuParser::init()
{
}

void TestQMuParser::cleanup()
{
}

void TestQMuParser::evaluate_data()
{
    QTest::addColumn<QString>("expression");
    QTest::addColumn<double>("result");

    /* binary */
    ADD_TEST("1 | 2", 3);
    ADD_TEST("123 & 456", 72);
    ADD_TEST("1 << 3", 8);
    ADD_TEST("8 >> 3", 1);

    /* Arithmetic */
    ADD_TEST("16 / 4", 4);
    ADD_TEST("9 * 4", 36);
    ADD_TEST("9 % 4", 1);
    ADD_TEST("-10+1", -9);
    ADD_TEST("1+2*3", 7);
    ADD_TEST("(1+2)*3", 9);
    ADD_TEST("3+-3^2", -6);
    ADD_TEST("2^2^3", 256);
    ADD_TEST("-2^2", -4);
    ADD_TEST("(-2)^2", 4);

    /* Hexadecimal */
    ADD_TEST("0xff", 255);
    ADD_TEST("0xffFF", 65535);
    ADD_TEST("0xFFFFffff", 4294967295);
    ADD_TEST("10+0xff", 265);
    ADD_TEST("0xff+10", 265);
    ADD_TEST("10*0xff", 2550);
    ADD_TEST("0xff*10", 2550);
    ADD_TEST("10+0xff+1", 266);
    ADD_TEST("1+0xff+10", 266);
    ADD_TEST("0x12345678", 305419896);
    ADD_TEST("0xABCDEF", 11259375);

    /* Binary */
    ADD_TEST("0b11111111", 255);
    ADD_TEST("0b11111111111111111111111111111111", 4294967295);
    ADD_TEST("0b000001", 1);
    ADD_TEST("0b00", 0);
    ADD_TEST("0b01 + 1 + 0x01", 3);
    ADD_TEST("0b1 << 3", 8);

    /* Floating point result */
    ADD_TEST("15 / 4", 3.75);
    ADD_TEST("1 / 1000", 0.001);
    ADD_TEST("15.6 + 5.5", 21.1);
    ADD_TEST("15.6 - 5.5", 10.1);
    ADD_TEST("3.33 * 3", 9.99);

    /* Floating point with binary/integer operations: fraction part is removed and ignored */
    ADD_TEST("15.5 | 0xF", 15);
    ADD_TEST("16.5 & 0x10", 16);
    ADD_TEST("1.3 << 3", 8);
    ADD_TEST("8.6 >> 3", 1);

    ADD_TEST("9.5 % 4", 1);

    /* Floating point result */
    ADD_TEST("1.5 + 1.5", 3);
    ADD_TEST("1,5 + 1", 2.5);
    ADD_TEST("1.5 + 1", 2.5);
    ADD_TEST("1,5 + 1,5", 3);
}

void TestQMuParser::evaluate()
{
    QFETCH(QString, expression);
    QFETCH(double, result);

    QMuParser parser(expression);

    bool bSuccess = parser.evaluate();

    QCOMPARE(parser.value(), result);
    QVERIFY(parser.isSuccess());
    QVERIFY(bSuccess);
    QCOMPARE(parser.errorType(), QMuParser::ErrorType::NONE);
}

void TestQMuParser::evaluateSingleRegister_data()
{
    QTest::addColumn<QString>("expression");
    QTest::addColumn<double>("registerValue");
    QTest::addColumn<double>("result");

    ADD_REG_TEST("r(0)", 2, 2);
    ADD_REG_TEST("r(0) + 2", 3, 5);
    ADD_REG_TEST("r(0) * 2", 4, 8);
    ADD_REG_TEST("r(0) * 4", 4.5, 18);
    ADD_REG_TEST("r(0) / 1000", 5, 0.005);
    ADD_REG_TEST("r(0) & 0xFF", 257, 1);

    ADD_REG_TEST("r(0)\n+r(0)", 2, 4);
}

void TestQMuParser::evaluateSingleRegister()
{
    QFETCH(QString, expression);
    QFETCH(double, registerValue);
    QFETCH(double, result);

    QMuParser parser(expression);

    parser.setRegistersData(ResultDoubleList() << ResultDouble(registerValue, State::SUCCESS));

    bool bSuccess = parser.evaluate();

    QCOMPARE(parser.value(), result);
    QVERIFY(parser.isSuccess());
    QVERIFY(bSuccess);
    QCOMPARE(parser.errorType(), QMuParser::ErrorType::NONE);
}

void TestQMuParser::evaluateMultipleRegisters()
{
    auto input = ResultDoubleList() << ResultDouble(1, State::SUCCESS) << ResultDouble(2, State::SUCCESS)
                                    << ResultDouble(3, State::SUCCESS);

    QMuParser parser("r(0)");
    parser.setRegistersData(input);

    bool bSuccess = parser.evaluate();

    QCOMPARE(parser.value(), 1);
    QVERIFY(parser.isSuccess());
    QVERIFY(bSuccess);
    QCOMPARE(parser.errorType(), QMuParser::ErrorType::NONE);

    parser.setExpression("r(1)");
    bSuccess = parser.evaluate();

    QCOMPARE(parser.value(), 2);
    QVERIFY(parser.isSuccess());
    QVERIFY(bSuccess);
    QCOMPARE(parser.errorType(), QMuParser::ErrorType::NONE);

    parser.setExpression("r(2)");
    bSuccess = parser.evaluate();

    QCOMPARE(parser.value(), 3);
    QVERIFY(parser.isSuccess());
    QVERIFY(bSuccess);
    QCOMPARE(parser.errorType(), QMuParser::ErrorType::NONE);
}

void TestQMuParser::evaluateSubsequentRegister()
{
    const int count = 10;
    QList<double> data;
    for (int idx = 0; idx < count; idx++)
    {
        data.append(idx);
    }

    QMuParser parser("r(0)");

    for (int idx = 0; idx < count; idx++)
    {
        auto input = ResultDoubleList() << ResultDouble(data[idx], State::SUCCESS);
        parser.setRegistersData(input);

        bool bSuccess = parser.evaluate();
        QCOMPARE(parser.value(), data[idx]);
        QVERIFY(parser.isSuccess());
        QVERIFY(bSuccess);
        QCOMPARE(parser.errorType(), QMuParser::ErrorType::NONE);
    }
}

void TestQMuParser::evaluateInvalidExpr()
{
    QMuParser parser("x11");

    bool bSuccess = parser.evaluate();

    QCOMPARE(parser.value(), 0);
    QVERIFY(!parser.isSuccess());
    QVERIFY(!bSuccess);
    QCOMPARE(parser.errorPos(), 0);
    QCOMPARE(parser.errorType(), QMuParser::ErrorType::SYNTAX);
}

void TestQMuParser::evaluateEmpty()
{
    QMuParser parser("");

    bool bSuccess = parser.evaluate();

    QCOMPARE(parser.value(), 0);
    QVERIFY(!parser.isSuccess());
    QVERIFY(!bSuccess);
    QCOMPARE(parser.errorPos(), -1);
    QCOMPARE(parser.errorType(), QMuParser::ErrorType::SYNTAX);
}

void TestQMuParser::evaluateFail()
{
    QString expression = "r(0)";
    auto resultList = ResultDoubleList() << ResultDouble(5, State::INVALID);

    QMuParser parser(expression);

    parser.setRegistersData(resultList);

    bool bSuccess = parser.evaluate();

    QCOMPARE(parser.value(), 0);
    QVERIFY(!parser.isSuccess());
    QVERIFY(!bSuccess);
    QCOMPARE(parser.errorPos(), -1);
    QCOMPARE(parser.errorType(), QMuParser::ErrorType::OTHER);
}

void TestQMuParser::evaluateMuParserException()
{
    QMuParser parser("-");
    bool bSuccess = parser.evaluate();

    QCOMPARE(parser.value(), 0);
    QVERIFY(!parser.isSuccess());
    QVERIFY(!bSuccess);
    QCOMPARE(parser.errorPos(), 2);
    QCOMPARE(parser.errorType(), QMuParser::ErrorType::SYNTAX);
}

void TestQMuParser::evaluateInvalidHexExpr()
{
    QMuParser parser("0x");

    bool bSuccess = parser.evaluate();

    QCOMPARE(parser.value(), 0);
    QVERIFY(!parser.isSuccess());
    QVERIFY(!bSuccess);
    QCOMPARE(parser.errorPos(), 1);
    QCOMPARE(parser.errorType(), QMuParser::ErrorType::SYNTAX);
}

void TestQMuParser::evaluateInvalidBinExpr()
{
    QMuParser parser("0b");

    bool bSuccess = parser.evaluate();

    QCOMPARE(parser.value(), 0);
    QVERIFY(!parser.isSuccess());
    QVERIFY(!bSuccess);
    QCOMPARE(parser.errorPos(), 1);
    QCOMPARE(parser.errorType(), QMuParser::ErrorType::SYNTAX);
}

void TestQMuParser::evaluateInvalidBinExpr_2()
{
    /* 33 bits */
    QMuParser parser("0b111101110111011101110111011101110");

    bool bSuccess = parser.evaluate();

    QCOMPARE(parser.value(), 0);
    QVERIFY(!parser.isSuccess());
    QVERIFY(!bSuccess);
    QCOMPARE(parser.errorPos(), 34);
    QCOMPARE(parser.errorType(), QMuParser::ErrorType::SYNTAX);
}

void TestQMuParser::evaluateInvalidDecimal()
{
    QMuParser parser("1.1 + 1,5");

    bool bSuccess = parser.evaluate();

    QCOMPARE(parser.value(), 0);
    QVERIFY(!parser.isSuccess());
    QVERIFY(!bSuccess);
    QCOMPARE(parser.errorPos(), -1);
    QCOMPARE(parser.errorType(), QMuParser::ErrorType::NONE);
}

void TestQMuParser::evaluateDivByZero()
{
    QMuParser parser("1/0");

    bool bSuccess = parser.evaluate();

    QCOMPARE(parser.value(), 0);
    QVERIFY(!parser.isSuccess());
    QVERIFY(!bSuccess);
    QCOMPARE(parser.errorPos(), -1);
    QCOMPARE(parser.errorType(), QMuParser::ErrorType::OTHER);
}

void TestQMuParser::evaluateDecimalSeparatorCombination()
{
    /* Test for #165 (locale is static) */
    /* Bug was that last expression without decimal separator would set
     * separator to default comma and let expression with point fail */
    QMuParser parser_1("1.5");
    QMuParser parser_2("5");

    bool bSuccess = parser_1.evaluate();

    QVERIFY(bSuccess);
    QCOMPARE(parser_1.value(), 1.5);
}

void TestQMuParser::expressionUpdate()
{
    QMuParser parser("r(0) + 1");

    auto input_1 = ResultDoubleList() << ResultDouble(5, State::SUCCESS);
    parser.setRegistersData(input_1);

    bool bSuccess = parser.evaluate();

    QCOMPARE(parser.value(), 6);
    QVERIFY(parser.isSuccess());
    QVERIFY(bSuccess);
    QCOMPARE(parser.errorType(), QMuParser::ErrorType::NONE);

    parser.setExpression("r(0) + r(1) + 2");

    auto input_2 = ResultDoubleList() << ResultDouble(1, State::SUCCESS) << ResultDouble(2, State::SUCCESS);
    parser.setRegistersData(input_2);
    bSuccess = parser.evaluate();

    QCOMPARE(parser.value(), 5);
    QVERIFY(parser.isSuccess());
    QVERIFY(bSuccess);
    QCOMPARE(parser.errorType(), QMuParser::ErrorType::NONE);
}

void TestQMuParser::copyConstructor()
{
    QMuParser parser("1 + 2");

    QMuParser copy(parser);

    bool bSuccess = parser.evaluate();
    QVERIFY(bSuccess);

    bSuccess = copy.evaluate();
    QVERIFY(bSuccess);
    QCOMPARE(copy.value(), parser.value());
    QCOMPARE(copy.isSuccess(), parser.isSuccess());
    QCOMPARE(copy.msg(), parser.msg());
    QCOMPARE(copy.errorPos(), parser.errorPos());
    QCOMPARE(copy.errorType(), parser.errorType());
    QCOMPARE(copy.errorType(), QMuParser::ErrorType::NONE);
}

void TestQMuParser::copyConstructorSyntaxError()
{
    QMuParser parser("1 +");

    QMuParser copy(parser);

    bool bSuccess = parser.evaluate();
    QVERIFY(!bSuccess);

    bSuccess = copy.evaluate();
    QVERIFY(!bSuccess);
    QCOMPARE(copy.value(), parser.value());
    QCOMPARE(copy.isSuccess(), parser.isSuccess());
    QCOMPARE(copy.msg(), parser.msg());
    QCOMPARE(copy.errorPos(), parser.errorPos());
    QCOMPARE(copy.errorType(), parser.errorType());
    QCOMPARE(copy.errorType(), QMuParser::ErrorType::SYNTAX);
}

void TestQMuParser::assignmentOperator()
{
    QMuParser parser("1 + 2");

    QMuParser copy("0");

    copy = parser;

    bool bSuccess = parser.evaluate();
    QVERIFY(bSuccess);

    bSuccess = copy.evaluate();
    QVERIFY(bSuccess);
    QCOMPARE(copy.value(), parser.value());
    QCOMPARE(copy.isSuccess(), parser.isSuccess());
    QCOMPARE(copy.msg(), parser.msg());
    QCOMPARE(copy.errorPos(), parser.errorPos());
    QCOMPARE(copy.errorType(), parser.errorType());
    QCOMPARE(copy.errorType(), QMuParser::ErrorType::NONE);
}

void TestQMuParser::assignmentOperatorSyntaxError()
{
    QMuParser parser("1 +");

    QMuParser copy("0");

    copy = parser;

    bool bSuccess = parser.evaluate();
    QVERIFY(!bSuccess);

    bSuccess = copy.evaluate();
    QVERIFY(!bSuccess);
    QCOMPARE(copy.value(), parser.value());
    QCOMPARE(copy.isSuccess(), parser.isSuccess());
    QCOMPARE(copy.msg(), parser.msg());
    QCOMPARE(copy.errorPos(), parser.errorPos());
    QCOMPARE(copy.errorType(), parser.errorType());
    QCOMPARE(copy.errorType(), QMuParser::ErrorType::SYNTAX);
}

QTEST_GUILESS_MAIN(TestQMuParser)
