
#include "settingsmodel.h"

SettingsModel::SettingsModel(QObject *parent) :
    QObject(parent)
{

    for(quint8 i = 0; i < CONNECTION_ID_CNT; i++)
    {
        ConnectionSettings connectionSettings;

        connectionSettings.ipAddress = "127.0.0.1";
        connectionSettings.port = 502;
        connectionSettings.slaveId = 1;
        connectionSettings.timeout = 1000;
        connectionSettings.consecutiveMax = 125;
        connectionSettings.bConnectionState = false;
        connectionSettings.bInt32LittleEndian = true;
        connectionSettings.bPersistentConnection = true;

        _connectionSettings.append(connectionSettings);
    }

    /* Connection 0 is always enabled */
    _connectionSettings[CONNECTION_ID_0].bConnectionState = true;

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

    for(quint8 i = 0; i < CONNECTION_ID_CNT; i++)
    {
        emit ipChanged(i);
        emit portChanged(i);
        emit slaveIdChanged(i);
        emit timeoutChanged(i);
        emit consecutiveMaxChanged(i);
        emit connectionStateChanged(i);
        emit int32LittleEndianChanged(i);
        emit persistentConnectionChanged(i);
    }
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

void SettingsModel::setConsecutiveMax(quint8 connectionId, quint8 max)
{
    if (connectionId >= CONNECTION_ID_CNT)
    {
        connectionId = CONNECTION_ID_0;
    }

    if (_connectionSettings[connectionId].consecutiveMax != max)
    {
        _connectionSettings[connectionId].consecutiveMax = max;
        emit consecutiveMaxChanged(connectionId);
    }
}

quint8 SettingsModel::consecutiveMax(quint8 connectionId)
{
    if (connectionId >= CONNECTION_ID_CNT)
    {
        connectionId = CONNECTION_ID_0;
    }

    return _connectionSettings[connectionId].consecutiveMax;
}

void SettingsModel::setConnectionState(quint8 connectionId, bool bState)
{
    if (connectionId >= CONNECTION_ID_CNT)
    {
        connectionId = CONNECTION_ID_0;
    }

    /* Connection 0 can't be disabled */
    if (connectionId == CONNECTION_ID_0)
    {
        bState = true;
    }

    if (_connectionSettings[connectionId].bConnectionState != bState)
    {
        _connectionSettings[connectionId].bConnectionState = bState;
        emit connectionStateChanged(connectionId);
    }

}

bool SettingsModel::connectionState(quint8 connectionId)
{
    if (connectionId >= CONNECTION_ID_CNT)
    {
        connectionId = CONNECTION_ID_0;
    }

    return _connectionSettings[connectionId].bConnectionState;
}

void SettingsModel::setInt32LittleEndian(quint8 connectionId, bool int32LittleEndian)
{
    if (connectionId >= CONNECTION_ID_CNT)
    {
        connectionId = CONNECTION_ID_0;
    }

    if (_connectionSettings[connectionId].bInt32LittleEndian != int32LittleEndian)
    {
        _connectionSettings[connectionId].bInt32LittleEndian = int32LittleEndian;
        emit int32LittleEndianChanged(connectionId);
    }
}

bool SettingsModel::int32LittleEndian(quint8 connectionId)
{
    if (connectionId >= CONNECTION_ID_CNT)
    {
        connectionId = CONNECTION_ID_0;
    }

    return _connectionSettings[connectionId].bInt32LittleEndian;
}

void SettingsModel::setPersistentConnection(quint8 connectionId, bool persistentConnection)
{
    if (connectionId >= CONNECTION_ID_CNT)
    {
        connectionId = CONNECTION_ID_0;
    }

    if (_connectionSettings[connectionId].bPersistentConnection != persistentConnection)
    {
        _connectionSettings[connectionId].bPersistentConnection = persistentConnection;
        emit persistentConnectionChanged(connectionId);
    }
}

bool SettingsModel::persistentConnection(quint8 connectionId)
{
    if (connectionId >= CONNECTION_ID_CNT)
    {
        connectionId = CONNECTION_ID_0;
    }

    return _connectionSettings[connectionId].bPersistentConnection;
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

void SettingsModel::setIpAddress(quint8 connectionId, QString ip)
{
    if (connectionId >= CONNECTION_ID_CNT)
    {
        connectionId = CONNECTION_ID_0;
    }

    if (_connectionSettings[connectionId].ipAddress != ip)
    {
        _connectionSettings[connectionId].ipAddress = ip;
        emit ipChanged(connectionId);
    }
}

QString SettingsModel::ipAddress(quint8 connectionId)
{
    if (connectionId >= CONNECTION_ID_CNT)
    {
        connectionId = CONNECTION_ID_0;
    }

    return _connectionSettings[connectionId].ipAddress;
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
        emit portChanged(connectionId);
    }
}

quint16 SettingsModel::port(quint8 connectionId)
{
    if (connectionId >= CONNECTION_ID_CNT)
    {
        connectionId = CONNECTION_ID_0;
    }

    return _connectionSettings[connectionId].port;
}

quint8 SettingsModel::slaveId(quint8 connectionId)
{
    if (connectionId >= CONNECTION_ID_CNT)
    {
        connectionId = CONNECTION_ID_0;
    }

    return _connectionSettings[connectionId].slaveId;
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
        emit slaveIdChanged(connectionId);
    }
}

quint32 SettingsModel::timeout(quint8 connectionId)
{
    if (connectionId >= CONNECTION_ID_CNT)
    {
        connectionId = CONNECTION_ID_0;
    }

    return _connectionSettings[connectionId].timeout;
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
        emit timeoutChanged(connectionId);
    }
}


