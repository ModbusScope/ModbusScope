#ifndef MODBUSDATAUNIT_H
#define MODBUSDATAUNIT_H

#include "util/modbusaddress.h"

class ModbusDataUnit : public ModbusAddress
{
public:
    using slaveId_t = quint8;

    explicit ModbusDataUnit(quint16 protocolAddress = 0, ObjectType type = ObjectType::UNKNOWN, slaveId_t slaveId = 1);
    ModbusDataUnit(ModbusAddress const& addr, slaveId_t slaveId);
    virtual ~ModbusDataUnit() = default;

    slaveId_t slaveId() const;
    void setSlaveId(slaveId_t id);

    ModbusDataUnit next(int i = 1) const;

    QString toString() const override;

    friend bool operator==(const ModbusDataUnit& unit1, const ModbusDataUnit& unit2);
    friend bool operator<(const ModbusDataUnit& unit1, const ModbusDataUnit& unit2);

private:
    slaveId_t _slaveId;
};

#endif // MODBUSDATAUNIT_H
