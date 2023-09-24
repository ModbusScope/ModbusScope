
#include "expressionchecker.h"

ExpressionChecker::ExpressionChecker(QObject *parent) : QObject(parent)
{
    connect(&_graphDataHandler, &GraphDataHandler::graphDataReady, this, &ExpressionChecker::handleDataReady);
}

bool ExpressionChecker::parseExpression(QString expr)
{
    if (
        (_localGraphDataModel.size() == 0)
        || (_localGraphDataModel.expression(0) != expr)
    )
    {
        _localGraphDataModel.clear();
        _localGraphDataModel.add();
        _localGraphDataModel.setExpression(0, expr);

        _graphDataHandler.processActiveRegisters(&_localGraphDataModel);

        QList<ModbusRegister> registerList;
        _graphDataHandler.modbusRegisterList(registerList);

        return true;
    }
    else
    {
        return false;
    }
}

void ExpressionChecker::descriptions(QStringList& descr)
{
    QList<ModbusRegister> registerList;
    _graphDataHandler.modbusRegisterList(registerList);

    descr.clear();
    for(ModbusRegister& reg : registerList)
    {
        descr.append(reg.description());
    }
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

    emit resultsReady();
}


