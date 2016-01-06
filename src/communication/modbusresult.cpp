#include "modbusresult.h"

ModbusResult::ModbusResult()
{
    ModbusResult(0, false);
}

ModbusResult::ModbusResult(quint16 value, bool bResult)
{
    _value = value;
    _bResult = bResult;
}

quint16 ModbusResult::value() const
{
    return _value;
}

void ModbusResult::setValue(quint16 value)
{
    _value = value;
}

bool ModbusResult::isSuccess() const
{
    return _bResult;
}

void ModbusResult::setSuccess(bool bSuccess)
{
    _bResult = bSuccess;
}


