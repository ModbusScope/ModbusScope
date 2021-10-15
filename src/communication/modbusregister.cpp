#include "modbusregister.h"

ModbusRegister::ModbusRegister()
{
    _address = 0;
    _connectionId = 0;
    _b32Bit = false;
    _bUnsigned = false;
}

quint16 ModbusRegister::address() const
{
    return _address;
}

void ModbusRegister::setAddress(quint16 address)
{
    _address = address;
}

quint8 ModbusRegister::connectionId() const
{
    return _connectionId;
}

void ModbusRegister::setConnectionId(quint8 connectionId)
{
    _connectionId = connectionId;
}

bool ModbusRegister::is32Bit() const
{
    return _b32Bit;
}

void ModbusRegister::set32Bit(bool b32Bit)
{
    _b32Bit = b32Bit;
}

bool ModbusRegister::isUnsigned() const
{
    return _bUnsigned;
}

void ModbusRegister::setUnsigned(bool bUnsigned)
{
    _bUnsigned = bUnsigned;
}
