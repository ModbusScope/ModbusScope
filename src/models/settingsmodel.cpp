
#include "settingsmodel.h"

SettingsModel::SettingsModel(QObject *parent) :
    QObject(parent)
{
    _connectionSettings.ipAddress = "127.0.0.1";
    _connectionSettings.port = 502;
    _connectionSettings.slaveId = 1;
    _connectionSettings.timeout = 1000;
    _connectionSettings.consecutiveMax = 125;

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
    emit ipChanged();
    emit portChanged();
    emit slaveIdChanged();
    emit timeoutChanged();
    emit absoluteTimesChanged();
    emit consecutiveMaxChanged();
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

void SettingsModel::setConsecutiveMax(quint8 max)
{
    if (_connectionSettings.consecutiveMax != max)
    {
        _connectionSettings.consecutiveMax = max;
        emit consecutiveMaxChanged();
    }
}

quint8 SettingsModel::consecutiveMax(void)
{
    return _connectionSettings.consecutiveMax;
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

void SettingsModel::setIpAddress(QString ip)
{
    if (_connectionSettings.ipAddress != ip)
    {
        _connectionSettings.ipAddress = ip;
        emit ipChanged();
    }
}

QString SettingsModel::ipAddress()
{
    return _connectionSettings.ipAddress;
}

void SettingsModel::setPort(quint16 port)
{
    if (_connectionSettings.port != port)
    {
        _connectionSettings.port = port;
        emit portChanged();
    }
}

quint16 SettingsModel::port()
{
    return _connectionSettings.port;
}

quint8 SettingsModel::slaveId()
{
    return _connectionSettings.slaveId;
}

void SettingsModel::setSlaveId(quint8 id)
{
    if (_connectionSettings.slaveId != id)
    {
        _connectionSettings.slaveId = id;
        emit slaveIdChanged();
    }
}

quint32 SettingsModel::timeout()
{
    return _connectionSettings.timeout;
}

void SettingsModel::setTimeout(quint32 timeout)
{
    if (_connectionSettings.timeout != timeout)
    {
        _connectionSettings.timeout = timeout;
        emit timeoutChanged();
    }
}


