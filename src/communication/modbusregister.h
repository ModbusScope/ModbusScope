#ifndef MODBUSREGISTER_H
#define MODBUSREGISTER_H

#include <QObject>
#include <QDebug>
#include "modbusaddress.h"
#include "modbusdatatype.h"

class ModbusRegister
{
public:
    ModbusRegister();
    ModbusRegister(ModbusAddress address, quint8 connectionId, ModbusDataType::Type type);

    ModbusAddress address() const;
    void setAddress(ModbusAddress address);
    
    quint8 connectionId() const;
    void setConnectionId(quint8 connectionId);

    void setType(ModbusDataType::Type type);
    ModbusDataType::Type type() const;

    QString description() const;

    double processValue(uint16_t lowerRegister, uint16_t upperRegister, bool int32LittleEndian) const;

    ModbusRegister(const ModbusRegister& copy)
        : _address { copy.address() }, _connectionId { copy.connectionId() }, _type {copy.type()}
    {

    }

    ModbusRegister& operator= (const ModbusRegister& modbusRegister);

    friend bool operator== (const ModbusRegister& reg1, const ModbusRegister& reg2);

    static QString dumpListToString(QList<ModbusRegister> list);

private:
    uint32_t convertEndianness(bool bLittleEndian, quint16 value, quint16 nextValue) const;
    double convertUint32ToFloat(quint32 value) const;

    ModbusAddress _address;
    quint8 _connectionId;
    ModbusDataType::Type _type;
};

QDebug operator<<(QDebug debug, const ModbusRegister &reg);

#endif // MODBUSREGISTER_H
