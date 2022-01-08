#include "modbusregister.h"

ModbusRegister::ModbusRegister()
    : ModbusRegister(0, 0, false, true)
{

}

ModbusRegister::ModbusRegister(quint16 address, quint8 connectionId, bool is32Bit, bool isUnsigned)
    : _address(address), _connectionId(connectionId), _b32Bit(is32Bit), _bUnsigned(isUnsigned)
{

}

quint16 ModbusRegister::address() const
{
    return _address;
}

void ModbusRegister::setAddress(quint16 address)
{
    _address = address;
}

quint8 ModbusRegister::connectionId() const
{
    return _connectionId;
}

void ModbusRegister::setConnectionId(quint8 connectionId)
{
    _connectionId = connectionId;
}

bool ModbusRegister::is32Bit() const
{
    return _b32Bit;
}

void ModbusRegister::set32Bit(bool b32Bit)
{
    _b32Bit = b32Bit;
}

bool ModbusRegister::isUnsigned() const
{
    return _bUnsigned;
}

void ModbusRegister::setUnsigned(bool bUnsigned)
{
    _bUnsigned = bUnsigned;
}

QString ModbusRegister::description() const
{
    QString unsignedStr = isUnsigned() ? "unsigned" : "signed" ;
    QString typeStr = is32Bit() ? "32 bit" : "16 bit" ;
    QString connStr = QString("conn %1").arg(connectionId() + 1);

    return QString("%1, %2, %3, %4").arg(address())
                                    .arg(unsignedStr, typeStr, connStr);
}

ModbusRegister& ModbusRegister::operator= (const ModbusRegister& modbusRegister)
{
    // self-assignment guard
    if (this == &modbusRegister)
    {
        return *this;
    }

    _address = modbusRegister.address();
    _connectionId = modbusRegister.connectionId();
    _b32Bit = modbusRegister.is32Bit();
    _bUnsigned = modbusRegister.isUnsigned();

    // return the existing object so we can chain this operator
    return *this;
}

bool operator== (const ModbusRegister& reg1, const ModbusRegister& reg2)
{
    if (
        (reg1._address == reg2._address)
        && (reg1._connectionId == reg2._connectionId)
        && (reg1._b32Bit == reg2._b32Bit)
        && (reg1._bUnsigned == reg2._bUnsigned)
    )
    {
        return true;
    }
    else
    {
        return false;
    }
}

QDebug operator<<(QDebug debug, const ModbusRegister &reg)
{
    QDebugStateSaver saver(debug);

    debug.nospace().noquote() << '['
                    << reg.description()
                    << ']';

    return debug;
}
