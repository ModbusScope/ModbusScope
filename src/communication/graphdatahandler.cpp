#include "graphdatahandler.h"

#include "qmuparser.h"
#include "graphdatamodel.h"
#include "expressionparser.h"

#include "scopelogging.h"

GraphDataHandler::GraphDataHandler(GraphDataModel* pGraphDataModel) :
  _pGraphDataModel(pGraphDataModel)
{

}

void GraphDataHandler::modbusRegisterList(QList<ModbusRegister>& registerList)
{
    QStringList exprList;

    _pGraphDataModel->activeGraphIndexList(&_activeIndexList);
    for(quint16 graphIdx: qAsConst(_activeIndexList))
    {
        exprList.append(_pGraphDataModel->expression(graphIdx));
    }

    ExpressionParser exprParser(exprList);
    exprParser.modbusRegisters(registerList);

    QStringList processedExpList;
    exprParser.processedExpressions(processedExpList);

    qDeleteAll(_valueParsers);
    _valueParsers.clear();

    for(const QString &expr: qAsConst(processedExpList))
    {
        /* Use pointer because our class otherwise needs copy/assignment constructor and such */
        /* Remember to delete before removal */
        _valueParsers.append(new QMuParser(expr));
    }
}

void GraphDataHandler::handleRegisterData(QList<ModbusResult> results)
{
    QList<bool> graphSuccess;
    QList<double> graphData;

    QList<double> registerResults;
    for(const ModbusResult &result: qAsConst(results))
    {
        double data = result.isSuccess() ? result.value(): 0;
        registerResults.append(data);
    }

    QMuParser::setRegisterValues(registerResults);

    for(qint32 listIdx = 0; listIdx < results.size(); listIdx++)
    {
        double processedResult = 0;
        bool bSuccess = results[listIdx].isSuccess();
        const double registerValue = results[listIdx].value();
        if (bSuccess)
        {
            if (_valueParsers[listIdx]->evaluate())
            {
                processedResult = _valueParsers[listIdx]->result();
            }
            else
            {
                processedResult = 0u;
                bSuccess = false;

                const quint16 activeIndex = _activeIndexList[listIdx];
                auto msg = QString("Expression evaluation failed (%1): address %2, expression %3, value %4")
                            .arg(_valueParsers[listIdx]->msg())
                            .arg(_pGraphDataModel->registerAddress(activeIndex))
                            .arg(_pGraphDataModel->expression(activeIndex))
                            .arg(registerValue);

                qCWarning(scopeComm) << msg;
            }
        }

        graphSuccess.append(bSuccess);
        graphData.append(processedResult);
    }

    emit graphDataReady(graphSuccess, graphData);
}



