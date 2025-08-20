
#include "settingsmodel.h"
#include "models/connectiontypes.h"

SettingsModel::SettingsModel(QObject *parent) :
    QObject(parent)
{

    for (quint8 i = 0; i < ConnectionId::ID_CNT; i++)
    {
        ConnectionSettings connectionSettings;
        _connectionSettings.append(connectionSettings);
    }

    /* Connection 1 is always enabled */
    _connectionSettings[ConnectionId::ID_1].bConnectionState = true;

    _devices[Device::cFirstDeviceId] = Device();

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

void SettingsModel::addDevice(deviceId_t devId)
{
    if (!_devices.contains(devId))
    {
        _devices[devId] = Device();
    }
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

QList<deviceId_t> SettingsModel::deviceListForConnection(connectionId_t connectionId)
{
    QList<deviceId_t> list;

    for (auto i = _devices.cbegin(), end = _devices.cend(); i != end; ++i)
    {
        if (static_cast<Device>(i.value()).connectionId() == connectionId)
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

Connection* SettingsModel::connectionSettings(connectionId_t connectionId)
{
    return &_connectionSettings[connectionId].connectionData;
}

Device* SettingsModel::deviceSettings(deviceId_t devId)
{
#if 0
TODO: dev
Check validity
#endif
    return &_devices[devId];
}

void SettingsModel::setConnectionState(connectionId_t connectionId, bool bState)
{
    /* Connection 1 can't be disabled */
    if (connectionId == ConnectionId::ID_1)
    {
        bState = true;
    }

    _connectionSettings[connectionId].bConnectionState = bState;
}

bool SettingsModel::connectionState(connectionId_t connectionId)
{
    return _connectionSettings[connectionId].bConnectionState;
}
