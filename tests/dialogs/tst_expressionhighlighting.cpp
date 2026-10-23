#include "tst_expressionhighlighting.h"

#include "dialogs/expressionhighlighting.h"
#include "util/expressionchecker.h"

#include <QTest>

void TestExpressionHighlighting::errorSpan_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<qint32>("errorPos");
    QTest::addColumn<qsizetype>("start");
    QTest::addColumn<qsizetype>("length");

    QTest::newRow("no error") << QStringLiteral("1 + 2") << -1 << qsizetype(-1) << qsizetype(0);
    QTest::newRow("position 0") << QStringLiteral("1 + 2") << 0 << qsizetype(0) << qsizetype(5);
    QTest::newRow("position 1") << QStringLiteral("1 + 2") << 1 << qsizetype(0) << qsizetype(5);
    QTest::newRow("position 2") << QStringLiteral("1 + 2") << 2 << qsizetype(0) << qsizetype(5);
    QTest::newRow("mid text") << QStringLiteral("1 + 2") << 4 << qsizetype(2) << qsizetype(3);
    QTest::newRow("last character") << QStringLiteral("1 + 2") << 6 << qsizetype(4) << qsizetype(1);
    QTest::newRow("past the end") << QStringLiteral("1 + 2") << 7 << qsizetype(-1) << qsizetype(0);
    QTest::newRow("empty text") << QString() << 0 << qsizetype(-1) << qsizetype(0);
}

void TestExpressionHighlighting::errorSpan()
{
    QFETCH(QString, text);
    QFETCH(qint32, errorPos);
    QFETCH(qsizetype, start);
    QFETCH(qsizetype, length);

    const ExpressionErrorMark::Span span = ExpressionErrorMark::span(errorPos, text);

    QCOMPARE(span.start, start);
    QCOMPARE(span.length, length);
}

void TestExpressionHighlighting::parserErrorSpan_data()
{
    QTest::addColumn<QString>("expression");
    QTest::addColumn<qint32>("errorPos");
    QTest::addColumn<qsizetype>("start");
    QTest::addColumn<qsizetype>("length");

    QTest::newRow("valid") << QStringLiteral("1 + + 2") << -1 << qsizetype(-1) << qsizetype(0);
    QTest::newRow("unexpected end") << QStringLiteral("1 +") << 4 << qsizetype(2) << qsizetype(1);
    QTest::newRow("missing parenthesis") << QStringLiteral("2 * (3") << 7 << qsizetype(5) << qsizetype(1);
    QTest::newRow("second reference") << QStringLiteral("${40001} ${40002}") << 9 << qsizetype(7) << qsizetype(10);
    QTest::newRow("unknown token") << QStringLiteral("${40001} + abc") << 11 << qsizetype(9) << qsizetype(5);
    QTest::newRow("unexpected value") << QStringLiteral("1 2") << 2 << qsizetype(0) << qsizetype(3);
    QTest::newRow("unexpected operator") << QStringLiteral("1 + * 3 + 4") << 4 << qsizetype(2) << qsizetype(9);
    QTest::newRow("unexpected hex value") << QStringLiteral("0x10 0x20") << 5 << qsizetype(3) << qsizetype(6);
}

/*!
 * \brief The marked span for the error position the parser reports on real expressions.
 */
void TestExpressionHighlighting::parserErrorSpan()
{
    QFETCH(QString, expression);
    QFETCH(qint32, errorPos);
    QFETCH(qsizetype, start);
    QFETCH(qsizetype, length);

    ExpressionChecker checker;
    checker.setExpression(expression);
    ResultDoubleList values;
    for (qsizetype idx = 0; idx < checker.requiredValueCount(); idx++)
    {
        values.append(ResultDouble(1, DataQuality::State::Good));
    }
    checker.checkWithValues(values);

    QCOMPARE(checker.errorPos(), errorPos);

    const ExpressionErrorMark::Span span = ExpressionErrorMark::span(checker.errorPos(), expression);
    QCOMPARE(span.start, start);
    QCOMPARE(span.length, length);
}

QTEST_MAIN(TestExpressionHighlighting)
