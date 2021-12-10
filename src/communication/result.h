#ifndef RESULT_H
#define RESULT_H

#include <QObject>
#include <QDebug>

class Result
{
public:
    Result();
    Result(quint32 value, bool bResult);

    quint32 value() const;
    void setValue(quint32 value);

    bool isSuccess() const;
    void setSuccess(bool bSuccess);

    // Copy constructor
    Result(const Result& copy)
        : _value { copy.value() }, _bResult { copy._bResult }
    {

    }

    Result& operator= (const Result& modbusResult);

    friend bool operator== (const Result& res1, const Result& res2);

private:

    quint32 _value;
    bool _bResult;
};

QDebug operator<<(QDebug debug, const Result &result);

#endif // RESULT_H
