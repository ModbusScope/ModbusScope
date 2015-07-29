
#include <QDir>

#include "settingsmodel.h"

SettingsModel::SettingsModel(QObject *parent) :
    QObject(parent)
{
    QString tempDir = QDir::toNativeSeparators(QDir::tempPath());

    if (tempDir.right(1) != QDir::separator())
    {
        tempDir.append(QDir::separator());
    }

    _pollTime = 1000;
    _bWriteDuringLog = true;
    _writeDuringLogPath = tempDir.append(tr("ModbusScope-autolog.csv"));
    _ipAddress = "127.0.0.1";
    _port = 502;
    _slaveId = 1;
    _timeout = 1000;
    _bAbsoluteTimes = false;
}

SettingsModel::~SettingsModel()
{

}

void SettingsModel::triggerUpdate(void)
{
    emit pollTimeChanged();
    emit writeDuringLogChanged();
    emit writeDuringLogPathChanged();
    emit ipChanged();
    emit portChanged();
    emit slaveIdChanged();
    emit timeoutChanged();
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

quint32 SettingsModel::writeDuringLog()
{
    return _bWriteDuringLog;
}

void SettingsModel::setWriteDuringLogPath(QString path)
{
    if (_writeDuringLogPath != path)
    {
        _writeDuringLogPath = path;
        emit writeDuringLogPathChanged();
    }
}

QString SettingsModel::writeDuringLogPath()
{
    return _writeDuringLogPath;
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


