#ifndef MODBUSADDRESS_H
#define MODBUSADDRESS_H

#include "qdebug.h"
#include <QObject>

class ModbusAddress
{

public:
    using slaveId_t = int;

    enum class ObjectType
    {
        COIL = 0,
        DISCRETE_INPUT = 1,
        INPUT_REGISTER = 2,
        HOLDING_REGISTER = 3,
        UNKNOWN = 4
    };

    ModbusAddress();
    ModbusAddress(quint32 address, ModbusAddress::ObjectType type);
    ModbusAddress(slaveId_t slaveId, quint32 address, ModbusAddress::ObjectType type);
    explicit ModbusAddress(quint32 address);
    explicit ModbusAddress(QString address);
    ModbusAddress(const ModbusAddress& t) = default;

    ~ModbusAddress() = default;

    ModbusAddress::ObjectType objectType() const;
    QString fullAddress() const;
    quint16 protocolAddress() const;
    slaveId_t slaveId() const;

    QString toString() const;

    ModbusAddress next() const;
    ModbusAddress next(int i) const;

    friend bool operator==(const ModbusAddress& reg1, const ModbusAddress& reg2);
    friend bool operator<(const ModbusAddress& reg1, const ModbusAddress& reg2);
    friend bool operator>(const ModbusAddress& reg1, const ModbusAddress& reg2);

private:
    void constructAddressFromNumber(quint32 address);
    void constructAddressFromStringWithType(QString addressWithtype);
    ModbusAddress::ObjectType convertFromOffset(quint32 address);

    quint16 _protocolAddress{ 0 };
    ObjectType _type{ ObjectType::UNKNOWN };
    slaveId_t _slaveId{ 1 }; // TODO: Is this the correct class, split this?

    static const QList<quint16> cObjectTypeOffsets;
    static const QList<QChar> cObjectTypePrefix;

};

inline QDebug operator<<(QDebug debug, const ModbusAddress &addr)
{
    QDebugStateSaver saver(debug);

    debug.nospace().noquote() << '(' << addr.toString() << ')';

    return debug;
}

#endif // MODBUSADDRESS_H
