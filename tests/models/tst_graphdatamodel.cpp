
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

void TestGraphDataModel::addAssignsUniqueDefaultLabel()
{
    GraphDataModel model;
    model.add();
    model.add();
    model.add();

    QCOMPARE(model.label(GraphIdx(0)), QStringLiteral("New signal"));
    QCOMPARE(model.label(GraphIdx(1)), QStringLiteral("New signal 2"));
    QCOMPARE(model.label(GraphIdx(2)), QStringLiteral("New signal 3"));
}

void TestGraphDataModel::addReusesLabelFreedByRename()
{
    GraphDataModel model;
    model.add();
    model.setLabel(GraphIdx(0), QStringLiteral("Renamed"));

    model.add();
    model.add();

    QCOMPARE(model.label(GraphIdx(1)), QStringLiteral("New signal"));
    QCOMPARE(model.label(GraphIdx(2)), QStringLiteral("New signal 2"));
}

void TestGraphDataModel::addReusesLabelFreedByRemove()
{
    GraphDataModel model;
    model.add();
    model.add();
    model.add();
    QCOMPARE(model.label(GraphIdx(1)), QStringLiteral("New signal 2"));

    model.removeRegister(GraphIdx(1));
    model.add();

    QCOMPARE(model.label(GraphIdx(2)), QStringLiteral("New signal 2"));
}

void TestGraphDataModel::addUniqueLabelIsCaseSensitive()
{
    /* "new signal" (lowercase) must not be treated as colliding with the "New signal" default,
       so the next add() reuses "New signal" rather than skipping to "New signal 2". */
    GraphDataModel model;
    model.add();
    model.setLabel(GraphIdx(0), QStringLiteral("new signal"));

    model.add();

    QCOMPARE(model.label(GraphIdx(1)), QStringLiteral("New signal"));
}

void TestGraphDataModel::activeCountZeroWhenEmpty()
{
    GraphDataModel model;
    QCOMPARE(model.activeCount(), 0);
}

void TestGraphDataModel::activeCountReflectsAddedRegisters()
{
    GraphDataModel model;
    model.add();
    QCOMPARE(model.activeCount(), 1);
    model.add();
    QCOMPARE(model.activeCount(), 2);
}

void TestGraphDataModel::activeCountZeroWhenAllDeactivated()
{
    /* Registers that exist but are inactive must not count — this is the condition
       used to show the empty-state label in the graph area. */
    GraphDataModel model;
    model.add();
    QCOMPARE(model.activeCount(), 1);

    model.setActive(GraphIdx(0), false);
    QCOMPARE(model.activeCount(), 0);
}

QTEST_GUILESS_MAIN(TestGraphDataModel)
