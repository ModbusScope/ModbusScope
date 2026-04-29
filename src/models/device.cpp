
#include "device.h"

deviceId_t const Device::cFirstDeviceId = 1;

Device::Device(deviceId_t devdId) : _name(QString("Device %1").arg(devdId)), _adapterId("modbus")
{
}

void Device::setName(QString const& name)
{
    _name = name;
}

QString Device::name()
{
    return _name;
}

void Device::setAdapterId(const QString& adapterId)
{
    _adapterId = adapterId;
}

QString Device::adapterId() const
{
    return _adapterId;
}
