#ifndef MODBUSREGISTER_H
#define MODBUSREGISTER_H

#include <QObject>
#include <QDebug>

class ModbusRegister
{
public:
    ModbusRegister();
    ModbusRegister(quint32 address, quint8 connectionId, QString type);

    quint32 address() const;
    void setAddress(quint32 address);
    
    quint8 connectionId() const;
    void setConnectionId(quint8 connectionId);

    bool setType(QString type);

    bool is32Bit() const;
    bool isUnsigned() const;
    bool isFloat() const;

    QString description() const;

    ModbusRegister(const ModbusRegister& copy)
        : _address { copy.address() }, _connectionId { copy.connectionId() },
          _b32Bit { copy.is32Bit() }, _bUnsigned { copy.isUnsigned()}, _bFloat { copy.isFloat() }
    {

    }

    ModbusRegister& operator= (const ModbusRegister& modbusRegister);

    friend bool operator== (const ModbusRegister& reg1, const ModbusRegister& reg2);

    static QString dumpListToString(QList<ModbusRegister> list);

private:

    quint32 _address;
    quint8 _connectionId;
    bool _b32Bit;
    bool _bUnsigned;
    bool _bFloat;
};

QDebug operator<<(QDebug debug, const ModbusRegister &reg);

#endif // MODBUSREGISTER_H
