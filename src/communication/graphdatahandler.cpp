#include "graphdatahandler.h"

#include "qmuparser.h"
#include "graphdatamodel.h"
#include "settingsmodel.h"

#include "scopelogging.h"

GraphDataHandler::GraphDataHandler(GraphDataModel* pGraphDataModel, SettingsModel *pSettingsModel) :
  _pGraphDataModel(pGraphDataModel), _pSettingsModel(pSettingsModel)
{

}

void GraphDataHandler::prepareForData()
{
    qDeleteAll(_valueParsers);
    _valueParsers.clear();

    _pGraphDataModel->activeGraphIndexList(&_activeIndexList);
    for(int idx = 0; idx < _activeIndexList.size(); idx++)
    {
        /* Use pointer because our class otherwise needs copy/assignment constructor and such */
        /* Remember to delete before removal */
        _valueParsers.append(new QMuParser(_pGraphDataModel->expression(_activeIndexList[idx])));
    }
}

void GraphDataHandler::modbusRegisterList(QList<ModbusRegister>& registerList)
{
    registerList.clear();

    for(quint16 index: _activeIndexList)
    {
        ModbusRegister reg;

        reg.setAddress(_pGraphDataModel->registerAddress(index));
        reg.set32Bit(_pGraphDataModel->isBit32(index));
        reg.setConnectionId(_pGraphDataModel->connectionId(index));
        reg.setUnsigned(_pGraphDataModel->isUnsigned(index));

        registerList.append(reg);
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
            if (_valueParsers[listIdx]->evaluate(registerValue))
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





