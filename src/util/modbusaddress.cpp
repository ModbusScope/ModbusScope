
#include "modbusaddress.h"

const quint16 ModbusAddress::cObjectTypeOffsets[] = { 0, 10001, 30001, 40001, 0 };

using ObjectType = ModbusAddress::ObjectType;

ModbusAddress::ModbusAddress()
    : _address(0), _type(ModbusAddress::ObjectType::HOLDING_REGISTER)
{

}

ModbusAddress::ModbusAddress(quint16 address, ObjectType type)
{
    if (type == ObjectType::UNKNOWN)
    {
        _type = convertFromOffset(address);

        quint16 offset = cObjectTypeOffsets[static_cast<int>(_type)];
        if (address >= offset)
        {
            _address = address - offset;
        }
        else
        {
            _address = 0;
            _type = ObjectType::UNKNOWN;
        }
    }
    else
    {
        _address = address;
        _type = type;
    }
}

ModbusAddress::ModbusAddress(quint16 address)
    : ModbusAddress(address, ObjectType::UNKNOWN)
{

}

ModbusAddress::ObjectType ModbusAddress::objectType() const
{
    return _type;
}

quint16 ModbusAddress::address(Offset offset) const
{
    if (offset == Offset::WITHOUT_OFFSET)
    {
        return _address;
    }
    else
    {
        return _address + cObjectTypeOffsets[static_cast<int>(_type)];
    }
}

QString ModbusAddress::toString() const
{
    /* TODO: show type and address */
    return QString("%1").arg(address(Offset::WITH_OFFSET));
}

ModbusAddress ModbusAddress::next() const
{
    return next(1);
}

ModbusAddress ModbusAddress::next(int i) const
{
    ModbusAddress nextAddres(*this);

    nextAddres._address += i;

    return nextAddres;
}


ModbusAddress::ObjectType ModbusAddress::convertFromOffset(quint16 address)
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
        (addr1._address == addr2._address)
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
        if (reg1._address < reg2._address)
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
        if (reg1._address > reg2._address)
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

