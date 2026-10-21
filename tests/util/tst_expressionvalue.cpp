#include "tst_expressionvalue.h"

#include "util/expressionvalue.h"

#include <QTest>

void TestExpressionValue::parseValid_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<double>("expected");

    QTest::newRow("integer") << QStringLiteral("42") << 42.0;
    QTest::newRow("decimal") << QStringLiteral("1.5") << 1.5;
    QTest::newRow("negative") << QStringLiteral("-3") << -3.0;
    QTest::newRow("whitespace") << QStringLiteral("  7  ") << 7.0;
    QTest::newRow("exponent") << QStringLiteral("1e3") << 1000.0;
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

void TestExpressionValue::parseInvalid_data()
{
    QTest::addColumn<QString>("text");

    QTest::newRow("empty") << QString();
    QTest::newRow("whitespace only") << QStringLiteral("   ");
    QTest::newRow("text") << QStringLiteral("abc");
    QTest::newRow("comma") << QStringLiteral("1,5");
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
