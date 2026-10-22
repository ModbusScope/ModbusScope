#include "tst_signallistdraft.h"

#include "models/graphdatamodel.h"
#include "models/signallistdraft.h"

#include <QMimeData>
#include <QSignalSpy>
#include <QTest>

void TestSignalListDraft::init()
{
    _pSource = new GraphDataModel();
}

void TestSignalListDraft::cleanup()
{
    delete _pSource;
    _pSource = nullptr;
}

//! Add one active signal per label, each with its own data point and three samples.
void TestSignalListDraft::seedSignals(const QStringList& labelList)
{
    for (const QString& label : labelList)
    {
        GraphData data;
        data.setLabel(label);
        data.setExpression(QString("${%1}").arg(40001 + _pSource->size()));
        _pSource->add(data);

        const GraphIdx idx(_pSource->size() - 1);
        _pSource->mutableDataSeries(idx)->setSamples({ 0, 1, 2 }, { 10, 11, 12 });
    }
}

QStringList TestSignalListDraft::labels(const GraphDataModel* pModel)
{
    QStringList result;
    for (qint32 idx = 0; idx < pModel->size(); idx++)
    {
        result.append(pModel->label(GraphIdx(idx)));
    }
    return result;
}

//! Move a row the way the table view does on an internal drag and drop.
void TestSignalListDraft::moveByDragAndDrop(GraphDataModel* pModel, int fromRow, int dropRow)
{
    QMimeData* pMimeData = pModel->mimeData({ pModel->index(fromRow, 0) });
    pModel->dropMimeData(pMimeData, Qt::MoveAction, dropRow, 0, QModelIndex());
    delete pMimeData;
}

qsizetype TestSignalListDraft::sampleCount(int row) const
{
    return _pSource->dataSeries(GraphIdx(row))->size();
}

void TestSignalListDraft::draftCopiesSettings()
{
    seedSignals({ "a", "b" });
    _pSource->setColor(GraphIdx(1), Qt::darkGreen);
    _pSource->setValueAxis(GraphIdx(1), GraphData::VALUE_AXIS_SECONDARY);
    _pSource->setVisible(GraphIdx(1), false);
    _pSource->setActive(GraphIdx(0), false);

    SignalListDraft draft(_pSource, &_settingsModel);
    GraphDataModel* pDraft = draft.model();

    QCOMPARE(labels(pDraft), labels(_pSource));
    QCOMPARE(pDraft->expression(GraphIdx(1)), _pSource->expression(GraphIdx(1)));
    QCOMPARE(pDraft->color(GraphIdx(1)), QColor(Qt::darkGreen));
    QCOMPARE(pDraft->valueAxis(GraphIdx(1)), GraphData::VALUE_AXIS_SECONDARY);
    QCOMPARE(pDraft->isVisible(GraphIdx(1)), false);
    QCOMPARE(pDraft->isActive(GraphIdx(0)), false);
    QVERIFY(!draft.hasChanges());
}

void TestSignalListDraft::draftRowsDoNotShareSeriesWithSource()
{
    seedSignals({ "a", "b" });

    SignalListDraft draft(_pSource, &_settingsModel);

    for (qint32 row = 0; row < _pSource->size(); row++)
    {
        QVERIFY(draft.model()->dataSeries(GraphIdx(row)) != _pSource->dataSeries(GraphIdx(row)));
        QVERIFY(draft.model()->dataSeries(GraphIdx(row))->isEmpty());
    }
}

void TestSignalListDraft::applyWithoutChangesEmitsNothing()
{
    seedSignals({ "a", "b" });
    SignalListDraft draft(_pSource, &_settingsModel);
    QSignalSpy spyDataChanged(_pSource, &GraphDataModel::dataChanged);
    QSignalSpy spyInserted(_pSource, &GraphDataModel::rowsInserted);
    QSignalSpy spyRemoved(_pSource, &GraphDataModel::rowsRemoved);
    QSignalSpy spyMoved(_pSource, &GraphDataModel::rowsMoved);
    QSignalSpy spySelected(_pSource, &GraphDataModel::selectedGraphChanged);

    draft.apply();

    QCOMPARE(spyDataChanged.count(), 0);
    QCOMPARE(spyInserted.count(), 0);
    QCOMPARE(spyRemoved.count(), 0);
    QCOMPARE(spyMoved.count(), 0);
    QCOMPARE(spySelected.count(), 0);
}

