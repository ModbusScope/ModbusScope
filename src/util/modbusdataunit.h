#ifndef MODBUSDATAUNIT_H
#define MODBUSDATAUNIT_H

#include "modbusaddress.h"

class ModbusDataUnit : public ModbusAddress
{
public:
    using slaveId_t = quint8;

    ModbusDataUnit(quint16 protocolAddress = 0, ObjectType type = ObjectType::UNKNOWN, slaveId_t slaveId = 1);
    ModbusDataUnit(ModbusAddress addr, slaveId_t slaveId);

    slaveId_t slaveId() const;
    void setSlaveId(slaveId_t id);

    ModbusDataUnit next(int i = 1) const;

    friend bool operator==(const ModbusDataUnit& unit1, const ModbusDataUnit& unit2);
    friend bool operator<(const ModbusDataUnit& unit1, const ModbusDataUnit& unit2);

private:
    slaveId_t _slaveId;
};

#endif // MODBUSDATAUNIT_H
