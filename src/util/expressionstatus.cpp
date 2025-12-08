
#include "expressionstatus.h"

#include "models/graphdatamodel.h"
#include "models/settingsmodel.h"

using State = ResultState::State;
using ExpressionState = GraphData::ExpressionStatus;

ExpressionStatus::ExpressionStatus(GraphDataModel* pGraphDataModel, SettingsModel* pSettingsModel, QObject* parent)
    : QObject(parent), _checker(parent), _pGraphDataModel(pGraphDataModel), _pSettingsModel(pSettingsModel)
{
    connect(_pGraphDataModel, &GraphDataModel::added, this, &ExpressionStatus::handlExpressionsChanged);
    connect(_pGraphDataModel, &GraphDataModel::expressionChanged, this, &ExpressionStatus::handlExpressionsChanged);

    connect(&_checker, &ExpressionChecker::resultsReady, this, &ExpressionStatus::handleResultReady);
}

void ExpressionStatus::handleResultReady(bool valid)
{
    QString verifiedExpression = _expressionQueue.dequeue();

    const qint32 size = _pGraphDataModel->size();
    for (qint32 idx = 0; idx < size; idx++)
    {
        if (_pGraphDataModel->expression(idx) == verifiedExpression)
        {
            bool bStatus = valid;

            if (!bStatus && !_checker.syntaxError())
            {
                /* Ignore value errors (for example divide by 0) */
                bStatus = true;
            }

            if (_pGraphDataModel->expressionStatus(idx) == ExpressionState::UNKNOWN ||
                _pGraphDataModel->expressionStatus(idx) == ExpressionState::SYNTAX_ERROR)
            {
                _pGraphDataModel->setExpressionStatus(idx,
                                                      bStatus ? ExpressionState::VALID : ExpressionState::SYNTAX_ERROR);
            }
            break;
        }
    }

    if (!_expressionQueue.isEmpty())
    {
        verifyExpression(_expressionQueue.head(), _pSettingsModel->deviceList());
    }
}

void ExpressionStatus::handlExpressionsChanged(const quint32 graphIdx)
{
    QString expression = _pGraphDataModel->expression(graphIdx);

    bool bStartChecker = false;
    if (_expressionQueue.isEmpty())
    {
        bStartChecker = true;
    }
    _expressionQueue.enqueue(expression);

    if (bStartChecker)
    {
        verifyExpression(expression, _pSettingsModel->deviceList());
    }
}

void ExpressionStatus::verifyExpression(QString const& expression, QList<deviceId_t> const& deviceIdList)
{
    Q_UNUSED(deviceIdList);

    _checker.setExpression(expression);

    const auto count = _checker.requiredValueCount();

    ResultDoubleList valueList;
    while(valueList.count() < count)
    {
        valueList.append(ResultDouble(1, State::SUCCESS));
    }

    _checker.checkWithValues(valueList);
}
