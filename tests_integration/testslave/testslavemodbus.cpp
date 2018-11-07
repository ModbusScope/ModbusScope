#include "testslavemodbus.h"
#include <QDebug>

TestSlaveModbus::TestSlaveModbus(TestSlaveData *pTestSlaveData, QObject *parent) : QModbusTcpServer(parent)
{
    _pTestSlaveData = pTestSlaveData;
    _exceptionCode = static_cast<QModbusPdu::ExceptionCode>(0);
}

TestSlaveModbus::~TestSlaveModbus()
{

}

bool TestSlaveModbus::connect(QUrl host, int slaveId)
{
    setConnectionParameter(QModbusDevice::NetworkPortParameter, host.port());
    setConnectionParameter(QModbusDevice::NetworkAddressParameter, host.host());
    setServerAddress(slaveId);

    return connectDevice();
}

void TestSlaveModbus::setException(QModbusPdu::ExceptionCode exception)
{
    _exceptionCode = exception;
}

bool TestSlaveModbus::readData(QModbusDataUnit *newData) const
{
    if (!verifyValidHoldingRegister(newData))
    {
        return false;
    }

    for(uint idx = 0; idx < static_cast<uint>(newData->valueCount()); idx++)
    {
        const uint regAddress = static_cast<uint>(newData->startAddress()) + idx;

        if (!_pTestSlaveData->registerState(regAddress))
        {
            return false;
        }

        newData->setValue(regAddress, _pTestSlaveData->registerValue(regAddress));
    }

    return true;
}

bool TestSlaveModbus::setMap(const QModbusDataUnitMap &map)
{
    if (!map.keys().contains(QModbusDataUnit::HoldingRegisters))
    {
        return false;
    }

    QModbusDataUnit holdingReg = map.value(QModbusDataUnit::HoldingRegisters);

    if (!verifyValidHoldingRegister(&holdingReg))
    {
        return false;
    }

    for(uint idx = 0; idx < static_cast<uint>(holdingReg.valueCount()); idx++)
    {
        const uint regAddress = static_cast<uint>(holdingReg.startAddress()) + idx;
        _pTestSlaveData->setRegisterState(regAddress, true);
        _pTestSlaveData->setRegisterValue(regAddress, holdingReg.value(regAddress));
    }

    emit dataWritten(holdingReg.registerType(), holdingReg.startAddress(), holdingReg.valueCount());

    return true;
}

bool TestSlaveModbus::writeData(const QModbusDataUnit &newData)
{
    if (!verifyValidHoldingRegister(&newData))
    {
        return false;
    }

    for(uint idx = 0; idx < static_cast<uint>(newData.valueCount()); idx++)
    {
        const uint regAddress = static_cast<uint>(newData.startAddress()) + idx;
        _pTestSlaveData->setRegisterState(regAddress, true);
        _pTestSlaveData->setRegisterValue(regAddress, newData.value(regAddress));
    }

    emit dataWritten(newData.registerType(), newData.startAddress(), newData.valueCount());

    return true;
}

QModbusResponse TestSlaveModbus::processRequest(const QModbusPdu &request)
{
    QModbusResponse response;
    if (_exceptionCode == 0)
    {
        response = QModbusTcpServer::processRequest(request);
    }
    else
    {
        response = QModbusExceptionResponse(request.functionCode(), _exceptionCode);
    }

    emit requestProcessed();

    return response;
}

bool TestSlaveModbus::verifyValidHoldingRegister(QModbusDataUnit const * dataUnit) const
{
    if (
        dataUnit->isValid()
        && dataUnit->registerType() == QModbusDataUnit::HoldingRegisters
        && ((dataUnit->valueCount() + static_cast<uint>(dataUnit->startAddress())) <= _pTestSlaveData->size())
        )
    {
        return true;
    }
    else
    {
        return false;
    }
}
