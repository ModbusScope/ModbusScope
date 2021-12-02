#include "modbusresult.h"

ModbusResult::ModbusResult()
    : ModbusResult(0, false)
{

}

ModbusResult::ModbusResult(quint32 value, bool bResult)
    : _value(value), _bResult(bResult)
{

}

quint32 ModbusResult::value() const
{
    return _value;
}

void ModbusResult::setValue(quint32 value)
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

bool operator== (const ModbusResult& res1, const ModbusResult& res2)
{
    if (
        (res1._value == res2._value)
        && (res1._bResult == res2._bResult)
    )
    {
        return true;
    }
    else
    {
        return false;
    }
}

QDebug operator<<(QDebug debug, const ModbusResult &result)
{
    QDebugStateSaver saver(debug);
    QString resultString = result.isSuccess() ? "Success" : "Fail" ;
    debug.nospace() << '(' << resultString << ", " << result.value() << ')';

    return debug;
}


