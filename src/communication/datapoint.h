#ifndef DATAPOINT_H
#define DATAPOINT_H

#include "models/device.h"
#include "util/modbusdatatype.h"
#include <QDebug>
#include <QObject>
#include <QString>

class DataPoint
{
public:
    DataPoint();
    DataPoint(QString const& address, deviceId_t deviceId, ModbusDataType::Type type);

    QString address() const;
    void setAddress(const QString& address);

    deviceId_t deviceId() const;
    void setDeviceId(deviceId_t deviceId);

    void setType(ModbusDataType::Type type);
    ModbusDataType::Type type() const;

    QString description() const;

    double processValue(uint16_t lowerRegister, uint16_t upperRegister, bool int32LittleEndian) const;

    DataPoint(const DataPoint& copy) : _address{ copy.address() }, _deviceId{ copy.deviceId() }, _type{ copy.type() }
    {
    }

    DataPoint& operator=(const DataPoint& dataPoint);

    friend bool operator==(const DataPoint& dp1, const DataPoint& dp2);

    static QString dumpListToString(QList<DataPoint> list);

private:
    uint32_t convertEndianness(bool bLittleEndian, quint16 value, quint16 nextValue) const;
    double convertUint32ToFloat(quint32 value) const;

    QString _address;
    deviceId_t _deviceId;
    ModbusDataType::Type _type;
};

QDebug operator<<(QDebug debug, const DataPoint& dp);

#endif // DATAPOINT_H
