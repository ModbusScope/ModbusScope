#include "settingsmodel.h"

#include "ProtocolAdapter/adapterhub.h"
#include "util/scopelogging.h"

#include <QJsonArray>
#include <QSet>

namespace {

/*!
 * \brief Build a human-readable summary of an adapter's parsed license info.
 * \param info The license info parsed from an adapter.describe response.
 * \return An empty string when the adapter didn't report any license info.
 */
QString licenseLogText(const AdapterLicenseInfo& info)
{
    if (info.state == AdapterLicenseInfo::State::NotFound)
    {
        return QString("license not found (searched %1)").arg(info.path);
    }
    if (info.state == AdapterLicenseInfo::State::Invalid)
    {
        return QString("license invalid: %1 (path: %2)").arg(info.reason, info.path);
    }
    if (info.state == AdapterLicenseInfo::State::Valid)
    {
        QString text = QString("license valid for %1").arg(info.customer);
        if (!info.expires.isEmpty())
        {
            text += QString(", expires %1").arg(info.expires);
        }
        return text;
    }

    return QString();
}

} // namespace

SettingsModel::SettingsModel(QObject* parent) : QObject(parent)
{
    _devices[Device::cFirstDeviceId] = Device(Device::cFirstDeviceId);

    _pollTime = 250;
    _bAbsoluteTimes = false;
    _bWriteDuringLog = true;
    _writeDuringLogFile = SettingsModel::defaultLogPath();
}

SettingsModel::~SettingsModel()
{
}

void SettingsModel::triggerUpdate(void)
{
    emit pollTimeChanged();
    emit writeDuringLogChanged();
    emit writeDuringLogFileChanged();
    emit absoluteTimesChanged();
}

void SettingsModel::setPollTime(quint32 pollTime)
{
    if (_pollTime != pollTime)
    {
        _pollTime = pollTime;
        emit pollTimeChanged();
    }
}

quint32 SettingsModel::pollTime()
{
    return _pollTime;
}

void SettingsModel::setAbsoluteTimes(bool bAbsolute)
{
    if (_bAbsoluteTimes != bAbsolute)
    {
        _bAbsoluteTimes = bAbsolute;
        emit absoluteTimesChanged();
    }
}

bool SettingsModel::absoluteTimes()
{
    return _bAbsoluteTimes;
}

deviceId_t SettingsModel::addNewDevice()
{
    deviceId_t newId = _devices.isEmpty() ? Device::cFirstDeviceId : static_cast<deviceId_t>(_devices.lastKey() + 1);

    _devices[newId] = Device(newId);

    emit deviceListChanged();
    return newId;
}

void SettingsModel::addDevice(deviceId_t devId)
{
    if (!_devices.contains(devId))
    {
        _devices[devId] = Device(devId);
        emit deviceListChanged();
    }
}

void SettingsModel::removeDevice(deviceId_t devId)
{
    _devices.remove(devId);
    emit deviceListChanged();
}

void SettingsModel::removeAllDevice()
{
    _devices.clear();
    emit deviceListChanged();
}

QList<deviceId_t> SettingsModel::deviceList()
{
    QList<deviceId_t> list;

    for (auto i = _devices.cbegin(), end = _devices.cend(); i != end; ++i)
    {
        list.append(i.key());
    }

    return list;
}

/*! \brief Return the adapter ID that owns the given device.
 *
 * When the device is unknown, the adapter ID of a default-constructed Device
 * ("modbus") is returned without inserting a new device entry.
 * \param devId  The device identifier.
 * \return The adapter ID string associated with the device.
 */
QString SettingsModel::adapterIdForDevice(deviceId_t devId) const
{
    return _devices.value(devId).adapterId();
}

void SettingsModel::setWriteDuringLog(bool bState)
{
    if (_bWriteDuringLog != bState)
    {
        _bWriteDuringLog = bState;
        emit writeDuringLogChanged();
    }
}

bool SettingsModel::writeDuringLog()
{
    return _bWriteDuringLog;
}

void SettingsModel::setWriteDuringLogFile(QString path)
{
    if (_writeDuringLogFile != path)
    {
        _writeDuringLogFile = path;
        emit writeDuringLogFileChanged();
    }
}

void SettingsModel::setWriteDuringLogFileToDefault(void)
{
    setWriteDuringLogFile(defaultLogPath());
}

