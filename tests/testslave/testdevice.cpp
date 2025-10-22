#include "testdevice.h"

TestDevice::TestDevice(QObject* parent) : QObject(parent)
{
    _slaveDataMap[QModbusDataUnit::Coils] = new TestSlaveData();
    _slaveDataMap[QModbusDataUnit::DiscreteInputs] = new TestSlaveData();
    _slaveDataMap[QModbusDataUnit::InputRegisters] = new TestSlaveData();
    _slaveDataMap[QModbusDataUnit::HoldingRegisters] = new TestSlaveData();

    _pSlaveModbus = new TestSlaveModbus(_slaveDataMap);
}

TestDevice::~TestDevice()
{
    _pSlaveModbus->disconnect();

    delete _pSlaveModbus;

    if (!_slaveDataMap.isEmpty())
    {
        qDeleteAll(_slaveDataMap);
        _slaveDataMap.clear();
    }
}

bool TestDevice::connect(QString ip, quint16 port, int slaveId)
{
    QUrl url;
    url.setHost(ip);
    url.setPort(port);
    return _pSlaveModbus->connect(url, slaveId);
}

void TestDevice::disconnect()
{
    _pSlaveModbus->disconnect();
}

void TestDevice::setException(QModbusPdu::ExceptionCode exception, bool bPersistent)
{
    _pSlaveModbus->setException(exception, bPersistent);
}

TestSlaveData* TestDevice::slaveData(QModbusDataUnit::RegisterType type) const
{
    return _slaveDataMap.value(type, nullptr);
}

void TestDevice::configureHoldingRegister(uint address, bool state, quint16 value)
{
    configureRegister(QModbusDataUnit::HoldingRegisters, address, state, value);
}

void TestDevice::configureInputRegister(uint address, bool state, quint16 value)
{
    configureRegister(QModbusDataUnit::InputRegisters, address, state, value);
}

void TestDevice::configureCoil(uint address, bool state, bool value)
{
    configureRegister(QModbusDataUnit::Coils, address, state, value ? 1 : 0);
}

void TestDevice::configureDiscreteInput(uint address, bool state, bool value)
{
    configureRegister(QModbusDataUnit::DiscreteInputs, address, state, value ? 1 : 0);
}

void TestDevice::configureRegister(QModbusDataUnit::RegisterType type, uint address, bool state, quint16 value)
{
    if (_slaveDataMap.contains(type))
    {
        _slaveDataMap[type]->setRegisterState(address, state);
        _slaveDataMap[type]->setRegisterValue(address, value);
    }
}
