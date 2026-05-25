
#include "tst_expressionstatus.h"

#include "models/graphdata.h"
#include "models/graphdatamodel.h"
#include "models/settingsmodel.h"
#include "util/expressionstatus.h"
#include "util/graphindex.h"

#include <QTest>

using ExpressionState = GraphData::ExpressionState;

void TestExpressionStatus::init()
{
    _pGraphDataModel = new GraphDataModel(this);
    _pSettingsModel = new SettingsModel(this);
}

void TestExpressionStatus::cleanup()
{
    delete _pGraphDataModel;
    _pGraphDataModel = nullptr;
    delete _pSettingsModel;
    _pSettingsModel = nullptr;
}

void TestExpressionStatus::validConstantExpression_setsValidState()
{
    ExpressionStatus status(_pGraphDataModel, _pSettingsModel);

    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(GraphIdx(0), QStringLiteral("1+1"));

    QCOMPARE(_pGraphDataModel->expressionState(GraphIdx(0)), ExpressionState::VALID);
}

void TestExpressionStatus::syntaxErrorExpression_setsSyntaxErrorState()
{
    ExpressionStatus status(_pGraphDataModel, _pSettingsModel);

    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(GraphIdx(0), QStringLiteral("1++"));

    QCOMPARE(_pGraphDataModel->expressionState(GraphIdx(0)), ExpressionState::SYNTAX_ERROR);
}

void TestExpressionStatus::registerExpression_noDevice_setsUnknownDeviceState()
{
    /* SettingsModel has only device 1 by default. An expression referencing device 2
       fails the device check and must produce UNKNOWN_DEVICE. */
    ExpressionStatus status(_pGraphDataModel, _pSettingsModel);

    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(GraphIdx(0), QStringLiteral("${40001@2}"));

    QCOMPARE(_pGraphDataModel->expressionState(GraphIdx(0)), ExpressionState::UNKNOWN_DEVICE);
}

void TestExpressionStatus::afterRemoval_subsequentExpressionChange_setsCorrectState()
{
    /* The staleness guard in handleResultReady drops any queued result whose stored
       expression no longer matches the model at that index.  With the current
       synchronous ExpressionChecker the guard cannot be triggered directly because
       checkWithValues() fires resultsReady() inline, leaving no window between
       enqueue and dequeue for a model mutation.

       This test instead verifies the behavioral invariant: after removeFromModel()
       slides graph 1 down to index 0, a subsequent expression change on the new
       graph at index 0 produces the correct state — not a state carried over from
       the now-removed graph.  A future async checker would expose the same invariant
       through the guard logic. */

    ExpressionStatus status(_pGraphDataModel, _pSettingsModel);

    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(GraphIdx(0), QStringLiteral("1+1"));
    QCOMPARE(_pGraphDataModel->expressionState(GraphIdx(0)), ExpressionState::VALID);

    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(GraphIdx(1), QStringLiteral("2+2"));
    QCOMPARE(_pGraphDataModel->expressionState(GraphIdx(1)), ExpressionState::VALID);

    /* Remove graph 0; graph 1 slides to index 0. */
    _pGraphDataModel->removeRows(0, 1, QModelIndex());
    QCOMPARE(_pGraphDataModel->rowCount(), 1);

    /* The remaining graph (now at index 0) was "2+2"; change it to a syntax error
       and verify the state is updated correctly — not silently dropped by a stale
       guard that would incorrectly compare "1+1" against "2+2". */
    _pGraphDataModel->setExpression(GraphIdx(0), QStringLiteral("bad++"));
    QCOMPARE(_pGraphDataModel->expressionState(GraphIdx(0)), ExpressionState::SYNTAX_ERROR);
}

QTEST_GUILESS_MAIN(TestExpressionStatus)
