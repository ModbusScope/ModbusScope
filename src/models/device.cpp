
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

/*! \brief Compare two devices field by field.
 *
 * The device ID is not part of Device — it is the key in SettingsModel's device map — so
 * only the name and the owning adapter are compared.
 * \param other  The device to compare with.
 * \return True when both devices carry the same name and adapter ID.
 */
bool Device::operator==(const Device& other) const
{
    return (_name == other._name) && (_adapterId == other._adapterId);
}

bool Device::operator!=(const Device& other) const
{
    return !(*this == other);
}
