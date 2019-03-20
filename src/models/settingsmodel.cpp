
#include "settingsmodel.h"

SettingsModel::SettingsModel(QObject *parent) :
    QObject(parent)
{

    for(int i = 0; i < CONNECTION_ID_CNT; i++)
    {
        ConnectionSettings connectionSettings;

        connectionSettings.ipAddress = "127.0.0.1";
        connectionSettings.port = 502;
        connectionSettings.slaveId = 1;
        connectionSettings.timeout = 1000;
        connectionSettings.consecutiveMax = 125;

        _connectionSettings.append(connectionSettings);
    }

    // TODO: remove hardcoded
    _connectionSettings[CONNECTION_ID_0].slaveId = 1;
    _connectionSettings[CONNECTION_ID_1].slaveId = 2;

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
    if (_connectionSettings[CONNECTION_ID_0].consecutiveMax != max)
    {
        _connectionSettings[CONNECTION_ID_0].consecutiveMax = max;
        emit consecutiveMaxChanged();
    }
}

void SettingsModel::setConsecutiveMax(quint8 connectionId, quint8 max)
{
    if (connectionId >= CONNECTION_ID_CNT)
    {
        connectionId = CONNECTION_ID_0;
    }

    if (_connectionSettings[connectionId].consecutiveMax != max)
    {
        _connectionSettings[connectionId].consecutiveMax = max;
        emit consecutiveMaxChanged();
    }
}

quint8 SettingsModel::consecutiveMax(void)
{
    return _connectionSettings[CONNECTION_ID_0].consecutiveMax;
}

quint8 SettingsModel::consecutiveMax(quint8 connectionId)
{
    if (connectionId >= CONNECTION_ID_CNT)
    {
        connectionId = CONNECTION_ID_0;
    }

    return _connectionSettings[connectionId].consecutiveMax;
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
    if (_connectionSettings[CONNECTION_ID_0].ipAddress != ip)
    {
        _connectionSettings[CONNECTION_ID_0].ipAddress = ip;
        emit ipChanged();
    }
}

void SettingsModel::setIpAddress(quint8 connectionId, QString ip)
{
    if (connectionId >= CONNECTION_ID_CNT)
    {
        connectionId = CONNECTION_ID_0;
    }

    if (_connectionSettings[connectionId].ipAddress != ip)
    {
        _connectionSettings[connectionId].ipAddress = ip;
        emit ipChanged();
    }
}

QString SettingsModel::ipAddress()
{
    return _connectionSettings[CONNECTION_ID_0].ipAddress;
}

QString SettingsModel::ipAddress(quint8 connectionId)
{
    if (connectionId >= CONNECTION_ID_CNT)
    {
        connectionId = CONNECTION_ID_0;
    }

    return _connectionSettings[connectionId].ipAddress;
}

void SettingsModel::setPort(quint16 port)
{
    if (_connectionSettings[CONNECTION_ID_0].port != port)
    {
        _connectionSettings[CONNECTION_ID_0].port = port;
        emit portChanged();
    }
}

void SettingsModel::setPort(quint8 connectionId, quint16 port)
{
    if (connectionId >= CONNECTION_ID_CNT)
    {
        connectionId = CONNECTION_ID_0;
    }

    if (_connectionSettings[connectionId].port != port)
    {
        _connectionSettings[connectionId].port = port;
        emit portChanged();
    }
}

quint16 SettingsModel::port()
{
    return _connectionSettings[CONNECTION_ID_0].port;
}

quint16 SettingsModel::port(quint8 connectionId)
{
    if (connectionId >= CONNECTION_ID_CNT)
    {
        connectionId = CONNECTION_ID_0;
    }

    return _connectionSettings[connectionId].port;
}

quint8 SettingsModel::slaveId()
{
    return _connectionSettings[CONNECTION_ID_0].slaveId;
}

quint8 SettingsModel::slaveId(quint8 connectionId)
{
    if (connectionId >= CONNECTION_ID_CNT)
    {
        connectionId = CONNECTION_ID_0;
    }

    return _connectionSettings[connectionId].slaveId;
}

void SettingsModel::setSlaveId(quint8 id)
{
    if (_connectionSettings[CONNECTION_ID_0].slaveId != id)
    {
        _connectionSettings[CONNECTION_ID_0].slaveId = id;
        emit slaveIdChanged();
    }
}

void SettingsModel::setSlaveId(quint8 connectionId, quint8 id)
{
    if (connectionId >= CONNECTION_ID_CNT)
    {
        connectionId = CONNECTION_ID_0;
    }

    if (_connectionSettings[connectionId].slaveId != id)
    {
        _connectionSettings[connectionId].slaveId = id;
        emit slaveIdChanged();
    }
}

quint32 SettingsModel::timeout()
{
    return _connectionSettings[CONNECTION_ID_0].timeout;
}

quint32 SettingsModel::timeout(quint8 connectionId)
{
    if (connectionId >= CONNECTION_ID_CNT)
    {
        connectionId = CONNECTION_ID_0;
    }

    return _connectionSettings[connectionId].timeout;
}

void SettingsModel::setTimeout(quint32 timeout)
{
    if (_connectionSettings[CONNECTION_ID_0].timeout != timeout)
    {
        _connectionSettings[CONNECTION_ID_0].timeout = timeout;
        emit timeoutChanged();
    }
}

void SettingsModel::setTimeout(quint8 connectionId, quint32 timeout)
{
    if (connectionId >= CONNECTION_ID_CNT)
    {
        connectionId = CONNECTION_ID_0;
    }

    if (_connectionSettings[connectionId].timeout != timeout)
    {
        _connectionSettings[connectionId].timeout = timeout;
        emit timeoutChanged();
    }
}


