#include "registervaluehandler.h"

#include "models/settingsmodel.h"
#include "util/modbusaddress.h"
#include "util/modbusdatatype.h"

using State = ResultState::State;

RegisterValueHandler::RegisterValueHandler(SettingsModel *pSettingsModel) :
    _pSettingsModel(pSettingsModel)
{
}

void RegisterValueHandler::startRead()
{
    _resultList.clear();

    for(quint16 listIdx = 0; listIdx < _registerList.size(); listIdx++)
    {
        _resultList.append(ResultDouble(0, State::INVALID));
    }
}

void RegisterValueHandler::finishRead()
{
    emit registerDataReady(_resultList);
}

void RegisterValueHandler::processPartialResult(ModbusResultMap partialResultMap, quint8 connectionId)
{
    for(qint32 listIdx = 0; listIdx < _registerList.size(); listIdx++)
    {
        const ModbusRegister mbReg = _registerList[listIdx];

        if (
            (mbReg.connectionId() == connectionId)
            && (partialResultMap.contains(mbReg.address()))
            )
        {
            Result<quint16> upperRegister;
            Result<quint16> lowerRegister;

            lowerRegister = partialResultMap.value(mbReg.address());

            if (ModbusDataType::is32Bit(mbReg.type()))
            {
                const auto addr = mbReg.address().next();
                if (partialResultMap.contains(addr))
                {
                    upperRegister = partialResultMap.value(addr);
                }
            }
            else
            {
                /* Dummy valid value */
                upperRegister = Result<quint16>(0, State::SUCCESS);
            }

            bool bSuccess = lowerRegister.isValid() && upperRegister.isValid();
            ResultDouble result;
            if (bSuccess)
            {
                double processedResult =
                  mbReg.processValue(lowerRegister.value(), upperRegister.value(),
                                     _pSettingsModel->connectionSettings(connectionId)->int32LittleEndian());
                result.setValue(processedResult);
            }
            else
            {
                result.setError();
            }

            _resultList[listIdx] = result;
        }
    }
}

// Get sorted list of active (unique) register addresses for a specific connection id
void RegisterValueHandler::registerAddresList(QList<ModbusAddress>& registerList, quint8 connectionId)
{
    QList<ModbusAddress> connRegisterList;

    foreach(ModbusRegister mbReg, _registerList)
    {
        if (mbReg.connectionId() == connectionId)
        {
            if (!connRegisterList.contains(mbReg.address()))
            {
                connRegisterList.append(mbReg.address());
            }

            /* When reading 32 bit value, also read next address */
            if (ModbusDataType::is32Bit(mbReg.type()))
            {
                const auto reg = mbReg.address().next();
                if (!connRegisterList.contains(reg))
                {
                    connRegisterList.append(reg);
                }
            }
        }
    }

    // sort qList
    std::sort(connRegisterList.begin(), connRegisterList.end(), std::less<ModbusAddress>());

    registerList = connRegisterList;
}

void RegisterValueHandler::setRegisters(QList<ModbusRegister>& registerList)
{
    _registerList = registerList;
}