void TestSignalListDraft::renameKeepsData()
{
    seedSignals({ "a", "b" });
    SignalListDraft draft(_pSource, &_settingsModel);
    draft.model()->setLabel(GraphIdx(0), "renamed");
    QSignalSpy spyLabel(_pSource, &GraphDataModel::labelChanged);
    QSignalSpy spyExpression(_pSource, &GraphDataModel::expressionChanged);
    QSignalSpy spyActive(_pSource, &GraphDataModel::activeChanged);

    draft.apply();

    QCOMPARE(labels(_pSource), QStringList({ "renamed", "b" }));
    QCOMPARE(spyLabel.count(), 1);
    QCOMPARE(spyExpression.count(), 0);
    QCOMPARE(spyActive.count(), 0);
    QCOMPARE(sampleCount(0), 3);
}

void TestSignalListDraft::renameAndBackIsNoChange()
{
    seedSignals({ "a" });
    SignalListDraft draft(_pSource, &_settingsModel);

    draft.model()->setLabel(GraphIdx(0), "renamed");
    QVERIFY(draft.hasChanges());

    draft.model()->setLabel(GraphIdx(0), "a");
    QVERIFY(!draft.hasChanges());
}

/*!
 * \brief The graph view clears the data of a signal whose expression changed, so apply() must
 * report the change for that signal only.
 */
void TestSignalListDraft::expressionChangeOnlyTouchesThatSignal()
{
    seedSignals({ "a", "b", "c" });
    SignalListDraft draft(_pSource, &_settingsModel);
    draft.model()->setExpression(GraphIdx(1), "${40010}");
    QSignalSpy spyExpression(_pSource, &GraphDataModel::expressionChanged);

    draft.apply();

    QCOMPARE(_pSource->expression(GraphIdx(1)), QString("${40010}"));
    QCOMPARE(spyExpression.count(), 1);
    QCOMPARE(spyExpression.first().first().value<GraphIdx>(), GraphIdx(1));
    QCOMPARE(sampleCount(0), 3);
    QCOMPARE(sampleCount(2), 3);
}

void TestSignalListDraft::deactivateThenDiscardKeepsSourceData()
{
    seedSignals({ "a" });
    QSignalSpy spyDataChanged(_pSource, &GraphDataModel::dataChanged);

    {
        SignalListDraft draft(_pSource, &_settingsModel);
        draft.model()->setActive(GraphIdx(0), false);
    }

    QVERIFY(_pSource->isActive(GraphIdx(0)));
    QCOMPARE(sampleCount(0), 3);
    QCOMPARE(spyDataChanged.count(), 0);
}

void TestSignalListDraft::deactivateThenApplyClearsSourceData()
{
    seedSignals({ "a", "b" });
    SignalListDraft draft(_pSource, &_settingsModel);
    draft.model()->setActive(GraphIdx(0), false);

    draft.apply();

    QVERIFY(!_pSource->isActive(GraphIdx(0)));
    QCOMPARE(sampleCount(0), 0);
    QCOMPARE(sampleCount(1), 3);
}

void TestSignalListDraft::deactivateAndReactivateIsNoChange()
{
    seedSignals({ "a" });
    SignalListDraft draft(_pSource, &_settingsModel);
    draft.model()->setActive(GraphIdx(0), false);
    draft.model()->setActive(GraphIdx(0), true);

    QVERIFY(!draft.hasChanges());
    draft.apply();

    QCOMPARE(sampleCount(0), 3);
}

