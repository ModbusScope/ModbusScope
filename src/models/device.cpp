
#include "device.h"

Device::Device() : _connectionId(0), _slaveId(1), _consecutiveMax(125), _bInt32LittleEndian(true)
{
}

Device::~Device()
{
}

void Device::setId(quint8 id)
{
    _id = id;
}

quint8 Device::id()
{
    return _id;
}

void Device::setConnectionId(connectionId_t connectionId)
{
    _connectionId = connectionId;
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
