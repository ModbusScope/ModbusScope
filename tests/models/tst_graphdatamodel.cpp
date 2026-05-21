
#include "tst_graphdatamodel.h"

#include "models/graphdatamodel.h"
#include "util/graphindex.h"

#include <QTest>

void TestGraphDataModel::init()
{
}

void TestGraphDataModel::cleanup()
{
}

void TestGraphDataModel::addUsesDefaultExpression()
{
    GraphDataModel model;
    model.add();

    QCOMPARE(model.expression(GraphIdx(0)), QStringLiteral("${h0}"));
}

void TestGraphDataModel::addUsesCustomDefaultExpression()
{
    GraphDataModel model;
    model.setDefaultExpression(QStringLiteral("${h5:s16b}"));
    model.add();

    QCOMPARE(model.expression(GraphIdx(0)), QStringLiteral("${h5:s16b}"));
}

void TestGraphDataModel::setDefaultExpressionUpdatesSubsequentAdds()
{
    GraphDataModel model;

    model.add();
    QCOMPARE(model.expression(GraphIdx(0)), QStringLiteral("${h0}"));

    model.setDefaultExpression(QStringLiteral("${i10}"));
    model.add();
    QCOMPARE(model.expression(GraphIdx(0)), QStringLiteral("${h0}"));
    QCOMPARE(model.expression(GraphIdx(1)), QStringLiteral("${i10}"));
}

void TestGraphDataModel::setDefaultExpressionIgnoresEmptyString()
{
    GraphDataModel model;
    model.setDefaultExpression(QStringLiteral("${i0}"));
    model.setDefaultExpression(QString());
    model.add();

    QCOMPARE(model.expression(GraphIdx(0)), QStringLiteral("${i0}"));
}

QTEST_GUILESS_MAIN(TestGraphDataModel)
