#ifndef DEVICE_H
#define DEVICE_H

#include "models/connectiontypes.h"
#include <QObject>

using deviceId_t = quint32;

class Device
{

public:
    explicit Device();
    ~Device();

    void setConnectionId(connectionId_t connectionId);
    void setSlaveId(quint8 id);
    void setConsecutiveMax(quint8 max);
    void setInt32LittleEndian(bool int32LittleEndian);

    connectionId_t connectionId();
    quint8 slaveId();
    quint8 consecutiveMax();
    bool int32LittleEndian();

    static deviceId_t const cFirstDeviceId;

private:
    connectionId_t _connectionId;
    quint8 _slaveId;
    quint8 _consecutiveMax;
    bool _bInt32LittleEndian;
};

#endif // CONNECTION_H
