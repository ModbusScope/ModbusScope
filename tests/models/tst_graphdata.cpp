
#include <QtTest/QtTest>

#include "tst_graphdata.h"

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

void TestGraphData::setExpressionResetsStatus()
{
    GraphData graphData;

    graphData.setExpression("${40001}}");
    graphData.setExpressionStatus(GraphData::ExpressionStatus::SYNTAX_ERROR);

    graphData.setExpression("${40001}");
    QCOMPARE(GraphData::ExpressionStatus::UNKNOWN, graphData.expressionStatus());
}

/* TODO: Add extra test for other functions */

QTEST_GUILESS_MAIN(TestGraphData)
