#include "graphdatahandler.h"

#include "scopelogging.h"
#include "qmuparser.h"
#include "graphdatamodel.h"
#include "expressionparser.h"

#include "scopelogging.h"

GraphDataHandler::GraphDataHandler() :
  _pGraphDataModel(nullptr)
{

}

void GraphDataHandler::processActiveRegisters(GraphDataModel* pGraphDataModel)
{
    QStringList exprList;

    _pGraphDataModel = pGraphDataModel;

    _pGraphDataModel->activeGraphIndexList(&_activeIndexList);
    for(quint16 graphIdx: qAsConst(_activeIndexList))
    {
        exprList.append(_pGraphDataModel->expression(graphIdx));
    }

    ExpressionParser exprParser(exprList);
    exprParser.modbusRegisters(_registerList);

    qCInfo(scopeComm) << "Active registers: " << ModbusRegister::dumpListToString(_registerList);

    QStringList processedExpList;
    exprParser.processedExpressions(processedExpList);

    _valueParsers.clear();

    for(const QString &expr: qAsConst(processedExpList))
    {
        /* Use pointer because our class otherwise needs copy/assignment constructor and such */
        /* Remember to delete before removal */
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

void GraphDataHandler::handleRegisterData(QList<Result> results)
{
    QList<bool> graphSuccess;
    QList<double> graphData;

    QMuParser::setRegistersData(results);

    for(qint32 listIdx = 0; listIdx < _valueParsers.size(); listIdx++)
    {
        double processedResult = 0;
        bool bSuccess = true;

        if (_valueParsers[listIdx].evaluate())
        {
            processedResult = _valueParsers[listIdx].value();
        }
        else
        {
            processedResult = 0u;
            bSuccess = false;

            const quint16 activeIndex = _activeIndexList[listIdx];
            auto msg = QString("Expression evaluation failed (%1): expression %2")
                        .arg(_valueParsers[listIdx].msg(), _pGraphDataModel->expression(activeIndex));

            qCWarning(scopeComm) << msg;
        }

        graphSuccess.append(bSuccess);
        graphData.append(processedResult);
    }

    emit graphDataReady(graphSuccess, graphData);
}



