#ifndef DEVICELISTHELPERS_H
#define DEVICELISTHELPERS_H

#include "models/device.h"
#include "models/settingsmodel.h"

#include <QMap>
#include <QString>

/*! \brief Device-list edits for tests, built on SettingsModel::applyDeviceList().
 *
 * applyDeviceList() replaces the whole list, so a test that only wants to add or drop one
 * device has to read the current list back first. These helpers do that, keeping the
 * convenience the model no longer carries out of SettingsModel itself.
 */
namespace DeviceListHelpers {

//! Read the model's device list back in the form applyDeviceList() takes.
inline QMap<deviceId_t, Device> currentDevices(SettingsModel* pModel)
{
    QMap<deviceId_t, Device> devices;
    const QList<deviceId_t> deviceIds = pModel->deviceList();
    for (const deviceId_t devId : deviceIds)
    {
        devices.insert(devId, *pModel->deviceSettings(devId));
    }
    return devices;
}

/*! \brief Add a device, or replace it when the ID is already in use.
 * \param pModel     Model to edit.
 * \param devId      Identifier of the device.
 * \param adapterId  Adapter owning the device; empty keeps Device's default adapter.
 * \param name       Display name of the device; empty keeps Device's default name.
 */
inline void seedDevice(SettingsModel* pModel,
                       deviceId_t devId,
                       const QString& adapterId = QString(),
                       const QString& name = QString())
{
    Device device(devId);
    if (!adapterId.isEmpty())
    {
        device.setAdapterId(adapterId);
    }
    if (!name.isEmpty())
    {
        device.setName(name);
    }

    QMap<deviceId_t, Device> devices = currentDevices(pModel);
    devices.insert(devId, device);
    pModel->applyDeviceList(devices);
}

/*! \brief Add a device one past the highest ID currently in the model.
 * \return The identifier of the new device.
 */
inline deviceId_t seedNextDevice(SettingsModel* pModel, const QString& adapterId = QString())
{
    const QList<deviceId_t> deviceIds = pModel->deviceList();
    const deviceId_t devId = deviceIds.isEmpty() ? Device::cFirstDeviceId : deviceIds.last() + 1;
    seedDevice(pModel, devId, adapterId);
    return devId;
}

//! Remove a single device, leaving the rest of the list untouched.
inline void dropDevice(SettingsModel* pModel, deviceId_t devId)
{
    QMap<deviceId_t, Device> devices = currentDevices(pModel);
    devices.remove(devId);
    pModel->applyDeviceList(devices);
}

//! Remove every device.
inline void clearDevices(SettingsModel* pModel)
{
    pModel->applyDeviceList(QMap<deviceId_t, Device>());
}

} // namespace DeviceListHelpers

#endif // DEVICELISTHELPERS_H
