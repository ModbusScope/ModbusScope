
#include <QDebug>

#include "settingsmodel.h"

SettingsModel::SettingsModel(QObject *parent) :
    QObject(parent)
{
    _pollTime = 250;
    _bWriteDuringLog = true;
    _writeDuringLogFile = SettingsModel::defaultLogPath();
    _ipAddress = "127.0.0.1";
    _port = 502;
    _slaveId = 1;
    _timeout = 1000;
    _consecutiveMax = 125;
    _bAbsoluteTimes = false;
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
    if (_consecutiveMax != max)
    {
        _consecutiveMax = max;
        emit consecutiveMaxChanged();
    }
}

quint8 SettingsModel::consecutiveMax(void)
{
    return _consecutiveMax;
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
    if (_ipAddress != ip)
    {
        _ipAddress = ip;
        emit ipChanged();
    }
}

QString SettingsModel::ipAddress()
{
    return _ipAddress;
}

void SettingsModel::setPort(quint16 port)
{
    if (_port != port)
    {
        _port = port;
        emit portChanged();
    }
}

quint16 SettingsModel::port()
{
    return _port;
}

quint8 SettingsModel::slaveId()
{
    return _slaveId;
}

void SettingsModel::setSlaveId(quint8 id)
{
    if (_slaveId != id)
    {
        _slaveId = id;
        emit slaveIdChanged();
    }
}

quint32 SettingsModel::timeout()
{
    return _timeout;
}

void SettingsModel::setTimeout(quint32 timeout)
{
    if (_timeout != timeout)
    {
        _timeout = timeout;
        emit timeoutChanged();
    }
}


