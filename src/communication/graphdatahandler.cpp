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
    qDeleteAll(_valueParsers);
    _valueParsers.clear();

    _pGraphDataModel->activeGraphIndexList(&_activeIndexList);
    for(quint16 graphIdx: qAsConst(_activeIndexList))
    {
        QString expr = _pGraphDataModel->expression(graphIdx);

        registerList.clear();

        //QString processedExpr = processExpression(registerList, expr);

        /* Use pointer because our class otherwise needs copy/assignment constructor and such */
        /* Remember to delete before removal */
        //_valueParsers.append(new QMuParser(processedExpr));
    }
}

void GraphDataHandler::handleRegisterData(QList<ModbusResult> results)
{
    QList<bool> graphSuccess;
    QList<double> graphData;

    for(qint32 listIdx = 0; listIdx < results.size(); listIdx++)
    {
        double processedResult = 0;
        bool bSuccess = results[listIdx].isSuccess();
        const double registerValue = results[listIdx].value();
        if (bSuccess)
        {
#if 0
            TODO
            if (_valueParsers[listIdx]->evaluate(registerValue))
            {
                processedResult = _valueParsers[listIdx]->result();
            }
            else
#endif
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



