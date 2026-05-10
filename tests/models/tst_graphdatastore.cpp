
#include "tst_graphdatastore.h"

#include "models/graphdata.h"
#include "models/graphdatastore.h"

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

    store.insertGraphData(makeGraph("B", "${h1}"));
    QCOMPARE(store.size(), 2);
}

void TestGraphDataStore::insertAssignsColorFromList()
{
    GraphDataStore store;
    GraphData g;
    g.setLabel("no color");
    g.setExpression("${h0}");
    // do not set a color — store should assign one
    store.insertGraphData(g);

    QVERIFY(store.color(0).isValid());
}

void TestGraphDataStore::insertPreservesExplicitColor()
{
    GraphDataStore store;
    GraphData g;
    g.setLabel("explicit color");
    g.setExpression("${h0}");
    g.setColor(QColor(Qt::red));
    store.insertGraphData(g);

    QCOMPARE(store.color(0), QColor(Qt::red));
}

void TestGraphDataStore::eraseRemovesCorrectEntry()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}"));
    store.insertGraphData(makeGraph("B", "${h1}"));
    store.insertGraphData(makeGraph("C", "${h2}"));

    store.eraseGraphDataAt(1);

    QCOMPARE(store.size(), 2);
    QCOMPARE(store.label(0), QStringLiteral("A"));
    QCOMPARE(store.label(1), QStringLiteral("C"));
}

void TestGraphDataStore::clearEmptiesStore()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}"));
    store.insertGraphData(makeGraph("B", "${h1}"));

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

    store.setActive(0, false);

    QCOMPARE(store.activeCount(), 1);
    QVERIFY(!store.isActive(0));
}

void TestGraphDataStore::setActiveFalseClearsData()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}", true));

    QSignalSpy spy(&store, &GraphDataStore::activeChanged);
    store.setActive(0, false);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toUInt(), 0u);
    QVERIFY(store.dataMap(0)->isEmpty());
}

void TestGraphDataStore::setActiveTrueRestoresVisibility()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}", true));
    store.setActive(0, false);
    store.setVisible(0, false);

    store.setActive(0, true);

    QVERIFY(store.isActive(0));
    QVERIFY(store.isVisible(0));
}

void TestGraphDataStore::activeGraphIndexListReturnsSortedIndices()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}", true));
    store.insertGraphData(makeGraph("B", "${h1}", false));
    store.insertGraphData(makeGraph("C", "${h2}", true));

    QList<quint16> list;
    store.activeGraphIndexList(&list);

    QCOMPARE(list.size(), 2);
    QCOMPARE(list.at(0), quint16(0));
    QCOMPARE(list.at(1), quint16(2));
}

void TestGraphDataStore::convertToActiveGraphIndexRoundTrip()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}", true));
    store.insertGraphData(makeGraph("B", "${h1}", false));
    store.insertGraphData(makeGraph("C", "${h2}", true));

    // graph index 2 is the second active graph (active index 1)
    QCOMPARE(store.convertToActiveGraphIndex(2), 1);
    QCOMPARE(store.convertToGraphIndex(1), 2);
}

void TestGraphDataStore::setVisibleFalseOnSelectedGraphResetsSelection()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}", true));
    store.setSelectedGraph(0);
    QCOMPARE(store.selectedGraph(), 0);

    store.setVisible(0, false);

    QCOMPARE(store.selectedGraph(), -1);
}

void TestGraphDataStore::moveGraphRowReorders()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}"));
    store.insertGraphData(makeGraph("B", "${h1}"));
    store.insertGraphData(makeGraph("C", "${h2}"));

    // QList::move(0, 2): remove A from 0, insert at 2 → [B, C, A]
    store.moveGraphRow(0, 2);

    QCOMPARE(store.label(0), QStringLiteral("B"));
    QCOMPARE(store.label(1), QStringLiteral("C"));
    QCOMPARE(store.label(2), QStringLiteral("A"));
}

void TestGraphDataStore::eraseSelectedGraphResetsSelection()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}"));
    store.insertGraphData(makeGraph("B", "${h1}"));
    store.setSelectedGraph(1);
    QCOMPARE(store.selectedGraph(), 1);

    store.eraseGraphDataAt(1);

    QCOMPARE(store.selectedGraph(), -1);
}

void TestGraphDataStore::eraseGraphBeforeSelectedResetsSelection()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}"));
    store.insertGraphData(makeGraph("B", "${h1}"));
    store.insertGraphData(makeGraph("C", "${h2}"));
    store.setSelectedGraph(2);
    QCOMPARE(store.selectedGraph(), 2);

    store.eraseGraphDataAt(0);

    QCOMPARE(store.selectedGraph(), -1);
}

void TestGraphDataStore::eraseGraphAfterSelectedResetsSelection()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}"));
    store.insertGraphData(makeGraph("B", "${h1}"));
    store.insertGraphData(makeGraph("C", "${h2}"));
    store.setSelectedGraph(0);
    QCOMPARE(store.selectedGraph(), 0);

    store.eraseGraphDataAt(2);

    QCOMPARE(store.selectedGraph(), -1);
}

void TestGraphDataStore::clearAllGraphDataResetsSelection()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}"));
    store.insertGraphData(makeGraph("B", "${h1}"));
    store.setSelectedGraph(1);
    QCOMPARE(store.selectedGraph(), 1);

    store.clearAllGraphData();

    QCOMPARE(store.selectedGraph(), -1);
}

void TestGraphDataStore::moveRowResetsSelection()
{
    GraphDataStore store;
    store.insertGraphData(makeGraph("A", "${h0}"));
    store.insertGraphData(makeGraph("B", "${h1}"));
    store.insertGraphData(makeGraph("C", "${h2}"));
    store.setSelectedGraph(0);
    QCOMPARE(store.selectedGraph(), 0);

    store.moveGraphRow(0, 2);

    QCOMPARE(store.selectedGraph(), -1);
}

QTEST_GUILESS_MAIN(TestGraphDataStore)
