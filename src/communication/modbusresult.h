#ifndef MODBUSRESULT_H
#define MODBUSRESULT_H

#include <QObject>
#include <QDebug>

class ModbusResult
{
public:
    ModbusResult();
    ModbusResult(quint32 value, bool bResult);

    quint32 value() const;
    void setValue(quint32 value);

    bool isSuccess() const;
    void setSuccess(bool bSuccess);

    // Copy constructor
    ModbusResult(const ModbusResult& copy)
        : _value { copy.value() }, _bResult { copy._bResult }
    {

    }

    ModbusResult& operator= (const ModbusResult& modbusResult);

private:

    quint32 _value;
    bool _bResult;
};

QDebug operator<<(QDebug debug, const ModbusResult &result);

#endif // MODBUSRESULT_H
