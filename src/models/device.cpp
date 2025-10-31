
#include "device.h"

using connectionId_t = ConnectionTypes::connectionId_t;

deviceId_t const Device::cFirstDeviceId = 1;

Device::Device(deviceId_t devdId)
    : _connectionId(0),
      _name(QString("Device %0").arg(devdId)),
      _slaveId(1),
      _consecutiveMax(125),
      _bInt32LittleEndian(true)
{
}

void Device::setName(QString const& name)
{
    _name = name;
}

void Device::setConnectionId(connectionId_t connectionId)
{
    _connectionId = connectionId;
}

QString Device::name()
{
    return _name;
}

connectionId_t Device::connectionId()
{
    return _connectionId;
}

void Device::setConsecutiveMax(quint8 max)
{
    _consecutiveMax = max;
}

quint8 Device::consecutiveMax()
{
    return _consecutiveMax;
}

void Device::setInt32LittleEndian(bool int32LittleEndian)
{
    _bInt32LittleEndian = int32LittleEndian;
}

bool Device::int32LittleEndian()
{
    return _bInt32LittleEndian;
}

quint8 Device::slaveId()
{
    return _slaveId;
}

void Device::setSlaveId(quint8 id)
{
    _slaveId = id;
}
