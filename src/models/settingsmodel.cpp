#include "settingsmodel.h"
#include "util/scopelogging.h"

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

QList<deviceId_t> SettingsModel::deviceListForAdapter(const QString& adapterId)
{
    QList<deviceId_t> list;

    for (auto i = _devices.cbegin(), end = _devices.cend(); i != end; ++i)
    {
        if (i.value().adapterId() == adapterId)
        {
            list.append(i.key());
        }
    }

    return list;
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

/*! \brief Persist a new configuration for an adapter and notify observers.
 *
 * Sets the adapter's current config and marks it as having a stored config,
 * then emits adapterDataChanged() so listeners can react.
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
 * \param schema     The full data point schema object (addressSchema, dataTypes, defaultDataType).
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

    const QString version = _adapters[adapterId].version();
    QString versionTxt("unknown version");
    if (!version.isEmpty())
    {
        versionTxt = QString(tr("v%1")).arg(version);
    }
    qCInfo(scopeComm) << qUtf8Printable(QString("Adapter %1: %2").arg(adapterId, versionTxt));

    emit adapterDataChanged(adapterId);
}

bool SettingsModel::hasDevice(deviceId_t devId) const
{
    return _devices.contains(devId);
}

bool SettingsModel::updateDeviceId(deviceId_t oldId, deviceId_t newId)
{
    if (!_devices.contains(oldId) || _devices.contains(newId))
        return false;

    Device device = _devices.take(oldId);
    _devices.insert(newId, device);
    emit deviceListChanged();
    return true;
}
