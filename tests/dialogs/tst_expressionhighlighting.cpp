#include "tst_expressionhighlighting.h"

#include "dialogs/expressionhighlighting.h"
#include "util/expressionchecker.h"

#include <QTest>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextLayout>

void TestExpressionHighlighting::errorSpan_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<qint32>("errorPos");
    QTest::addColumn<qsizetype>("start");
    QTest::addColumn<qsizetype>("length");

    QTest::newRow("no error") << QStringLiteral("1 + 2") << -1 << qsizetype(-1) << qsizetype(0);
    QTest::newRow("first character") << QStringLiteral("1 + 2") << 0 << qsizetype(0) << qsizetype(1);
    QTest::newRow("operator") << QStringLiteral("1 + 2") << 2 << qsizetype(2) << qsizetype(1);
    QTest::newRow("number") << QStringLiteral("1 + 123") << 4 << qsizetype(4) << qsizetype(3);
    QTest::newRow("decimal number") << QStringLiteral("1 + 1.5") << 4 << qsizetype(4) << qsizetype(3);
    QTest::newRow("binary number") << QStringLiteral("1 + 0b101") << 4 << qsizetype(4) << qsizetype(5);
    QTest::newRow("name") << QStringLiteral("1 + abc") << 4 << qsizetype(4) << qsizetype(3);
    QTest::newRow("whole reference") << QStringLiteral("1 ${40001:s16b}") << 2 << qsizetype(2) << qsizetype(13);
    QTest::newRow("inside reference") << QStringLiteral("1 ${40001:s16b}") << 5 << qsizetype(2) << qsizetype(13);
    QTest::newRow("at the end") << QStringLiteral("1 + 2") << 5 << qsizetype(4) << qsizetype(1);
    QTest::newRow("past the end") << QStringLiteral("1 + ") << 7 << qsizetype(2) << qsizetype(1);
    QTest::newRow("only spaces") << QStringLiteral("  ") << 3 << qsizetype(-1) << qsizetype(0);
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
    QTest::newRow("second reference") << QStringLiteral("${40001} ${40002}") << 9 << qsizetype(9) << qsizetype(8);
    QTest::newRow("unknown token") << QStringLiteral("${40001} + abc") << 11 << qsizetype(11) << qsizetype(3);
    QTest::newRow("unexpected value") << QStringLiteral("1 2") << 2 << qsizetype(2) << qsizetype(1);
    QTest::newRow("unexpected operator") << QStringLiteral("1 + * 3 + 4") << 4 << qsizetype(4) << qsizetype(1);
    QTest::newRow("unexpected hex value") << QStringLiteral("0x10 0x20") << 5 << qsizetype(5) << qsizetype(4);
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

namespace {

//! Returns the ranges of \a block that have the error colour.
QList<QTextLayout::FormatRange> errorRanges(const QTextBlock& block)
{
    QList<QTextLayout::FormatRange> ranges;
    const QList<QTextLayout::FormatRange> formats = block.layout()->formats();
    for (const QTextLayout::FormatRange& range : formats)
    {
        if (range.format.foreground().color() == QColor(Qt::darkRed))
        {
            ranges.append(range);
        }
    }
    return ranges;
}

} // namespace

//! An expression with a line break marks the error in its own line only.
void TestExpressionHighlighting::errorMarkedOnlyInItsLine()
{
    QTextDocument document(QStringLiteral("1 + 2\n3 4"));
    ExpressionHighlighting highlighter(&document);

    highlighter.setExpressionErrorPosition(8);
    highlighter.rehighlight();

    QVERIFY(errorRanges(document.firstBlock()).isEmpty());
    const QList<QTextLayout::FormatRange> ranges = errorRanges(document.lastBlock());
    QCOMPARE(ranges.size(), 1);
    QCOMPARE(ranges[0].start, 2);
    QCOMPARE(ranges[0].length, 1);
}

QTEST_MAIN(TestExpressionHighlighting)