void TestSignalListDraft::addAppendsSignal()
{
    seedSignals({ "a" });
    SignalListDraft draft(_pSource, &_settingsModel);
    GraphData data;
    data.setLabel("new");
    data.setExpression("${40020}");
    draft.model()->add(data);
    QCOMPARE(_pSource->size(), 1);

    draft.apply();

    QCOMPARE(labels(_pSource), QStringList({ "a", "new" }));
    QCOMPARE(_pSource->expression(GraphIdx(1)), QString("${40020}"));
    QCOMPARE(_pSource->color(GraphIdx(1)), draft.model()->color(GraphIdx(1)));
    QCOMPARE(sampleCount(0), 3);
}

void TestSignalListDraft::removeRemovesSignalAndKeepsOthersData()
{
    seedSignals({ "a", "b", "c" });
    SignalListDraft draft(_pSource, &_settingsModel);
    draft.model()->removeSignals({ GraphIdx(1) });
    QCOMPARE(_pSource->size(), 3);

    draft.apply();

    QCOMPARE(labels(_pSource), QStringList({ "a", "c" }));
    QCOMPARE(sampleCount(0), 3);
    QCOMPARE(sampleCount(1), 3);
}

void TestSignalListDraft::dragAndDropMoveThenApply()
{
    seedSignals({ "a", "b", "c" });
    SignalListDraft draft(_pSource, &_settingsModel);

    /* Drop "a" below "c" */
    moveByDragAndDrop(draft.model(), 0, 3);
    QCOMPARE(labels(draft.model()), QStringList({ "b", "c", "a" }));

    draft.apply();

    QCOMPARE(labels(_pSource), QStringList({ "b", "c", "a" }));
    QCOMPARE(_pSource->expression(GraphIdx(2)), QString("${40001}"));
    QCOMPARE(sampleCount(2), 3);
}

void TestSignalListDraft::combinedChangesFollowDraft()
{
    seedSignals({ "a", "b", "c", "d" });
    SignalListDraft draft(_pSource, &_settingsModel);
    GraphDataModel* pDraft = draft.model();

    pDraft->removeSignals({ GraphIdx(1) }); // a c d
    GraphData data;
    data.setLabel("new");
    pDraft->add(data);               // a c d new
    moveByDragAndDrop(pDraft, 3, 0); // new a c d
    moveByDragAndDrop(pDraft, 3, 1); // new d a c
    pDraft->setLabel(GraphIdx(2), "a2");
    pDraft->setExpression(GraphIdx(3), "${40030}");

    draft.apply();

    QCOMPARE(labels(_pSource), QStringList({ "new", "d", "a2", "c" }));
    QCOMPARE(_pSource->expression(GraphIdx(1)), QString("${40004}"));
    QCOMPARE(_pSource->expression(GraphIdx(3)), QString("${40030}"));
    QCOMPARE(sampleCount(0), 0);
    QCOMPARE(sampleCount(1), 3);
    QCOMPARE(sampleCount(2), 3);
    QVERIFY(!draft.hasChanges());
}

void TestSignalListDraft::secondApplyIsNoChange()
{
    seedSignals({ "a", "b" });
    SignalListDraft draft(_pSource, &_settingsModel);
    draft.model()->removeSignals({ GraphIdx(0) });
    draft.apply();
    QSignalSpy spyRemoved(_pSource, &GraphDataModel::rowsRemoved);

    draft.apply();

    QCOMPARE(labels(_pSource), QStringList({ "b" }));
    QCOMPARE(spyRemoved.count(), 0);
}

void TestSignalListDraft::draftExpressionStateFollowsEdits()
{
    seedSignals({ "a" });
    SignalListDraft draft(_pSource, &_settingsModel);

    draft.model()->setExpression(GraphIdx(0), "${40001} +");

    QCOMPARE(draft.model()->expressionState(GraphIdx(0)), GraphData::ExpressionState::SYNTAX_ERROR);
}

void TestSignalListDraft::moveAndMoveBackIsNoChange()
{
    seedSignals({ "a", "b", "c", "d" });
    SignalListDraft draft(_pSource, &_settingsModel);

    moveByDragAndDrop(draft.model(), 0, 2); // b a c d
    QVERIFY(draft.hasChanges());
    moveByDragAndDrop(draft.model(), 1, 0); // a b c d

    QCOMPARE(labels(draft.model()), QStringList({ "a", "b", "c", "d" }));
    QVERIFY(!draft.hasChanges());
}

