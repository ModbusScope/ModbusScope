#include "modbusresult.h"

ModbusResult::ModbusResult()
{
    ModbusResult(0, false);
}

ModbusResult::ModbusResult(quint16 value, bool bResult)
    : _value(value), _bResult(bResult)
{

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

ModbusResult& ModbusResult::operator= (const ModbusResult& modbusResult)
{
    // self-assignment guard
    if (this == &modbusResult)
    {
        return *this;
    }

    _value = modbusResult.value();
    _bResult = modbusResult.isSuccess();

    // return the existing object so we can chain this operator
    return *this;
}

QDebug operator<<(QDebug debug, const ModbusResult &result)
{
    QDebugStateSaver saver(debug);
    QString resultString = result.isSuccess() ? "Success" : "Fail" ;
    debug.nospace() << '(' << resultString << ", " << result.value() << ')';

    return debug;
}


