#include "testslavemodbus.h"

TestSlaveModbus::TestSlaveModbus(ModbusDataMap &testSlaveData, QObject *parent)
    : QModbusTcpServer(parent), _testSlaveData(testSlaveData)
{
    _exceptionCode = static_cast<QModbusPdu::ExceptionCode>(0);
    _bExceptionPersistent = false;
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

void TestSlaveModbus::disconnect()
{
    return disconnectDevice();
}

void TestSlaveModbus::setException(QModbusPdu::ExceptionCode exception, bool bPersistent)
{
    _exceptionCode = exception;
    _bExceptionPersistent = bPersistent;
}

bool TestSlaveModbus::readData(QModbusDataUnit *newData) const
{
    if (!verifyValidObject(newData))
    {
        return false;
    }

    for(uint idx = 0; idx < static_cast<uint>(newData->valueCount()); idx++)
    {
        const uint regAddress = static_cast<uint>(newData->startAddress()) + idx;

        if (!_testSlaveData[newData->registerType()]->registerState(regAddress))
        {
            return false;
        }

        newData->setValue(idx, _testSlaveData[newData->registerType()]->registerValue(regAddress));
    }

    return true;
}

bool TestSlaveModbus::setMap(const QModbusDataUnitMap &map)
{
    Q_UNUSED(map);
    return false; /* Not implemented */
}

bool TestSlaveModbus::writeData(const QModbusDataUnit &newData)
{
    if (!verifyValidObject(&newData))
    {
        return false;
    }

    for(uint idx = 0; idx < static_cast<uint>(newData.valueCount()); idx++)
    {
        const uint regAddress = static_cast<uint>(newData.startAddress()) + idx;
        _testSlaveData[newData.registerType()]->setRegisterState(regAddress, true);
        _testSlaveData[newData.registerType()]->setRegisterValue(regAddress, newData.value(regAddress));
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

    /* Reset exception when not persistent */
    if (!_bExceptionPersistent)
    {
        _exceptionCode = static_cast<QModbusPdu::ExceptionCode>(0);
    }

    return response;
}

bool TestSlaveModbus::verifyValidObject(QModbusDataUnit const * dataUnit) const
{
    if (
        dataUnit->isValid()
        && _testSlaveData.contains(dataUnit->registerType())
        && _testSlaveData[dataUnit->registerType()]->IsValidAddress(static_cast<quint32>(dataUnit->startAddress()), static_cast<quint32>(dataUnit->valueCount()))
        )
    {
        return true;
    }
    else
    {
        return false;
    }
}
