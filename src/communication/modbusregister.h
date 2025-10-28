#ifndef MODBUSREGISTER_H
#define MODBUSREGISTER_H

#include "models/device.h"
#include "util/modbusaddress.h"
#include "util/modbusdatatype.h"
#include <QDebug>
#include <QObject>

class ModbusRegister
{
public:
    ModbusRegister();
    ModbusRegister(ModbusAddress const& address, deviceId_t deviceId, ModbusDataType::Type type);

    ModbusAddress address() const;
    void setAddress(ModbusAddress const& address);

    deviceId_t deviceId() const;
    void setDeviceId(deviceId_t deviceId);

    void setType(ModbusDataType::Type type);
    ModbusDataType::Type type() const;

    QString description() const;

    double processValue(uint16_t lowerRegister, uint16_t upperRegister, bool int32LittleEndian) const;

    ModbusRegister(const ModbusRegister& copy)
        : _address{ copy.address() }, _deviceId{ copy.deviceId() }, _type{ copy.type() }
    {

    }

    ModbusRegister& operator= (const ModbusRegister& modbusRegister);

    friend bool operator== (const ModbusRegister& reg1, const ModbusRegister& reg2);

    static QString dumpListToString(QList<ModbusRegister> list);

private:
    uint32_t convertEndianness(bool bLittleEndian, quint16 value, quint16 nextValue) const;
    double convertUint32ToFloat(quint32 value) const;

    ModbusAddress _address;
    deviceId_t _deviceId;
    ModbusDataType::Type _type;
};

QDebug operator<<(QDebug debug, const ModbusRegister &reg);

#endif // MODBUSREGISTER_H
