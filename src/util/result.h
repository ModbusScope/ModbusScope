#ifndef RESULT_H
#define RESULT_H

#include <QObject>
#include <QDebug>

template <typename T>
class Result
{
public:
    Result();
    Result(T value, bool bResult);
    Result(const Result<T>& copy);

    T value() const;
    void setValue(T value);

    bool isSuccess() const;
    void setSuccess(bool bSuccess);

    Result<T>& operator= (Result<T> const & result);

    friend bool operator== (const Result<T>& res1, const Result<T>& res2)
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

private:

    T _value;
    bool _bResult;
};


/* Implementations need to be in header */

template <class T>
Result<T>::Result()
    : Result(0, false)
{

}

template <class T>
Result<T>::Result(T value, bool bResult)
    : _value(value), _bResult(bResult)
{

}

template<class T>
Result<T>::Result(const Result<T>& copy)
{
    _value = copy.value();
    _bResult = copy.isSuccess();
}

template <class T>
T Result<T>::value() const
{
    return _value;
}

template <class T>
void Result<T>::setValue(T value)
{
    _value = value;
}

template <class T>
bool Result<T>::isSuccess() const
{
    return _bResult;
}

template <class T>
void Result<T>::setSuccess(bool bSuccess)
{
    _bResult = bSuccess;
}

template <class T>
Result<T>& Result<T>::operator= (Result<T> const & result)
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

template <class T>
QDebug operator<<(QDebug debug, const Result<T> &result)
{
    QDebugStateSaver saver(debug);
    QString resultString = result.isSuccess() ? "Success" : "Fail" ;
    debug.nospace().noquote() << '(' << resultString << ", " << result.value() << ')';

    return debug;
}

#endif // RESULT_H
