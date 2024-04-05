
#include "modbusaddress.h"

/* Also add value for UNKOWN */
const QList<quint16> ModbusAddress::cObjectTypeOffsets = QList<quint16>() << 0 << 10001 << 30001 << 40001 << 0;
const QList<QChar> ModbusAddress::cObjectTypePrefix = QList<QChar>() << 'c' << 'd' << 'i' << 'h' << 'h';

using ObjectType = ModbusAddress::ObjectType;

ModbusAddress::ModbusAddress()
    : _type(ModbusAddress::ObjectType::HOLDING_REGISTER)
{

}

ModbusAddress::ModbusAddress(quint32 address, ObjectType type)
{
    if (type == ObjectType::UNKNOWN)
    {
        constructAddressFromNumber(address);
    }
    else
    {
        _protocolAddress = static_cast<quint16>(address);
        _type = type;
    }
}

ModbusAddress::ModbusAddress(quint32 address)
    : ModbusAddress(address, ObjectType::UNKNOWN)
{

}

ModbusAddress::ModbusAddress(QString address)
{
    bool bOk = false;
    quint32 addressNumber = address.toUInt(&bOk);

    if (bOk)
    {
        constructAddressFromNumber(addressNumber);
    }
    else
    {
        constructAddressFromStringWithType(address);
    }
}

ModbusAddress::ObjectType ModbusAddress::objectType() const
{
    return _type;
}

QString ModbusAddress::fullAddress() const
{
    return QString("%1").arg(_protocolAddress + cObjectTypeOffsets[static_cast<int>(_type)]);
}

quint16 ModbusAddress::protocolAddress() const
{
    return _protocolAddress;
}

QString ModbusAddress::toString() const
{
    QString typeStr;

    switch (_type)
    {
    case ObjectType::COIL: typeStr = QString("coil"); break;
    case ObjectType::DISCRETE_INPUT: typeStr = QString("discrete input"); break;
    case ObjectType::INPUT_REGISTER: typeStr = QString("input register"); break;
    case ObjectType::HOLDING_REGISTER: typeStr = QString("holding register"); break;
    case ObjectType::UNKNOWN: typeStr = QString("unknown"); break;
    }

    return QString("%1, %2").arg(typeStr).arg(protocolAddress());
}

ModbusAddress ModbusAddress::next() const
{
    return next(1);
}

ModbusAddress ModbusAddress::next(int i) const
{
    ModbusAddress nextAddres(*this);

    nextAddres._protocolAddress += i;

    return nextAddres;
}

void ModbusAddress::constructAddressFromNumber(quint32 address)
{
    _type = convertFromOffset(address);

    quint16 offset = cObjectTypeOffsets[static_cast<int>(_type)];
    if (address >= offset)
    {
        _protocolAddress = static_cast<quint16>(address - offset);
    }
    else
    {
        _protocolAddress = 0;
        _type = ObjectType::UNKNOWN;
    }
}

void ModbusAddress::constructAddressFromStringWithType(QString addressWithtype)
{
    QChar prefix = addressWithtype.at(0);
    QString address = addressWithtype.mid(1);

    bool bOk = false;
    quint32 addressNumber = address.toUInt(&bOk);

    if (cObjectTypePrefix.contains(prefix) && bOk)
    {
        _protocolAddress = static_cast<quint16>(addressNumber);
        _type = static_cast<ObjectType>( cObjectTypePrefix.indexOf(prefix));
    }
    else
    {
        _protocolAddress = 0;
        _type = ObjectType::UNKNOWN;
    }
}

ModbusAddress::ObjectType ModbusAddress::convertFromOffset(quint32 address)
{
    ModbusAddress::ObjectType type = ObjectType::UNKNOWN;

    if (address >= cObjectTypeOffsets[static_cast<int>(ObjectType::HOLDING_REGISTER)])
    {
        type = ObjectType::HOLDING_REGISTER;
    }
    else if (address >= cObjectTypeOffsets[static_cast<int>(ObjectType::INPUT_REGISTER)])
    {
        type = ObjectType::INPUT_REGISTER;
    }
    else if (address >= cObjectTypeOffsets[static_cast<int>(ObjectType::DISCRETE_INPUT)])
    {
        type = ObjectType::DISCRETE_INPUT;
    }
    else if (address >= cObjectTypeOffsets[static_cast<int>(ObjectType::COIL)])
    {
        type = ObjectType::COIL;
    }
    else
    {
        type = ObjectType::UNKNOWN;
    }

    return type;
}

bool operator== (const ModbusAddress& addr1, const ModbusAddress& addr2)
{
    if (
        (addr1._protocolAddress == addr2._protocolAddress)
        && (addr1._type == addr2._type)
        )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool operator< (const ModbusAddress& reg1, const ModbusAddress& reg2)
{
    if (reg1._type < reg2._type)
    {
        return true;
    }
    else if (reg1._type == reg2._type)
    {
        if (reg1._protocolAddress < reg2._protocolAddress)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool operator> (const ModbusAddress& reg1, const ModbusAddress& reg2)
{
    if (reg1._type > reg2._type)
    {
        return true;
    }
    else if (reg1._type == reg2._type)
    {
        if (reg1._protocolAddress > reg2._protocolAddress)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

