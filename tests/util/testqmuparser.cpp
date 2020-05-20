
#include <QtTest/QtTest>

#include "qmuparser.h"

#include "testqmuparser.h"

void TestQMuParser::init()
{

}

void TestQMuParser::cleanup()
{

}

void TestQMuParser::evaluate()
{
    QMuParser parser(QStringLiteral("1+1"));

    QVERIFY(parser.evaluate());

    QCOMPARE(parser.result(), 2);
    QCOMPARE(parser.msg(), QStringLiteral("Success"));
}

QTEST_GUILESS_MAIN(TestQMuParser)
