#include "testdevice.h"

TestDevice::TestDevice(QObject* parent) : QObject(parent)
{
    _slaveDataMap[QModbusDataUnit::Coils] = new TestSlaveData();
    _slaveDataMap[QModbusDataUnit::DiscreteInputs] = new TestSlaveData();
    _slaveDataMap[QModbusDataUnit::InputRegisters] = new TestSlaveData();
    _slaveDataMap[QModbusDataUnit::HoldingRegisters] = new TestSlaveData();
}

TestDevice::~TestDevice()
{
    if (!_slaveDataMap.isEmpty())
    {
        qDeleteAll(_slaveDataMap);
        _slaveDataMap.clear();
    }
}

TestSlaveData* TestDevice::slaveData(QModbusDataUnit::RegisterType type) const
{
    return _slaveDataMap.value(type, nullptr);
}

void TestDevice::setSlaveData(QModbusDataUnit::RegisterType type, TestSlaveData* slaveData)
{
    if (_slaveDataMap.contains(type))
    {
        delete _slaveDataMap[type];
    }
    _slaveDataMap[type] = slaveData;
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
