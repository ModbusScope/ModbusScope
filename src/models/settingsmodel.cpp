
#include "settingsmodel.h"

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