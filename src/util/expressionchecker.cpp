
#include "expressionchecker.h"

ExpressionChecker::ExpressionChecker(QObject* parent) : QObject(parent)
{
    connect(&_graphDataHandler, &GraphDataHandler::graphDataReady, this, &ExpressionChecker::handleDataReady);
}

void ExpressionChecker::setExpression(QString expr)
{
    _localGraphDataModel.clear();
    _localGraphDataModel.add();
    _localGraphDataModel.setExpression(0, expr);

    QList<ModbusRegister> registerList;
    _graphDataHandler.setupExpressions(&_localGraphDataModel, registerList);

    _expectedDeviceIdList.clear();
    _descriptions.clear();
    for (ModbusRegister const& reg : std::as_const(registerList))
    {
        _descriptions.append(reg.description());

        if (!_expectedDeviceIdList.contains(reg.deviceId()))
        {
            _expectedDeviceIdList.append(reg.deviceId());
        }
    }
}

QString ExpressionChecker::expression(void)
{
    if (_localGraphDataModel.size() > 0)
    {
        return _localGraphDataModel.expression(0);
    }
    else
    {
        return QString();
    }
}

void ExpressionChecker::descriptions(QStringList& descr)
{
    descr = _descriptions;
}

qsizetype ExpressionChecker::requiredValueCount()
{
    return _descriptions.size();
}

bool ExpressionChecker::checkForDevices(QList<deviceId_t> const& deviceIdList)
{
    /* Verify if all required devices exist */
    for (deviceId_t reqDevId : std::as_const(_expectedDeviceIdList))
    {
        if (!deviceIdList.contains(reqDevId))
        {
            return false;
        }
    }

    return true;
}

void ExpressionChecker::checkWithValues(ResultDoubleList results)
{
    _graphDataHandler.handleRegisterData(results);
}

bool ExpressionChecker::isValid()
{
    return _bValid;
}

double ExpressionChecker::result()
{
    return _result;
}

QString ExpressionChecker::strError()
{
    return _strError;
}

qint32 ExpressionChecker::errorPos()
{
    return _errorPos;
}

bool ExpressionChecker::syntaxError()
{
    return _bSyntaxError;
}

void ExpressionChecker::handleDataReady(ResultDoubleList resultList)
{
    _bValid = !resultList.isEmpty() && resultList.first().isValid();

    if (_bValid)
    {
        _result = resultList.first().value();
        _strError = QString();
    }
    else
    {
        _result = 0;
        _strError = _graphDataHandler.expressionParseMsg(0);
    }

    _errorPos = _graphDataHandler.expressionErrorPos(0);
    _bSyntaxError = _graphDataHandler.expressionErrorType(0) == QMuParser::ErrorType::SYNTAX;

    emit resultsReady(_bValid);
}
