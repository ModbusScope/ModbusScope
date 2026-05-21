
#include "tst_graphdatastore.h"

#include "models/graphdata.h"
#include "models/graphdatastore.h"
#include "util/graphindex.h"
#include "util/util.h"

#include <QSignalSpy>
#include <QTest>

static GraphData makeGraph(const QString& label, const QString& expression, bool active = true)
{
    GraphData g;
    g.setLabel(label);
    g.setExpression(expression);
    g.setActive(active);
    return g;
}

void TestGraphDataStore::initTestCase()
{
    qRegisterMetaType<GraphIdx>("GraphIdx");
    qRegisterMetaType<ActiveIdx>("ActiveIdx");
}

void TestGraphDataStore::init()
{
}

void TestGraphDataStore::cleanup()
{
}

void TestGraphDataStore::insertIncreasesSize()
{
    GraphDataStore store;

    store.insertGraphData(makeGraph("A", "${h0}"));
    QCOMPARE(store.size(), 1);
    QCOMPARE(store.activeCount(), 1);

    store.insertGraphData(makeGraph("B", "${h1}"));
    QCOMPARE(store.size(), 2);
    QCOMPARE(store.activeCount(), 2);
}

void TestGraphDataStore::insertAssignsColorFromList()
{
    GraphDataStore store;
    GraphData g;
    g.setLabel("no color");
    g.setExpression("${h0}");
    // do not set a color — store should assign one
    store.insertGraphData(g);

    QCOMPARE(store.color(GraphIdx(0)), Util::cColorlist[0]);
}

void TestGraphDataStore::insertPreservesExplicitColor()
{
    GraphDataStore store;
    GraphData g;
    g.setLabel("explicit color");
    g.setExpression("${h0}");
    g.setColor(QColor(Qt::red));
    store.insertGraphData(g);

    QCOMPARE(store.color(GraphIdx(0)), QColor(Qt::red));
}

void TestGraphDataStore::eraseRemovesCorrectEntry()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}"));
    store.insertGraphData(makeGraph("B", "${h1}"));
    store.insertGraphData(makeGraph("C", "${h2}"));

    store.eraseGraphDataAt(GraphIdx(1));

    QCOMPARE(store.size(), 2);
    QCOMPARE(store.activeCount(), 2);
    QCOMPARE(store.label(GraphIdx(0)), QStringLiteral("A"));
    QCOMPARE(store.label(GraphIdx(1)), QStringLiteral("C"));
}

void TestGraphDataStore::clearEmptiesStore()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}"));
    store.insertGraphData(makeGraph("B", "${h1}"));

    QCOMPARE(store.size(), 2);
    QCOMPARE(store.activeCount(), 2);

    store.clearAllGraphData();

    QCOMPARE(store.size(), 0);
    QCOMPARE(store.activeCount(), 0);
}

void TestGraphDataStore::setActiveFalseRemovesFromActiveCount()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}", true));
    store.insertGraphData(makeGraph("B", "${h1}", true));

    QCOMPARE(store.activeCount(), 2);

    store.setActive(GraphIdx(0), false);

    QCOMPARE(store.activeCount(), 1);
    QVERIFY(!store.isActive(GraphIdx(0)));
}

void TestGraphDataStore::setActiveFalseClearsData()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}", true));

    QSignalSpy spy(&store, &GraphDataStore::activeChanged);
    store.setActive(GraphIdx(0), false);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<GraphIdx>(), GraphIdx(0));
    QVERIFY(!store.isActive(GraphIdx(0)));
    QVERIFY(store.dataMap(GraphIdx(0))->isEmpty());
}

void TestGraphDataStore::setActiveTrueRestoresVisibility()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}", true));
    store.setActive(GraphIdx(0), false);
    store.setVisible(GraphIdx(0), false);

    QSignalSpy visSpy(&store, &GraphDataStore::visibilityChanged);
    store.setActive(GraphIdx(0), true);

    QCOMPARE(visSpy.count(), 1);
    QCOMPARE(visSpy.at(0).at(0).value<GraphIdx>(), GraphIdx(0));
    QVERIFY(store.isActive(GraphIdx(0)));
    QVERIFY(store.isVisible(GraphIdx(0)));
}

void TestGraphDataStore::activeGraphIndexListReturnsSortedIndices()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}", true));
    store.insertGraphData(makeGraph("B", "${h1}", false));
    store.insertGraphData(makeGraph("C", "${h2}", true));

    QList<GraphIdx> list;
    store.activeGraphIndexList(list);

    QCOMPARE(list.size(), 2);
    QCOMPARE(list.at(0), GraphIdx(0));
    QCOMPARE(list.at(1), GraphIdx(2));
}