void TestSignalListDraft::adjacentDropIsNoChange()
{
    seedSignals({ "a", "b", "c" });
    SignalListDraft draft(_pSource, &_settingsModel);

    moveByDragAndDrop(draft.model(), 1, 1);
    moveByDragAndDrop(draft.model(), 1, 2);

    QCOMPARE(labels(draft.model()), QStringList({ "a", "b", "c" }));
    QVERIFY(!draft.hasChanges());
}

void TestSignalListDraft::reorderOnlyKeepsData()
{
    seedSignals({ "a", "b", "c", "d" });
    SignalListDraft draft(_pSource, &_settingsModel);
    moveByDragAndDrop(draft.model(), 0, 2);  // b a c d
    moveByDragAndDrop(draft.model(), 3, -1); // b a c d (drop at end)
    moveByDragAndDrop(draft.model(), 3, 0);  // d b a c
    QSignalSpy spyExpression(_pSource, &GraphDataModel::expressionChanged);

    draft.apply();

    QCOMPARE(labels(_pSource), QStringList({ "d", "b", "a", "c" }));
    QCOMPARE(_pSource->expression(GraphIdx(0)), QString("${40004}"));
    QCOMPARE(spyExpression.count(), 0);
    for (qint32 row = 0; row < _pSource->size(); row++)
    {
        QCOMPARE(sampleCount(row), 3);
    }
}

void TestSignalListDraft::colorAxisAndVisibilityAreApplied()
{
    seedSignals({ "a", "b", "c" });
    SignalListDraft draft(_pSource, &_settingsModel);
    draft.model()->setColor(GraphIdx(0), Qt::magenta);
    draft.model()->setValueAxis(GraphIdx(1), GraphData::VALUE_AXIS_SECONDARY);
    draft.model()->setVisible(GraphIdx(2), false);
    QVERIFY(draft.hasChanges());

    draft.apply();

    QCOMPARE(_pSource->color(GraphIdx(0)), QColor(Qt::magenta));
    QCOMPARE(_pSource->valueAxis(GraphIdx(1)), GraphData::VALUE_AXIS_SECONDARY);
    QVERIFY(!_pSource->isVisible(GraphIdx(2)));
    QCOMPARE(sampleCount(2), 3);
}

/*!
 * \brief Activating a signal makes it visible, so a signal activated but hidden in the draft must
 * end up active and hidden.
 */
void TestSignalListDraft::activatedHiddenSignalStaysHidden()
{
    seedSignals({ "a" });
    _pSource->setActive(GraphIdx(0), false);
    _pSource->setVisible(GraphIdx(0), false);
    SignalListDraft draft(_pSource, &_settingsModel);
    draft.model()->setActive(GraphIdx(0), true);
    draft.model()->setVisible(GraphIdx(0), false);

    draft.apply();

    QVERIFY(_pSource->isActive(GraphIdx(0)));
    QVERIFY(!_pSource->isVisible(GraphIdx(0)));
}

void TestSignalListDraft::editAfterApplyIsApplied()
{
    seedSignals({ "a", "b" });
    SignalListDraft draft(_pSource, &_settingsModel);
    GraphData data;
    data.setLabel("new");
    draft.model()->add(data);
    draft.apply();

    draft.model()->removeSignals({ GraphIdx(0) });
    draft.model()->setLabel(GraphIdx(1), "new2");
    draft.apply();

    QCOMPARE(labels(_pSource), QStringList({ "b", "new2" }));
    QVERIFY(draft.model()->dataSeries(GraphIdx(1)) != _pSource->dataSeries(GraphIdx(1)));
    QVERIFY(!draft.hasChanges());
}

void TestSignalListDraft::emptyLabelIsKept()
{
    seedSignals({ "a" });
    _pSource->setLabel(GraphIdx(0), QString());

    SignalListDraft draft(_pSource, &_settingsModel);

    QCOMPARE(draft.model()->label(GraphIdx(0)), QString());
    QVERIFY(!draft.hasChanges());
}

QTEST_GUILESS_MAIN(TestSignalListDraft)
