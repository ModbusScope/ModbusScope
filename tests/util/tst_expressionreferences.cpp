#include "tst_expressionreferences.h"

#include "util/expressionreferences.h"

#include <QTest>

using ExpressionReferences::Reference;

void TestExpressionReferences::findNone()
{
    QVERIFY(ExpressionReferences::find(QStringLiteral("1 + 2")).isEmpty());
    QVERIFY(ExpressionReferences::find(QString()).isEmpty());
}

void TestExpressionReferences::findOne()
{
    const QList<Reference> refs = ExpressionReferences::find(QStringLiteral("${40003:s16b} * 2"));

    QCOMPARE(refs.size(), 1);
    QCOMPARE(refs[0].start, 0);
    QCOMPARE(refs[0].length, 13);
    QCOMPARE(refs[0].text, QStringLiteral("${40003:s16b}"));
    QCOMPARE(refs[0].deviceId, Device::cFirstDeviceId);
}

void TestExpressionReferences::findDuplicates()
{
    const QList<Reference> refs = ExpressionReferences::find(QStringLiteral("${40001} - ${40001}"));

    QCOMPARE(refs.size(), 2);
    QCOMPARE(refs[0].start, 0);
    QCOMPARE(refs[1].start, 11);
    QCOMPARE(refs[0].text, refs[1].text);
}

void TestExpressionReferences::findWithWhitespaceAndDevice()
{
    const QList<Reference> refs = ExpressionReferences::find(QStringLiteral("2 * ${ 40001 @ 2 : s16b }"));

    QCOMPARE(refs.size(), 1);
    QCOMPARE(refs[0].start, 4);
    QCOMPARE(refs[0].text, QStringLiteral("${ 40001 @ 2 : s16b }"));
    QCOMPARE(refs[0].deviceId, deviceId_t(2));
}

void TestExpressionReferences::findInvalidContent()
{
    const QList<Reference> refs = ExpressionReferences::find(QStringLiteral("${@3} + ${1@2}"));

    QCOMPARE(refs.size(), 2);
    QCOMPARE(refs[0].text, QStringLiteral("${@3}"));
    QCOMPARE(refs[0].deviceId, Device::cFirstDeviceId);
    QCOMPARE(refs[1].deviceId, deviceId_t(2));
}

void TestExpressionReferences::replaceSecondOfTwoIdentical()
{
    const QString expression = QStringLiteral("${40001} - ${40001}");
    const QList<Reference> refs = ExpressionReferences::find(expression);

    const QString result = ExpressionReferences::replace(expression, refs[1], QStringLiteral("${h5}"));

    QCOMPARE(result, QStringLiteral("${40001} - ${h5}"));
}

void TestExpressionReferences::replaceKeepsSurroundingText()
{
    const QString expression = QStringLiteral("(${40001} + 10) / 2");
    const QList<Reference> refs = ExpressionReferences::find(expression);

    const QString result = ExpressionReferences::replace(expression, refs[0], QStringLiteral("${40002:s16b}"));

    QCOMPARE(result, QStringLiteral("(${40002:s16b} + 10) / 2"));
}

void TestExpressionReferences::replaceStaleReference()
{
    const QList<Reference> refs = ExpressionReferences::find(QStringLiteral("${40001} + 1"));
    const QString changed = QStringLiteral("1 + ${40001}");

    QCOMPARE(ExpressionReferences::replace(changed, refs[0], QStringLiteral("${h5}")), changed);
}

QTEST_MAIN(TestExpressionReferences)
