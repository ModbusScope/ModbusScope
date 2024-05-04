
#include <QtTest/QtTest>

#include "expressionchecker.h"
#include "tst_expressionchecker.h"

using State = ResultState::State;

void TestExpressionChecker::init()
{

}

void TestExpressionChecker::cleanup()
{
}

void TestExpressionChecker::dataIsPrimed()
{
    ExpressionChecker checker;

    QString expr("${40001} + ${40002}");

    checker.checkExpression(expr);
    QCOMPARE(checker.expression(), expr);

    QStringList descriptions;
    checker.descriptions(descriptions);
    auto expDescriptions = QStringList() << "holding register, 0, unsigned 16-bit, conn 1"
                                         << "holding register, 1, unsigned 16-bit, conn 1";
    QCOMPARE(descriptions, expDescriptions);

    QCOMPARE(checker.requiredValueCount(), 2);
}

void TestExpressionChecker::expressionIsValid()
{
    ExpressionChecker checker;

    QString expr("${40001} + ${40002}");
    checker.checkExpression(expr);

    QSignalSpy spyResult(&checker, &ExpressionChecker::resultsReady);

    auto resultList = ResultDoubleList() << ResultDouble(2, State::SUCCESS) << ResultDouble(1, State::SUCCESS);
    checker.setValues(resultList);

    QCOMPARE(spyResult.count(), 1);

    QList<QVariant> arguments = spyResult.takeFirst();
    QCOMPARE(arguments.count(), 1);
    QVERIFY(arguments.first().toBool());

    QVERIFY(checker.isValid());
    QCOMPARE(checker.result(), 3);
    QCOMPARE(checker.strError(), "");
    QCOMPARE(checker.errorPos(), -1);
    QVERIFY(checker.syntaxError() == false);
}

void TestExpressionChecker::expressionHasSyntaxError()
{
    ExpressionChecker checker;

    QString expr("${40001}++");
    checker.checkExpression(expr);

    QSignalSpy spyResult(&checker, &ExpressionChecker::resultsReady);

    auto resultList = ResultDoubleList() << ResultDouble(2, State::SUCCESS);
    checker.setValues(resultList);

    QCOMPARE(spyResult.count(), 1);

    QList<QVariant> arguments = spyResult.takeFirst();
    QCOMPARE(arguments.count(), 1);
    QVERIFY(arguments.first().toBool() == false);

    QVERIFY(checker.isValid() == false);
    QCOMPARE(checker.result(), 0);
    QCOMPARE(checker.strError(), "Invalid expression (error at position 11)");
    QCOMPARE(checker.errorPos(), 11);
    QVERIFY(checker.syntaxError());
}

void TestExpressionChecker::valuErrorIsNotSyntaxError()
{
    ExpressionChecker checker;

    QString expr("1/${40001}");
    checker.checkExpression(expr);

    QSignalSpy spyResult(&checker, &ExpressionChecker::resultsReady);

    auto resultList = ResultDoubleList() << ResultDouble(0, State::SUCCESS);
    checker.setValues(resultList);

    QCOMPARE(spyResult.count(), 1);

    QList<QVariant> arguments = spyResult.takeFirst();
    QCOMPARE(arguments.count(), 1);
    QVERIFY(arguments.first().toBool() == false);

    QVERIFY(checker.isValid() == false);
    QCOMPARE(checker.result(), 0);
    QCOMPARE(checker.strError(), "Result value is an undefined number. Check input validity.");
    QCOMPARE(checker.errorPos(), -1);
    QVERIFY(checker.syntaxError() == false);
}

QTEST_GUILESS_MAIN(TestExpressionChecker)
