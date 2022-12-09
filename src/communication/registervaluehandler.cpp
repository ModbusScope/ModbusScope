#include "registervaluehandler.h"
#include "settingsmodel.h"

RegisterValueHandler::RegisterValueHandler(SettingsModel *pSettingsModel) :
    _pSettingsModel(pSettingsModel)
{
}

void RegisterValueHandler::startRead()
{
    _resultList.clear();

    for(quint16 listIdx = 0; listIdx < _registerList.size(); listIdx++)
    {
        _resultList.append(Result<double>(0, false));
    }
}

void RegisterValueHandler::finishRead()
{
    // propagate processed data
    emit registerDataReady(_resultList);
}

void RegisterValueHandler::processPartialResult(QMap<quint32, Result<quint16> > partialResultMap, quint8 connectionId)
{
    for(qint32 listIdx = 0; listIdx < _registerList.size(); listIdx++)
    {
        const ModbusRegister mbReg = _registerList[listIdx];

        if (mbReg.connectionId() == connectionId)
        {
            if (partialResultMap.contains(mbReg.address()))
            {
                Result<quint16> upperRegister;
                Result<quint16> lowerRegister;
                double processedResult = 0;

                lowerRegister = partialResultMap[mbReg.address()];

                if (mbReg.is32Bit())
                {
                    const quint32 addr = mbReg.address() + 1u;
                    if (partialResultMap.contains(addr))
                    {
                        upperRegister = partialResultMap[addr];
                    }
                }
                else
                {
                    /* Dummy valid value */
                    upperRegister = Result<quint16>(0, true);
                }

                bool bSuccess = lowerRegister.isSuccess() && upperRegister.isSuccess();
                if (bSuccess)
                {
                    processedResult = mbReg.processValue(lowerRegister.value(), upperRegister.value(), _pSettingsModel->int32LittleEndian(connectionId));
                }
                else
                {
                    processedResult = 0;
                }

                _resultList[listIdx] = Result<double>(static_cast<qint64>(processedResult), bSuccess);
            }
        }
    }
}

// Get sorted list of active (unique) register addresses for a specific connection id
void RegisterValueHandler::registerAddresList(QList<quint32>& registerList, quint8 connectionId)
{
    QList<quint32> connRegisterList;

    foreach(ModbusRegister mbReg, _registerList)
    {
        if (mbReg.connectionId() == connectionId)
        {
            if (!connRegisterList.contains(mbReg.address()))
            {
                connRegisterList.append(mbReg.address());
            }

            /* When reading 32 bit value, also read next address */
            if (mbReg.is32Bit())
            {
                const quint32 reg = mbReg.address() + 1;
                if (!connRegisterList.contains(reg))
                {
                    connRegisterList.append(reg);
                }
            }
        }
    }

    // sort qList
    std::sort(connRegisterList.begin(), connRegisterList.end(), std::less<int>());

    registerList = connRegisterList;
}

void RegisterValueHandler::setRegisters(QList<ModbusRegister>& registerList)
{
    _registerList = registerList;
}
