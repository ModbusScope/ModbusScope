
#include <QtTest/QtTest>

#include "testgraphdata.h"

#include "graphdata.h"
#include "util.h"

void TestGraphData::init()
{

}

void TestGraphData::cleanup()
{

}

/* TODO: Mock Util.h */

void TestGraphData::setLabel()
{
    GraphData graphData;

    QString separatorString = QString(Util::separatorCharacter());
    QString baseString = QString("TEST");

    graphData.setLabel(baseString);
    QCOMPARE(graphData.label(), baseString);

    QString testString1 = baseString + separatorString;
    graphData.setLabel(testString1);
    QCOMPARE(graphData.label(), baseString);

    QString testString2 = separatorString + baseString;
    graphData.setLabel(testString2);
    QCOMPARE(graphData.label(), baseString);

    QString testString3 = separatorString + baseString + separatorString;
    graphData.setLabel(testString3);
    QCOMPARE(graphData.label(), baseString);

    QString testString4 = baseString + separatorString + baseString;
    graphData.setLabel(testString4);
    QCOMPARE(graphData.label(), baseString + baseString);
}

void TestGraphData::setExpression_data()
{

    QTest::addColumn<QString>("regString");

    QTest::newRow("reg") << "reg";
    QTest::newRow("ReG") << "ReG";
    QTest::newRow("REG") << "REG";
    QTest::newRow("Reg") << "Reg";
}

void TestGraphData::setExpression()
{
    QFETCH(QString, regString);

    GraphData graphData;

    graphData.setExpression(regString);

    QCOMPARE(graphData.expression(), QStringLiteral("REG"));
}

/* TODO: Add extra test for other functions */

QTEST_GUILESS_MAIN(TestGraphData)
