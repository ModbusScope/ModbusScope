#ifndef DEVICE_H
#define DEVICE_H

#include "models/connectiontypes.h"
#include <QObject>

using deviceId_t = quint32;

class Device
{

public:
    explicit Device(deviceId_t devdId = cFirstDeviceId);
    ~Device() = default;

    void setConnectionId(ConnectionTypes::connectionId_t connectionId);
    void setSlaveId(quint8 id);
    void setConsecutiveMax(quint8 max);
    void setInt32LittleEndian(bool int32LittleEndian);
    void setName(QString const& name);

    ConnectionTypes::connectionId_t connectionId();
    quint8 slaveId();
    quint8 consecutiveMax();
    bool int32LittleEndian();
    QString name();

    static deviceId_t const cFirstDeviceId;

private:
    ConnectionTypes::connectionId_t _connectionId;
    QString _name;
    quint8 _slaveId;
    quint8 _consecutiveMax;
    bool _bInt32LittleEndian;
};

#endif // CONNECTION_H
