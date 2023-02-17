#ifndef RESULT_H
#define RESULT_H

#include <QList>
#include <QDebug>


namespace ResultState
{
    enum class State
    {
        ERROR = 0,
        SUCCESS,
        NO_VALUE
    };
}

template <typename T>
class Result
{
public:
    Result();
    Result(T value, ResultState::State bResult);
    Result(const Result<T>& copy);

    T value() const;
    void setValue(T value);

    void setError();

    bool isValid() const;

    ResultState::State state();
    void setState(ResultState::State state);

    Result<T>& operator= (Result<T> const & result);

    friend bool operator== (const Result<T>& res1, const Result<T>& res2)
    {
        if (
            (res1._value == res2._value)
            && (res1._state == res2._state)
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
    ResultState::State _state;
};

/* Implementations need to be in header */

template <class T>
Result<T>::Result()
    : Result(0, ResultState::State::NO_VALUE)
{

}

template <class T>
Result<T>::Result(T value, ResultState::State bResult)
    : _value(value), _state(bResult)
{

}

template<class T>
Result<T>::Result(const Result<T>& copy)
{
    _value = copy._value;
    _state = copy._state;
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
    _state = ResultState::State::SUCCESS;
}

template <class T>
void Result<T>::setError()
{
    _value = 0;
    _state = ResultState::State::ERROR;
}

template <class T>
bool Result<T>::isValid() const
{
    return _state == ResultState::State::SUCCESS;
}

template <class T>
typename ResultState::State Result<T>::state()
{
    return _state;
}

template <class T>
void Result<T>::setState(ResultState::State state)
{
    _state = state;
}

template <class T>
Result<T>& Result<T>::operator= (Result<T> const & result)
{
    // self-assignment guard
    if (this == &result)
    {
        return *this;
    }

    _value = result._value;
    _state = result._state;

    // return the existing object so we can chain this operator
    return *this;
}

template <class T>
QDebug operator<<(QDebug debug, const Result<T> &result)
{
    QDebugStateSaver saver(debug);
    QString resultString = result.isValid() ? "Success" : "Fail" ;
    debug.nospace().noquote() << '(' << resultString << ", " << result.value() << ')';

    return debug;
}

typedef Result<double> ResultDouble;
typedef QList<Result<double>> ResultDoubleList;

#endif // RESULT_H
