
#include <QtTest/QtTest>

#include "qmuparser.h"

#include "testqmuparser.h"

#define ADD_TEST(expr, result)      QTest::newRow(expr) << QString(expr) << static_cast<double>(result)

#define ADD_REG_TEST(expr, registerValue, result)      QTest::newRow(expr) << QString(expr) << static_cast<quint32>(registerValue) << static_cast<double>(result)

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
    ADD_TEST("1 | 2",           3    );
    ADD_TEST("123 & 456",       72   );
    ADD_TEST("1 << 3",          8    );
    ADD_TEST("8 >> 3",          1    );

    /* Arithmetic */
    ADD_TEST("16 / 4",          4    );
    ADD_TEST("9 * 4",           36   );
    ADD_TEST("9 % 4",           1    );
    ADD_TEST("-10+1",           -9   );
    ADD_TEST("1+2*3",           7    );
    ADD_TEST("(1+2)*3",         9    );
    ADD_TEST("3+-3^2",          -6   );
    ADD_TEST("2^2^3",           256  );
    ADD_TEST("-2^2",            -4   );
    ADD_TEST("(-2)^2",          4    );

    /* Hexadecimal */
    ADD_TEST("0xff",            255  );
    ADD_TEST("0xffFF",          65535  );
    ADD_TEST("0xFFFFffff",      4294967295  );
    ADD_TEST("0xff",            255  );
    ADD_TEST("10+0xff",         265  );
    ADD_TEST("0xff+10",         265  );
    ADD_TEST("10*0xff",         2550 );
    ADD_TEST("0xff*10",         2550 );
    ADD_TEST("10+0xff+1",       266  );
    ADD_TEST("1+0xff+10",       266  );
    ADD_TEST("0x12345678",      305419896  );
    ADD_TEST("0xABCDEF",        11259375  );

    /* Binary */
    ADD_TEST("0b11111111",      255  );
    ADD_TEST("0b11111111111111111111111111111111",      4294967295);
    ADD_TEST("0b000001",        1);
    ADD_TEST("0b00",            0);
    ADD_TEST("0b01 + 1 + 0x01", 3);
    ADD_TEST("0b1 << 3",        8);

    /* Floating point result */
    ADD_TEST("15 / 4",          3.75   );
    ADD_TEST("1 / 1000",        0.001  );
    ADD_TEST("15.6 + 5.5",      21.1   );
    ADD_TEST("15.6 - 5.5",      10.1   );
    ADD_TEST("3.33 * 3",        9.99   );

    /* Floating point with binary/integer operations: fraction part is removed and ignored */
    ADD_TEST("15.5 | 0xF",     15  );
    ADD_TEST("16.5 & 0x10",     16  );
    ADD_TEST("1.3 << 3",        8    );
    ADD_TEST("8.6 >> 3",        1    );

    ADD_TEST("9.5 % 4",         1    );

    /* Floating point result */
    ADD_TEST("1.5 + 1.5",       3    );
    ADD_TEST("1,5 + 1",         2.5  );
    ADD_TEST("1.5 + 1",         2.5  );
    ADD_TEST("1,5 + 1,5",       3    );

}

void TestQMuParser::evaluate()
{
    QFETCH(QString, expression);
    QFETCH(double, result);

    QMuParser parser(expression);

    bool bSuccess = parser.evaluate();

    QCOMPARE(parser.result(), result);
    QVERIFY(parser.isSuccess());
    QVERIFY(bSuccess);
}

void TestQMuParser::evaluateRegister_data()
{
    QTest::addColumn<QString>("expression");
    QTest::addColumn<quint32>("registerValue");
    QTest::addColumn<double>("result");


    ADD_REG_TEST("VAL",           2,        2    );
    ADD_REG_TEST("VAL + 2",       3,        5    );
    ADD_REG_TEST("VAL * 2",       4,        8    );
    ADD_REG_TEST("VAL / 1000",    5,        0.005);
    ADD_REG_TEST("VAL & 0xFF",    257,      1);

}

void TestQMuParser::evaluateRegister()
{
    QFETCH(QString, expression);
    QFETCH(quint32, registerValue);
    QFETCH(double, result);

    QMuParser parser(expression);

    bool bSuccess = parser.evaluate(registerValue);

    QCOMPARE(parser.result(), result);
    QVERIFY(parser.isSuccess());
    QVERIFY(bSuccess);
}

void TestQMuParser::evaluateInvalidExpr()
{
    QMuParser parser("x11");

    bool bSuccess = parser.evaluate();

    QCOMPARE(parser.result(), 0);
    QVERIFY(!parser.isSuccess());
    QVERIFY(!bSuccess);
}

void TestQMuParser::evaluateInvalidHexExpr()
{
    QMuParser parser("0x");

    bool bSuccess = parser.evaluate();

    QCOMPARE(parser.result(), 0);
    QVERIFY(!parser.isSuccess());
    QVERIFY(!bSuccess);
}

void TestQMuParser::evaluateInvalidBinExpr()
{
    QMuParser parser("0b");

    bool bSuccess = parser.evaluate();

    QCOMPARE(parser.result(), 0);
    QVERIFY(!parser.isSuccess());
    QVERIFY(!bSuccess);
}

void TestQMuParser::evaluateInvalidBinExpr_2()
{
    /* 33 bits */
    QMuParser parser("0b111101110111011101110111011101110");

    bool bSuccess = parser.evaluate();

    QCOMPARE(parser.result(), 0);
    QVERIFY(!parser.isSuccess());
    QVERIFY(!bSuccess);
}

void TestQMuParser::evaluateInvalidDecimal()
{
    QMuParser parser("1.1 + 1,5");

    bool bSuccess = parser.evaluate();

    QCOMPARE(parser.result(), 0);
    QVERIFY(!parser.isSuccess());
    QVERIFY(!bSuccess);
}

void TestQMuParser::evaluateDivByZero()
{
    QMuParser parser("1/0");

    bool bSuccess = parser.evaluate();

    QCOMPARE(parser.result(), 0);
    QVERIFY(!parser.isSuccess());
    QVERIFY(!bSuccess);
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
    QCOMPARE(parser_1.result(), 1.5);
}

void TestQMuParser::expressionGet()
{
    QString expr = QStringLiteral("1.1 + 1,5");

    QMuParser parser(expr);

    QCOMPARE(parser.expression(), expr);
}

void TestQMuParser::expressionUpdate()
{
    QMuParser parser("VAL + 1");

    bool bSuccess = parser.evaluate(5);

    QCOMPARE(parser.result(), 6);
    QVERIFY(parser.isSuccess());
    QVERIFY(bSuccess);

    parser.setExpression("VAL + 2");

    bSuccess = parser.evaluate(5);

    QCOMPARE(parser.result(), 7);
    QVERIFY(parser.isSuccess());
    QVERIFY(bSuccess);
}

QTEST_GUILESS_MAIN(TestQMuParser)
