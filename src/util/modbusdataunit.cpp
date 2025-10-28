#include "modbusdataunit.h"

ModbusDataUnit::ModbusDataUnit(quint16 protocolAddress, ObjectType type, slaveId_t slaveId)
    : ModbusAddress(protocolAddress, type), _slaveId(slaveId)
{
}

ModbusDataUnit::ModbusDataUnit(ModbusAddress const& addr, slaveId_t slaveId) : ModbusAddress(addr), _slaveId(slaveId)
{
}

ModbusDataUnit::slaveId_t ModbusDataUnit::slaveId() const
{
    return _slaveId;
}

void ModbusDataUnit::setSlaveId(slaveId_t id)
{
    _slaveId = id;
}

ModbusDataUnit ModbusDataUnit::next(int i) const
{
    return ModbusDataUnit(_protocolAddress + i, _type, _slaveId);
}

QString ModbusDataUnit::toString() const
{
    return QString("%0, slave id %1").arg(ModbusAddress::toString()).arg(_slaveId);
}

bool operator==(const ModbusDataUnit& unit1, const ModbusDataUnit& unit2)
{
    if ((unit1._protocolAddress == unit2._protocolAddress) && (unit1._type == unit2._type) &&
        (unit1._slaveId == unit2._slaveId))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool operator<(const ModbusDataUnit& unit1, const ModbusDataUnit& unit2)
{
    if (unit1._type < unit2._type)
    {
        return true;
    }
    else if (unit1._type == unit2._type)
    {
        if (unit1._protocolAddress < unit2._protocolAddress)
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
