#ifndef MODBUSRESULT_H
#define MODBUSRESULT_H

#include <QObject>
#include <QDebug>

class ModbusResult
{
public:
    ModbusResult();
    ModbusResult(quint16 value, bool bResult);

    quint16 value() const;
    void setValue(quint16 value);

    bool isSuccess() const;
    void setSuccess(bool bSuccess);

private:

    quint16 _value;
    bool _bResult;
};

QDebug operator<<(QDebug debug, const ModbusResult &result);

#endif // MODBUSRESULT_H
