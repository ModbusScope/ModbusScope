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

void RegisterValueHandler::processPartialResult(ModbusResultMap partialResultMap, connectionId_t connectionId)
{
    auto deviceList = _pSettingsModel->deviceListForConnection(connectionId);

    for(qint32 listIdx = 0; listIdx < _registerList.size(); listIdx++)
    {
        const ModbusRegister mbReg = _registerList[listIdx];

        if ((deviceList.contains(mbReg.deviceId())) && (partialResultMap.contains(mbReg.address())))
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
                                     _pSettingsModel->deviceSettings(mbReg.deviceId())->int32LittleEndian());
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
void RegisterValueHandler::registerAddresListForConnection(QList<ModbusAddress>& registerList,
                                                           connectionId_t connectionId)
{
    QList<ModbusAddress> connRegisterList;

    // Get list of devices for specific connection
    auto deviceList = _pSettingsModel->deviceListForConnection(connectionId);

    for (auto const& mbReg : std::as_const(_registerList))
    {
        if (deviceList.contains(mbReg.deviceId()))
        {
            // TODO dev: Use different class from here on

            auto slaveId = _pSettingsModel->deviceSettings(mbReg.deviceId())->slaveId();
            ModbusAddress address =
              ModbusAddress(slaveId, mbReg.address().protocolAddress(), mbReg.address().objectType());

            if (!connRegisterList.contains(address))
            {
                connRegisterList.append(address);
            }

            /* When reading 32 bit value, also read next address */
            if (ModbusDataType::is32Bit(mbReg.type()))
            {
                const auto reg = address.next();
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
