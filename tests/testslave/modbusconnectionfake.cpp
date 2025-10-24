#include "modbusconnectionfake.h"

#include "util/modbusdataunit.h"
#include <QVariant>

using RegisterType = QModbusDataUnit::RegisterType;
using ObjectType = ModbusAddress::ObjectType;

/*!
 * Constructor for ModbusConnectionFake module
 */
ModbusConnectionFake::ModbusConnectionFake()
{
}

void ModbusConnectionFake::addSlaveDevice(ModbusAddress::slaveId_t slaveId, TestDevice* pDevice)
{
    _deviceMap.insert(slaveId, pDevice);
}

void ModbusConnectionFake::configureTcpConnection(tcpSettings_t const& tcpSettings)
{
    Q_UNUSED(tcpSettings);
}

void ModbusConnectionFake::configureSerialConnection(serialSettings_t const& serialSettings)
{
    Q_UNUSED(serialSettings);
}

/*!
 * Start opening of connection
 * Emits signals (\ref connectionSuccess, \ref errorOccurred) when connection is ready or failed
 *
 * \param[in]   timeout         Timeout of connection (in seconds)
 */
void ModbusConnectionFake::open(quint32 timeout)
{
    Q_UNUSED(timeout);

    emit connectionSuccess();
}

/*!
 *  Close connection
 */
void ModbusConnectionFake::close(void)
{
}

/*!
 * Send read request over connection
 *
 * \param regAddress    register address
 * \param size          number of registers
 * \param serverAddress     slave address
 */
void ModbusConnectionFake::sendReadRequest(ModbusDataUnit regAddress, quint16 size)
{
    using RegisterType = QModbusDataUnit::RegisterType;

    // Map ModbusAddress::ObjectType to QModbusDataUnit::RegisterType
    RegisterType type = QModbusDataUnit::HoldingRegisters;
    switch (regAddress.objectType())
    {
    case ModbusAddress::ObjectType::COIL:
        type = QModbusDataUnit::Coils;
        break;
    case ModbusAddress::ObjectType::DISCRETE_INPUT:
        type = QModbusDataUnit::DiscreteInputs;
        break;
    case ModbusAddress::ObjectType::INPUT_REGISTER:
        type = QModbusDataUnit::InputRegisters;
        break;
    case ModbusAddress::ObjectType::HOLDING_REGISTER:
        type = QModbusDataUnit::HoldingRegisters;
        break;
    default:
        emit readRequestProtocolError(QModbusPdu::IllegalDataAddress);
        return;
    }

    // find device for requested slave id
    const ModbusAddress::slaveId_t sid = regAddress.slaveId();
    TestDevice* pDevice = _deviceMap.value(sid, nullptr);
    if (!pDevice)
    {
        emit readRequestError(QStringLiteral("No test device for slave id %1").arg(static_cast<int>(sid)),
                              QModbusDevice::ConfigurationError);
        return;
    }

    TestSlaveData* pData = pDevice->slaveData(type);
    if (!pData)
    {
        emit readRequestProtocolError(QModbusPdu::IllegalDataAddress);
        return;
    }

    const quint32 startAddress = regAddress.protocolAddress();
    QList<quint16> registerData;
    registerData.reserve(size);
    for (quint16 i = 0; i < size; ++i)
    {
        registerData.append(pData->registerValue(startAddress + i));
    }

    emit readRequestSuccess(regAddress, registerData);
}

/*!
 *  Return whether connection is ok
 *
 * \return Connection state
 */
bool ModbusConnectionFake::isConnected(void)
{
    return true;
}
