
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

namespace {

using QualityRows = QList<QList<DataQuality::Quality> >;

//! Calls setAllData on a model with two graphs and returns the qualities that graphsAddData emitted
QualityRows emittedQualities(const QList<double>& timeData,
                             const QList<QList<double> >& data,
                             const QualityRows& qualities)
{
    GraphDataModel model;
    model.add();
    model.add();

    QualityRows emitted;
    bool bEmitted = false;
    QObject::connect(
      &model, &GraphDataModel::graphsAddData,
      [&emitted](const QList<double>&, const QList<QList<double> >&, const QualityRows& q) { emitted = q; });
    QObject::connect(&model, &GraphDataModel::graphsAddData, [&bEmitted]() { bEmitted = true; });

    model.setAllData(timeData, data, qualities);

    if (!bEmitted)
    {
        QTest::qFail("graphsAddData not emitted", __FILE__, __LINE__);
    }
    return emitted;
}

} // namespace

void TestGraphDataModel::setAllDataPassesQualities()
{
    using DataQuality::Flag;
    using DataQuality::Quality;
    using DataQuality::State;

    const QList<double> timeData{ 0, 1 };
    const QList<QList<double> > data{ { 1, 2 }, { 3, 4 } };
    const QualityRows qualities{ { Quality{ State::Good, Flag::NoFlags }, Quality{ State::Invalid, Flag::NoFlags } },
                                 { Quality{ State::Degraded, Flag::OldData }, Quality{ State::Good, Flag::NoFlags } } };

    QVERIFY(emittedQualities(timeData, data, qualities) == qualities);
}

void TestGraphDataModel::setAllDataDropsMisalignedQualities()
{
    using DataQuality::Flag;
    using DataQuality::Quality;
    using DataQuality::State;

    const QList<double> timeData{ 0, 1 };
    const QList<QList<double> > data{ { 1, 2 }, { 3, 4 } };

    /* Second graph has one quality for two samples */
    const QualityRows qualities{ { Quality{ State::Good, Flag::NoFlags }, Quality{ State::Invalid, Flag::NoFlags } },
                                 { Quality{ State::Degraded, Flag::OldData } } };

    QVERIFY(emittedQualities(timeData, data, qualities).isEmpty());
}

QTEST_GUILESS_MAIN(TestGraphDataModel)