void TestGraphDataStore::convertToActiveGraphIndexRoundTrip()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}", true));
    store.insertGraphData(makeGraph("B", "${h1}", false));
    store.insertGraphData(makeGraph("C", "${h2}", true));

    // graph index 2 is the second active graph (active index 1)
    QCOMPARE(store.convertToActiveGraphIndex(GraphIdx(2)), ActiveIdx(1));
    QCOMPARE(store.convertToGraphIndex(ActiveIdx(1)), GraphIdx(2));
    // graph index 1 is inactive — must return invalid (NONE)
    QCOMPARE(store.convertToActiveGraphIndex(GraphIdx(1)), ActiveIdx());
}

void TestGraphDataStore::setVisibleFalseOnSelectedGraphResetsSelection()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}", true));
    store.setSelectedGraph(GraphIdx(0));
    QCOMPARE(store.selectedGraph(), GraphIdx(0));

    QSignalSpy visSpy(&store, &GraphDataStore::visibilityChanged);
    store.setVisible(GraphIdx(0), false);

    QCOMPARE(visSpy.count(), 1);
    QCOMPARE(visSpy.at(0).at(0).value<GraphIdx>(), GraphIdx(0));
    QCOMPARE(store.selectedGraph(), GraphIdx());
}

void TestGraphDataStore::moveGraphRowReorders()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}"));
    store.insertGraphData(makeGraph("B", "${h1}"));
    store.insertGraphData(makeGraph("C", "${h2}"));

    // QList::move(0, 2): remove A from 0, insert at 2 → [B, C, A]
    store.moveGraphRow(GraphIdx(0), GraphIdx(2));

    QCOMPARE(store.label(GraphIdx(0)), QStringLiteral("B"));
    QCOMPARE(store.label(GraphIdx(1)), QStringLiteral("C"));
    QCOMPARE(store.label(GraphIdx(2)), QStringLiteral("A"));

    QList<GraphIdx> activeList;
    store.activeGraphIndexList(activeList);
    QCOMPARE(activeList.size(), 3);
    QCOMPARE(activeList.at(0), GraphIdx(0));
    QCOMPARE(activeList.at(1), GraphIdx(1));
    QCOMPARE(activeList.at(2), GraphIdx(2));
}

void TestGraphDataStore::eraseSelectedGraphResetsSelection()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}"));
    store.insertGraphData(makeGraph("B", "${h1}"));
    store.setSelectedGraph(GraphIdx(1));
    QCOMPARE(store.selectedGraph(), GraphIdx(1));

    QSignalSpy spy(&store, &GraphDataStore::selectedGraphChanged);
    store.eraseGraphDataAt(GraphIdx(1));

    QCOMPARE(store.selectedGraph(), GraphIdx());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<GraphIdx>(), GraphIdx());
}

void TestGraphDataStore::eraseGraphBeforeSelectedResetsSelection()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}"));
    store.insertGraphData(makeGraph("B", "${h1}"));
    store.insertGraphData(makeGraph("C", "${h2}"));
    store.setSelectedGraph(GraphIdx(2));
    QCOMPARE(store.selectedGraph(), GraphIdx(2));

    QSignalSpy spy(&store, &GraphDataStore::selectedGraphChanged);
    store.eraseGraphDataAt(GraphIdx(0));

    QCOMPARE(store.selectedGraph(), GraphIdx());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<GraphIdx>(), GraphIdx());
}

void TestGraphDataStore::eraseGraphAfterSelectedResetsSelection()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}"));
    store.insertGraphData(makeGraph("B", "${h1}"));
    store.insertGraphData(makeGraph("C", "${h2}"));
    store.setSelectedGraph(GraphIdx(0));
    QCOMPARE(store.selectedGraph(), GraphIdx(0));

    QSignalSpy spy(&store, &GraphDataStore::selectedGraphChanged);
    store.eraseGraphDataAt(GraphIdx(2));

    QCOMPARE(store.selectedGraph(), GraphIdx());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<GraphIdx>(), GraphIdx());
}

void TestGraphDataStore::clearAllGraphDataResetsSelection()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}"));
    store.insertGraphData(makeGraph("B", "${h1}"));
    store.setSelectedGraph(GraphIdx(1));
    QCOMPARE(store.selectedGraph(), GraphIdx(1));

    QSignalSpy spy(&store, &GraphDataStore::selectedGraphChanged);
    store.clearAllGraphData();

    QCOMPARE(store.selectedGraph(), GraphIdx());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<GraphIdx>(), GraphIdx());
}

void TestGraphDataStore::moveRowResetsSelection()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}"));
    store.insertGraphData(makeGraph("B", "${h1}"));
    store.insertGraphData(makeGraph("C", "${h2}"));
    store.setSelectedGraph(GraphIdx(0));
    QCOMPARE(store.selectedGraph(), GraphIdx(0));

    QSignalSpy spy(&store, &GraphDataStore::selectedGraphChanged);
    store.moveGraphRow(GraphIdx(0), GraphIdx(2));

    QCOMPARE(store.selectedGraph(), GraphIdx());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<GraphIdx>(), GraphIdx());
}

