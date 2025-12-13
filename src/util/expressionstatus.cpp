
#include "expressionstatus.h"

#include "models/graphdatamodel.h"
#include "models/settingsmodel.h"

using State = ResultState::State;
using ExpressionState = GraphData::ExpressionState;

ExpressionStatus::ExpressionStatus(GraphDataModel* pGraphDataModel, SettingsModel* pSettingsModel, QObject* parent)
    : QObject(parent),
      _checker(parent),
      _deviceCheckPassed(true),
      _pGraphDataModel(pGraphDataModel),
      _pSettingsModel(pSettingsModel)
{
    connect(_pGraphDataModel, &GraphDataModel::added, this, &ExpressionStatus::handlExpressionsChanged);
    connect(_pGraphDataModel, &GraphDataModel::expressionChanged, this, &ExpressionStatus::handlExpressionsChanged);

    connect(&_checker, &ExpressionChecker::resultsReady, this, &ExpressionStatus::handleResultReady);
}

void ExpressionStatus::handleResultReady(bool valid)
{
    /* valid is a combination of SYNTAX and OTHER (value), but we're only interested
        in SYNTAX here. We can use syntaxError() to determine that.
    */
    Q_UNUSED(valid);

    QString verifiedExpression = _expressionQueue.dequeue();

    const qint32 size = _pGraphDataModel->size();
    for (qint32 idx = 0; idx < size; idx++)
    {
        if (_pGraphDataModel->expression(idx) == verifiedExpression)
        {
            if (_checker.syntaxError())
            {
                _pGraphDataModel->setExpressionState(idx, ExpressionState::SYNTAX_ERROR);
            }
            else if (!_deviceCheckPassed)
            {
                _pGraphDataModel->setExpressionState(idx, ExpressionState::UNKNOWN_DEVICE);
            }
            else
            {
                _pGraphDataModel->setExpressionState(idx, ExpressionState::VALID);
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
    _checker.setExpression(expression);

    _deviceCheckPassed = _checker.checkForDevices(deviceIdList);

    const auto count = _checker.requiredValueCount();
    ResultDoubleList valueList;
    while(valueList.count() < count)
    {
        valueList.append(ResultDouble(1, State::SUCCESS));
    }

    _checker.checkWithValues(valueList);
}
