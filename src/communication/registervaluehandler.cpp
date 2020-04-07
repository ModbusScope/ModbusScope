#include "registervaluehandler.h"

#include "graphdatamodel.h"
#include "settingsmodel.h"

RegisterValueHandler::RegisterValueHandler(GraphDataModel *pGraphDataModel, SettingsModel* pSettingsModel)
{
    _pGraphDataModel = pGraphDataModel;
    _pSettingsModel = pSettingsModel;
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

                    qint64 combinedValueToProcess = 0;
                    if (bSuccess)
                    {
                        if (_pGraphDataModel->isBit32(activeIndex))
                        {
                            ModbusResult nextResult = i.peekNext().value();

                            uint32_t combinedValue;
                            if (_pSettingsModel->int32LittleEndian(connectionId))
                            {
                                combinedValue = (static_cast<uint32_t>(nextResult.value()) << 16) | value;
                            }
                            else
                            {
                                combinedValue = (static_cast<uint32_t>(value) << 16) | nextResult.value();
                            }

                            if (nextResult.isSuccess())
                            {
                                if (_pGraphDataModel->isUnsigned(activeIndex))
                                {
                                    combinedValueToProcess = static_cast<qint64>(static_cast<quint32>(combinedValue));
                                }
                                else
                                {
                                    combinedValueToProcess = static_cast<qint64>(static_cast<qint32>(combinedValue));
                                }
                            }
                            else
                            {
                                bSuccess = false;
                                combinedValueToProcess = 0;
                            }
                        }
                        else
                        {
                            if (_pGraphDataModel->isUnsigned(activeIndex))
                            {
                                combinedValueToProcess = static_cast<qint64>(static_cast<quint16>(value));
                            }
                            else
                            {
                                combinedValueToProcess = static_cast<qint64>(static_cast<qint16>(value));
                            }
                        }
                    }
                    else
                    {
                        combinedValueToProcess = 0;
                    }

                    double processedResult = 0;
                    if (bSuccess)
                    {
                        processedResult = processValue(activeIndex, combinedValueToProcess);
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

double RegisterValueHandler::processValue(quint32 graphIndex, qint64 value)
{
    double processedValue = value;

    if (_pGraphDataModel->isUnsigned(graphIndex))
    {
        // Apply bitmask
        processedValue = static_cast<quint32>(processedValue) & _pGraphDataModel->bitmask(graphIndex);
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
