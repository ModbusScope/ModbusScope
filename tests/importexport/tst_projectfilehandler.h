#ifndef TST_PROJECTFILEHANDLER_H
#define TST_PROJECTFILEHANDLER_H

#include "models/settingsmodel.h"

#include <QObject>
#include <QStringList>

/*!
 * \brief Records a device's name as it stands each time deviceListChanged() is emitted.
 *
 * Observers rebuild their view of the device list from inside that signal, so what they
 * read there is what ends up on screen. Checking the name only once the load has finished
 * would pass even when the name was applied too late to reach them.
 */
class DeviceNameProbe : public QObject
{
    Q_OBJECT

public:
    explicit DeviceNameProbe(SettingsModel* pSettingsModel, deviceId_t devId, QObject* parent = nullptr)
        : QObject(parent), _pSettingsModel(pSettingsModel), _devId(devId)
    {
        connect(_pSettingsModel, &SettingsModel::deviceListChanged, this, &DeviceNameProbe::recordName);
    }

    QStringList names;

private slots:
    void recordName()
    {
        if (_pSettingsModel->hasDevice(_devId))
        {
            names.append(_pSettingsModel->deviceSettings(_devId)->name());
        }
    }

private:
    SettingsModel* _pSettingsModel;
    deviceId_t _devId;
};

class TestProjectFileHandler : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void applyDeviceSettingsAppliesAdapterId();
    void applyDeviceSettingsAppliesName();
    void applyDeviceSettingsAppliesNameBeforeNotifying();
    void applyDeviceSettingsMultipleAdapters();
    void applyDeviceSettingsWithoutNameKeepsDefault();
    void applyDeviceSettingsEmitsDeviceListChangedOnce();
    void applyDeviceSettingsEmptyListClearsModel();
    void applyDeviceSettingsClearsPreviousDevices();

    void loadSetsAdapterConfigInSettingsModel();
    void savePreservesAdapterConfigAfterDialogAccept();
    void roundTripPreservesAdapterConfig();

    void applyAdapterSettingsDoesNotTruncateDevicesExceedingMaxItems();
    void applyDeviceSettingsDoesNotTruncateDevicesExceedingMaxItems();
    void applyDeviceSettingsNoLimitWhenSchemaAbsent();
    void applyDeviceSettingsNotCappedByOtherAdaptersLowerLimit();
    void openingProjectWithExcessDevicesKeepsAdapterConfigAndDeviceListConsistent();
};

#endif /* TST_PROJECTFILEHANDLER_H */