QString SettingsModel::writeDuringLogFile()
{
    return _writeDuringLogFile;
}

Device* SettingsModel::deviceSettings(deviceId_t devId)
{
#if 0
TODO: dev
Check validity
#endif
    return &_devices[devId];
}

/*! \brief Return a pointer to the AdapterData for the given adapter ID.
 * \param adapterId  The adapter identifier string.
 * \return Pointer to the AdapterData entry; inserts a default entry if not present.
 */
const AdapterData* SettingsModel::adapterData(const QString& adapterId)
{
    if (!_adapters.contains(adapterId))
    {
        _adapters[adapterId] = AdapterData();
    }
    return &_adapters[adapterId];
}

/*! \brief Return the list of registered adapter IDs.
 * \return QStringList of all adapter ID strings currently in the model.
 */
QStringList SettingsModel::adapterIds() const
{
    return _adapters.keys();
}

/*! \brief Remove the adapter entry with the given ID.
 * \param adapterId  The adapter identifier string to remove.
 */
void SettingsModel::removeAdapter(const QString& adapterId)
{
    _adapters.remove(adapterId);
}

/*! \brief Return whether any registered adapter reports mbcCompatible capability.
 * \return True if at least one adapter has mbcCompatible: true in its capabilities.
 */
bool SettingsModel::isMbcCompatible() const
{
    for (const AdapterData& adapter : _adapters)
    {
        if (adapter.isMbcCompatible())
        {
            return true;
        }
    }
    return false;
}

/*! \brief Persist a new configuration for an adapter and notify observers.
 *
 * Sets the adapter's current config and marks it as having a stored config, then emits
 * adapterDataChanged() so listeners can react. Deliberately does not reconcile device
 * ownership itself: a device named in the stored config may not be meant to exist in
 * \c _devices yet (e.g. a project file's devices section can omit a device the adapter
 * config still lists) — registration and ownership stay driven by
 * reconcileDevicesWithAdapters()'s own trigger points (updateAdapterFromDescribe() and
 * AdapterDeviceSettings opening), not by every config write.
 * \param adapterId  The adapter identifier string.
 * \param config     The configuration JSON object to store.
 */
void SettingsModel::setAdapterCurrentConfig(const QString& adapterId, const QJsonObject& config)
{
    if (!_adapters.contains(adapterId))
    {
        _adapters[adapterId] = AdapterData();
    }
    _adapters[adapterId].setCurrentConfig(config);
    _adapters[adapterId].setHasStoredConfig(true);
    emit adapterDataChanged(adapterId);
}

/*! \brief Store the data point schema from an adapter.dataPointSchema response and notify observers.
 * \param adapterId  The adapter identifier string.
 * \param schema     The full data point schema object (addressSchema, defaults).
 */
void SettingsModel::setAdapterDataPointSchema(const QString& adapterId, const QJsonObject& schema)
{
    if (!_adapters.contains(adapterId))
    {
        _adapters[adapterId] = AdapterData();
    }
    _adapters[adapterId].setDataPointSchema(schema);
    emit adapterDataChanged(adapterId);
}

void SettingsModel::updateAdapterFromDescribe(const QString& adapterId, const QJsonObject& describeResult)
{
    if (!_adapters.contains(adapterId))
    {
        _adapters[adapterId] = AdapterData();
    }
    _adapters[adapterId].updateFromDescribe(describeResult);
    reconcileDevicesWithAdapters();

    const QString version = _adapters[adapterId].version();
    QString versionTxt("unknown version");
    if (!version.isEmpty())
    {
        versionTxt = QString(tr("v%1")).arg(version);
    }
    qCInfo(scopeComm) << qUtf8Printable(QString("Adapter %1: %2").arg(adapterId, versionTxt));

    const AdapterLicenseInfo licenseInfo = _adapters[adapterId].licenseInfo();
    const QString licenseText = licenseLogText(licenseInfo);
    if (!licenseText.isEmpty())
    {
        if (licenseInfo.state == AdapterLicenseInfo::State::Valid)
        {
            qCInfo(scopeComm) << qUtf8Printable(QString("Adapter %1: %2").arg(adapterId, licenseText));
        }
        else
        {
            qCWarning(scopeComm) << qUtf8Printable(QString("Adapter %1: %2").arg(adapterId, licenseText));
        }
    }

    emit adapterDataChanged(adapterId);
}