void TestGraphDataStore::insertInactiveGraphDoesNotIncreaseActiveCount()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}", true));
    QCOMPARE(store.activeCount(), 1);

    store.insertGraphData(makeGraph("B", "${h1}", false));

    QCOMPARE(store.size(), 2);
    QCOMPARE(store.activeCount(), 1);
}

void TestGraphDataStore::setActiveNoopDoesNotEmitSignal()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}", true));

    QSignalSpy spy(&store, &GraphDataStore::activeChanged);
    store.setActive(GraphIdx(0), true);

    QCOMPARE(spy.count(), 0);
}

void TestGraphDataStore::setSelectedGraphEmitsSignal()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}", true));

    QSignalSpy spy(&store, &GraphDataStore::selectedGraphChanged);
    store.setSelectedGraph(GraphIdx(0));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<GraphIdx>(), GraphIdx(0));
    QCOMPARE(store.selectedGraph(), GraphIdx(0));
}

void TestGraphDataStore::setSelectedGraphOnThirdRegisterWithMiddleInactiveEmitsGraphIndex()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}", true));
    store.insertGraphData(makeGraph("B", "${h1}", false));
    store.insertGraphData(makeGraph("C", "${h2}", true));

    QSignalSpy spy(&store, &GraphDataStore::selectedGraphChanged);
    store.setSelectedGraph(GraphIdx(2));

    // signal must carry graph index (2), not the active index (1)
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<GraphIdx>(), GraphIdx(2));
    QCOMPARE(store.selectedGraph(), GraphIdx(2));
    // and the active index for graph 2 must be 1, not 2
    QCOMPARE(store.convertToActiveGraphIndex(GraphIdx(2)), ActiveIdx(1));
}

void TestGraphDataStore::setSelectedGraphOnInvisibleGraphIsIgnored()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}", true));
    store.setVisible(GraphIdx(0), false);

    store.setSelectedGraph(GraphIdx(0));

    QCOMPARE(store.selectedGraph(), GraphIdx());
}

void TestGraphDataStore::setVisibleTrueEmitsSignal()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}", true));
    store.setVisible(GraphIdx(0), false);

    QSignalSpy spy(&store, &GraphDataStore::visibilityChanged);
    store.setVisible(GraphIdx(0), true);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<GraphIdx>(), GraphIdx(0));
    QVERIFY(store.isVisible(GraphIdx(0)));
}

void TestGraphDataStore::setVisibleFalseEmitsSignal()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}", true));
    store.insertGraphData(makeGraph("B", "${h1}", true));

    QSignalSpy spy(&store, &GraphDataStore::visibilityChanged);
    store.setVisible(GraphIdx(1), false);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<GraphIdx>(), GraphIdx(1));
    QVERIFY(!store.isVisible(GraphIdx(1)));
}

void TestGraphDataStore::setValueAxisEmitsSignal()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}", true));

    QSignalSpy spy(&store, &GraphDataStore::valueAxisChanged);
    store.setValueAxis(GraphIdx(0), GraphData::VALUE_AXIS_SECONDARY);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<GraphIdx>(), GraphIdx(0));
    QCOMPARE(store.valueAxis(GraphIdx(0)), GraphData::VALUE_AXIS_SECONDARY);
}

void TestGraphDataStore::setExpressionEmitsSignal()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}", true));

    QSignalSpy spy(&store, &GraphDataStore::expressionChanged);
    store.setExpression(GraphIdx(0), "${h5}");

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<GraphIdx>(), GraphIdx(0));
    QCOMPARE(store.expression(GraphIdx(0)), QStringLiteral("${h5}"));
}

void TestGraphDataStore::setExpressionStateEmitsSignal()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}", true));

    QSignalSpy spy(&store, &GraphDataStore::expressionStateChanged);
    store.setExpressionState(GraphIdx(0), GraphData::ExpressionState::VALID);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<GraphIdx>(), GraphIdx(0));
    QCOMPARE(store.expressionState(GraphIdx(0)), GraphData::ExpressionState::VALID);
    QVERIFY(store.isExpressionValid(GraphIdx(0)));
}

void TestGraphDataStore::setLabelEmitsSignal()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}", true));

    QSignalSpy spy(&store, &GraphDataStore::labelChanged);
    store.setLabel(GraphIdx(0), "NewLabel");

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<GraphIdx>(), GraphIdx(0));
    QCOMPARE(store.label(GraphIdx(0)), QStringLiteral("NewLabel"));
}

void TestGraphDataStore::setColorEmitsSignal()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}", true));

    QSignalSpy spy(&store, &GraphDataStore::colorChanged);
    store.setColor(GraphIdx(0), QColor(Qt::blue));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<GraphIdx>(), GraphIdx(0));
    QCOMPARE(store.color(GraphIdx(0)), QColor(Qt::blue));
}

QTEST_GUILESS_MAIN(TestGraphDataStore)
