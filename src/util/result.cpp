#include "result.h"

Result::Result()
    : Result(0, false)
{

}

Result::Result(quint32 value, bool bResult)
    : _value(value), _bResult(bResult)
{

}

quint32 Result::value() const
{
    return _value;
}

void Result::setValue(quint32 value)
{
    _value = value;
}

bool Result::isSuccess() const
{
    return _bResult;
}

void Result::setSuccess(bool bSuccess)
{
    _bResult = bSuccess;
}

Result& Result::operator= (const Result& result)
{
    // self-assignment guard
    if (this == &result)
    {
        return *this;
    }

    _value = result.value();
    _bResult = result.isSuccess();

    // return the existing object so we can chain this operator
    return *this;
}

bool operator== (const Result& res1, const Result& res2)
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

QDebug operator<<(QDebug debug, const Result &result)
{
    QDebugStateSaver saver(debug);
    QString resultString = result.isSuccess() ? "Success" : "Fail" ;
    debug.nospace().noquote() << '(' << resultString << ", " << result.value() << ')';

    return debug;
}


