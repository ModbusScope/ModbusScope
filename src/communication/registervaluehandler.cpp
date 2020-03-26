#include "registervaluehandler.h"

#include "graphdatamodel.h"

RegisterValueHandler::RegisterValueHandler(GraphDataModel *pGraphDataModel)
{
    _pGraphDataModel = pGraphDataModel;
}

void RegisterValueHandler::startRead()
{
    /* Prepare result lists */
    _processedValues.clear();
    _successList.clear();
    _pGraphDataModel->activeGraphIndexList(&_activeIndexList);

    for(int idx = 0; idx < _activeIndexList.size(); idx++)
    {
        _processedValues.append(0);
        _successList.append(false);
    }
}

void RegisterValueHandler::processPartialResult(QMap<quint16, ModbusResult> partialResultMap, quint8 connectionId)
{
    QMapIterator<quint16, ModbusResult> i(partialResultMap);
    while (i.hasNext())
    {
        i.next();

        for(quint16 listIdx = 0; listIdx < _activeIndexList.size(); listIdx++)
        {
            const quint16 activeIndex = _activeIndexList[listIdx];
            if (_pGraphDataModel->connectionId(activeIndex) == connectionId)
            {
                if (_pGraphDataModel->registerAddress(activeIndex) == i.key())
                {
                    bool bSuccess = i.value().isSuccess();
                    uint16_t value = static_cast<uint16_t>(i.value().value());
                    double processedResult = 0;

                    if (_pGraphDataModel->isBit32(activeIndex))
                    {

                        ModbusResult nextResult = i.peekNext().value();

                        /* TODO: use connection endiannes settings */
                        uint32_t combinedValue = static_cast<uint32_t>(nextResult.value()) | value;

                        if (nextResult.isSuccess())
                        {
                            processedResult = processValue(activeIndex, combinedValue);
                        }
                        else
                        {
                            bSuccess = false;
                            processedResult = 0;
                        }
                    }
                    else
                    {
                        processedResult = processValue(activeIndex, value);
                    }

                    _processedValues[listIdx] = processedResult;
                    _successList[listIdx] = bSuccess;
                }
            }
        }
    }
}

QList<double> RegisterValueHandler::processedValues()
{
    return _processedValues;
}

QList<bool> RegisterValueHandler::successList()
{
    return _successList;
}

// Get sorted list of active (unique) register addresses for a specific connection id
void RegisterValueHandler::activeGraphAddresList(QList<quint16> * pRegisterList, quint8 connectionId)
{
    // Clear list
    pRegisterList->clear();

    foreach(quint32 idx, _activeIndexList)
    {
        if (_pGraphDataModel->connectionId(idx) == connectionId)
        {
            if (!pRegisterList->contains(_pGraphDataModel->registerAddress(idx)))
            {
                pRegisterList->append(_pGraphDataModel->registerAddress(idx));
            }

            /* When reading 32 bit value, also read next address */
            if (_pGraphDataModel->isBit32(idx))
            {
                const uint16_t reg = _pGraphDataModel->registerAddress(idx) + 1;
                if (!pRegisterList->contains(reg))
                {
                    pRegisterList->append(reg);
                }
            }
        }
    }

    // sort qList
    qSort(*pRegisterList);
}

double RegisterValueHandler::processValue(quint32 graphIndex, quint32 value)
{

    /* TODO: Rework and unit test fully !!! */
    /* 16 bit vs 32 bit */

    double processedValue = 0;

    if (_pGraphDataModel->isUnsigned(graphIndex))
    {
        processedValue = value;
    }
    else
    {
        processedValue = static_cast<qint32>(value);
    }

    // Apply bitmask
    if (_pGraphDataModel->isUnsigned(graphIndex))
    {
        processedValue = static_cast<quint32>(processedValue) & _pGraphDataModel->bitmask(graphIndex);
    }
    else
    {
        processedValue = static_cast<qint32>(static_cast<qint32>(processedValue) & _pGraphDataModel->bitmask(graphIndex));
    }

    // Apply shift
    if (_pGraphDataModel->shift(graphIndex) != 0)
    {
        if (_pGraphDataModel->shift(graphIndex) > 0)
        {
            processedValue = static_cast<qint32>(processedValue) << _pGraphDataModel->shift(graphIndex);
        }
        else
        {
            processedValue = static_cast<qint32>(processedValue) >> abs(_pGraphDataModel->shift(graphIndex));
        }

        if (!_pGraphDataModel->isUnsigned(graphIndex))
        {
            processedValue = static_cast<qint32>(processedValue);
        }
    }

    // Apply multiplyFactor
    processedValue *= _pGraphDataModel->multiplyFactor(graphIndex);

    // Apply divideFactor
    processedValue /= _pGraphDataModel->divideFactor(graphIndex);

    return processedValue;
}
