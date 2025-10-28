#include "testslavemodbus.h"

TestSlaveModbus::TestSlaveModbus(QObject* parent) : QModbusTcpServer(parent)
{
    _exceptionCode = static_cast<QModbusPdu::ExceptionCode>(0);
    _bExceptionPersistent = false;
}

TestSlaveModbus::~TestSlaveModbus()
{

}

bool TestSlaveModbus::connect(QString ip, quint16 port, int slaveId)
{
    setConnectionParameter(QModbusDevice::NetworkPortParameter, port);
    setConnectionParameter(QModbusDevice::NetworkAddressParameter, ip);
    setServerAddress(slaveId);

    return connectDevice();
}

void TestSlaveModbus::disconnect()
{
    disconnectDevice();
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

        TestSlaveData* slaveData = _testDevice.slaveData(newData->registerType());
        if (!slaveData->registerState(regAddress))
        {
            return false;
        }

        newData->setValue(idx, slaveData->registerValue(regAddress));
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
        TestSlaveData* slaveData = _testDevice.slaveData(newData.registerType());
        slaveData->setRegisterState(regAddress, true);
        slaveData->setRegisterValue(regAddress, newData.value(regAddress));
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
    TestSlaveData* slaveData = _testDevice.slaveData(dataUnit->registerType());
    if (dataUnit->isValid() && slaveData->isValidAddress(dataUnit->startAddress(), dataUnit->valueCount()))
    {
        return true;
    }
    else
    {
        return false;
    }
}
