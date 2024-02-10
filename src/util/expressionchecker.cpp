
#include "expressionchecker.h"

ExpressionChecker::ExpressionChecker(QObject *parent) : QObject(parent)
{
    connect(&_graphDataHandler, &GraphDataHandler::graphDataReady, this, &ExpressionChecker::handleDataReady);
}

void ExpressionChecker::checkExpression(QString expr)
{
    _localGraphDataModel.clear();
    _localGraphDataModel.add();
    _localGraphDataModel.setExpression(0, expr);

    _graphDataHandler.processActiveRegisters(&_localGraphDataModel);

    QList<ModbusRegister> registerList;
    _graphDataHandler.modbusRegisterList(registerList);

    _descriptions.clear();
    for(ModbusRegister& reg : registerList)
    {
        _descriptions.append(reg.description());
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

quint32 ExpressionChecker::requiredValueCount()
{
    return _descriptions.size();
}

void ExpressionChecker::setValues(ResultDoubleList results)
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
    _bSyntaxError = _graphDataHandler.expressionErrorType(0) == QMuParser::ErrorType::SYNTAX_ERROR;

    emit resultsReady(_bValid);
}


