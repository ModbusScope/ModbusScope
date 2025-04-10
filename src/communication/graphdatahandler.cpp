#include "graphdatahandler.h"

#include "models/graphdatamodel.h"
#include "util/expressionparser.h"
#include "util/qmuparser.h"
#include "util/scopelogging.h"

#include <QRegularExpression>

GraphDataHandler::GraphDataHandler() :
  _pGraphDataModel(nullptr)
{

}

void GraphDataHandler::processActiveRegisters(GraphDataModel* pGraphDataModel)
{
    QStringList exprList;

    _pGraphDataModel = pGraphDataModel;

    _pGraphDataModel->activeGraphIndexList(&_activeIndexList);
    for(quint16 graphIdx: std::as_const(_activeIndexList))
    {
        exprList.append(_pGraphDataModel->expression(graphIdx));
    }

    ExpressionParser exprParser(exprList);
    exprParser.modbusRegisters(_registerList);

    qCInfo(scopeComm) << "Active registers: " << ModbusRegister::dumpListToString(_registerList);

    QStringList processedExpList;
    exprParser.processedExpressions(processedExpList);

    _valueParsers.clear();

    for(const QString &expr: std::as_const(processedExpList))
    {
        _valueParsers.append(QMuParser(expr));
    }
}

void GraphDataHandler::modbusRegisterList(QList<ModbusRegister>& registerList)
{
    registerList = _registerList;
}

QString GraphDataHandler::expressionParseMsg(qint32 exprIdx) const
{
    if (exprIdx >= _valueParsers.size())
    {
        return QString();
    }

    return _valueParsers[exprIdx].msg();
}

qint32 GraphDataHandler::expressionErrorPos(qint32 exprIdx) const
{
    if (exprIdx >= _valueParsers.size())
    {
        return -1;
    }

    return _valueParsers[exprIdx].errorPos();
}

QMuParser::ErrorType GraphDataHandler::expressionErrorType(qint32 exprIdx) const
{
    if (exprIdx >= _valueParsers.size())
    {
        return QMuParser::ErrorType::SYNTAX;
    }

    return _valueParsers[exprIdx].errorType();
}

void GraphDataHandler::handleRegisterData(ResultDoubleList results)
{
    ResultDoubleList registerList;

    QMuParser::setRegistersData(results);

    for(qint32 listIdx = 0; listIdx < _valueParsers.size(); listIdx++)
    {
        ResultDouble result;

        if (_valueParsers[listIdx].evaluate())
        {
            result.setValue(_valueParsers[listIdx].value());
        }
        else
        {
            result.setError();

            const quint16 activeIndex = _activeIndexList[listIdx];
            auto msg = QString("Expression evaluation failed (%1): expression %2")
                        .arg(_valueParsers[listIdx].msg(), _pGraphDataModel->expression(activeIndex));

            qCWarning(scopeComm) << msg;
        }

        registerList.append(result);
    }

    emit graphDataReady(registerList);
}



