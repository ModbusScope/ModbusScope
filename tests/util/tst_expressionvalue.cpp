#include "tst_expressionvalue.h"

#include "util/expressionvalue.h"

#include <QTest>

#include <cmath>

void TestExpressionValue::parseValid_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<double>("expected");

    QTest::newRow("integer") << QStringLiteral("42") << 42.0;
    QTest::newRow("decimal") << QStringLiteral("1.5") << 1.5;
    QTest::newRow("negative") << QStringLiteral("-3") << -3.0;
    QTest::newRow("whitespace") << QStringLiteral("  7  ") << 7.0;
    QTest::newRow("exponent") << QStringLiteral("1e3") << 1000.0;
    QTest::newRow("hex") << QStringLiteral("0x10") << 16.0;
    QTest::newRow("hex upper case") << QStringLiteral("0XfF") << 255.0;
    QTest::newRow("binary") << QStringLiteral("0b101") << 5.0;
    QTest::newRow("negative hex") << QStringLiteral("-0x10") << -16.0;
    QTest::newRow("hex with whitespace") << QStringLiteral(" 0x1 ") << 1.0;
    QTest::newRow("negative binary") << QStringLiteral("-0b101") << -5.0;
    QTest::newRow("binary upper case") << QStringLiteral("0B11") << 3.0;
    QTest::newRow("negative hex zero") << QStringLiteral("-0x0") << 0.0;
}

void TestExpressionValue::parseValid()
{
    QFETCH(QString, text);
    QFETCH(double, expected);

    bool ok = false;
    const double value = ExpressionValue::parse(text, &ok);

    QVERIFY(ok);
    QCOMPARE(value, expected);
}

//! A negative zero is returned as positive zero, so it never shows as "-0".
void TestExpressionValue::parseNegativeZero()
{
    bool ok = false;
    const double value = ExpressionValue::parse(QStringLiteral("-0x0"), &ok);

    QVERIFY(ok);
    QVERIFY(!std::signbit(value));
}

void TestExpressionValue::parseWithoutOk()
{
    QCOMPARE(ExpressionValue::parse(QStringLiteral("0x10"), nullptr), 16.0);
}

void TestExpressionValue::parseInvalid_data()
{
    QTest::addColumn<QString>("text");

    QTest::newRow("empty") << QString();
    QTest::newRow("whitespace only") << QStringLiteral("   ");
    QTest::newRow("text") << QStringLiteral("abc");
    QTest::newRow("comma") << QStringLiteral("1,5");
    QTest::newRow("hex prefix only") << QStringLiteral("0x");
    QTest::newRow("hex invalid digit") << QStringLiteral("0xZZ");
    QTest::newRow("binary invalid digit") << QStringLiteral("0b2");
    QTest::newRow("hex double sign") << QStringLiteral("--0x1");
    QTest::newRow("hex double prefix") << QStringLiteral("0x0x10");
    QTest::newRow("hex space after prefix") << QStringLiteral("0x 10");
    QTest::newRow("hex plus after prefix") << QStringLiteral("0x+10");
    QTest::newRow("hex minus after prefix") << QStringLiteral("0x-0");
    QTest::newRow("binary plus after prefix") << QStringLiteral("0b+1");
    QTest::newRow("space after minus") << QStringLiteral("- 0x10");
    QTest::newRow("minus only") << QStringLiteral("-");
    QTest::newRow("hex overflow") << QStringLiteral("0x10000000000000000");
}

void TestExpressionValue::parseInvalid()
{
    QFETCH(QString, text);

    bool ok = true;
    const double value = ExpressionValue::parse(text, &ok);

    QVERIFY(!ok);
    QCOMPARE(value, 0.0);
}

QTEST_GUILESS_MAIN(TestExpressionValue)