/*! \brief Claim each adapter-declared default device ID for the adapter that should own it.
 *
 * For every adapter with known schema, walks its effective config's device list and, for any
 * device ID not already claimed earlier in the resulting order, ensures the device exists and
 * assigns it that adapter. This keeps a device ID shared by two adapters' defaults (e.g. both
 * declaring device 1) consistently assigned to one adapter as soon as adapter data is available,
 * instead of leaving it on Device's constructor default until the user opens Settings.
 *
 * Adapters with an explicitly saved config (hasStoredConfig()) are considered before adapters
 * still on raw, never-configured defaults, so a device the user deliberately assigned to one
 * adapter can't later be reclaimed by another adapter that merely happens to share that device ID
 * in its untouched defaults — this method re-runs on every describe, including reconnects, so a
 * once-off "first adapter alphabetically wins" rule would let a later reconnect silently steal a
 * device back from its explicitly configured owner. Within each of those two groups, adapters are
 * considered in adapterIds() order, so a device ID shared by two equally-unconfigured (or two
 * equally-configured) adapters still resolves deterministically, just not meaningfully — that
 * remaining tie only matters before either adapter has been explicitly configured. The one
 * exception: within the unconfigured group, "modbus" is moved to the front of that order when
 * present, so the app's built-in initial device (Device::cFirstDeviceId, "modbus" by
 * construction) can't be silently reassigned to another adapter that merely happens to sort
 * alphabetically ahead of it and share the same default device ID.
 *
 * Any code that needs to know which adapter owns a device must read it back via
 * deviceSettings()/adapterIdForDevice() rather than re-deriving ownership itself (e.g. by picking
 * the first adapter that declares a given device ID) — that would disagree with this method's
 * tie-break and could silently discard the reconciled owner's config.
 *
 * Deliberately does not remove devices no adapter currently declares — that pruning is only safe
 * once every relevant adapter's config is known, whereas this runs incrementally as each adapter
 * describes and must not delete devices belonging to adapters that simply haven't described yet.
 */
void SettingsModel::reconcileDevicesWithAdapters()
{
    QStringList validAdapterIds;
    const QStringList allAdapterIds = adapterIds();
    for (const auto& id : allAdapterIds)
    {
        if (!_adapters.value(id).schema().isEmpty())
        {
            validAdapterIds.append(id);
        }
    }

    QStringList orderedAdapterIds;
    for (const auto& id : validAdapterIds)
    {
        if (_adapters.value(id).hasStoredConfig())
        {
            orderedAdapterIds.append(id);
        }
    }

    QStringList unconfiguredAdapterIds;
    for (const auto& id : validAdapterIds)
    {
        if (!_adapters.value(id).hasStoredConfig())
        {
            unconfiguredAdapterIds.append(id);
        }
    }
    const int modbusIndex = unconfiguredAdapterIds.indexOf(QString(cModbusAdapterId));
    if (modbusIndex > 0)
    {
        unconfiguredAdapterIds.move(modbusIndex, 0);
    }
    orderedAdapterIds.append(unconfiguredAdapterIds);

    QSet<deviceId_t> seenDeviceIds;
    bool ownerChanged = false;
    for (const auto& adapterId : orderedAdapterIds)
    {
        const QJsonArray devices = _adapters.value(adapterId).effectiveConfig().value("devices").toArray();
        for (const auto& device : devices)
        {
            const int id = device.toObject().value("id").toInt(-1);
            if (id < 0)
            {
                continue;
            }

            const deviceId_t devId = static_cast<deviceId_t>(id);
            if (seenDeviceIds.contains(devId))
            {
                continue;
            }
            seenDeviceIds.insert(devId);

            const bool alreadyKnownDevice = _devices.contains(devId);
            addDevice(devId);
            if (deviceSettings(devId)->adapterId() != adapterId)
            {
                deviceSettings(devId)->setAdapterId(adapterId);
                if (alreadyKnownDevice)
                {
                    /* addDevice() only emits deviceListChanged() for brand-new devices; an
                     * already-known device silently changing owner needs its own notification,
                     * deferred until the end so a pass reassigning several devices only emits once. */
                    ownerChanged = true;
                }
            }
        }
    }

    if (ownerChanged)
    {
        emit deviceListChanged();
    }
}

bool SettingsModel::hasDevice(deviceId_t devId) const
{
    return _devices.contains(devId);
}
