#ifndef PROJECTFILEDATA
#define PROJECTFILEDATA

#include "models/connectiontypes.h"
#include "models/device.h"
#include <QColor>
#include <QList>

namespace ProjectFileData
{
    typedef struct _RegisterSettings
    {
        QString text = QString("");
        bool bActive = false;
        ConnectionTypes::connectionId_t connectionId = 0;

        QString expression = QString("0");

        bool bColor = false;
        QColor color;

        quint32 valueAxis = 0;

    } RegisterSettings;

    typedef struct
    {
        QList<RegisterSettings> registerList;
    } ScopeSettings;

    typedef struct _XAxisSettings
    {
        bool bSliding = false;
        quint32 slidingInterval = 60;

    } XAxisSettings;

    typedef struct _YAxisSettings
    {
        bool bWindowScale = false;

        bool bMinMax = false;
        double scaleMin = 0;
        double scaleMax = 10;
    } YAxisSettings;

    typedef struct _ScaleSettings
    {
        XAxisSettings xAxis;
        YAxisSettings yAxis;
        YAxisSettings y2Axis;
    } ScaleSettings;

    typedef struct
    {
        ScaleSettings scaleSettings;

    } ViewSettings;

    typedef struct _LogSettings
    {
        bool bPollTime = false;
        quint32 pollTime;

        bool bAbsoluteTimes = false;

        bool bLogToFile = true;
        bool bLogToFileFile = false;
        QString logFile;

    } LogSettings;

    typedef struct _ConnectionSettings
    {
        bool bConnectionId = false;
        ConnectionTypes::connectionId_t connectionId = 0;

        bool bConnectionState = true;

        bool bConnectionType = false;
        QString connectionType;

        bool bIp = false;
        QString ip;

        bool bPort = false;
        quint16 port;

        bool bPortName = false;
        QString portName;

        bool bBaudrate = false;
        quint32 baudrate;

        bool bParity = false;
        quint32 parity = 0;

        bool bStopbits = false;
        quint32 stopbits;

        bool bDatabits = false;
        quint32 databits;

        bool bTimeout = false;
        quint32 timeout;

        bool bPersistentConnection = true;

    } ConnectionSettings;

    typedef struct _DeviceSettings
    {
        bool bDeviceId = false;
        deviceId_t deviceId = Device::cFirstDeviceId;

        bool bName = false;
        QString name = QString("");

        bool bConnectionId = false;
        ConnectionTypes::connectionId_t connectionId = 0;

        bool bSlaveId = false;
        quint8 slaveId;

        bool bConsecutiveMax = false;
        quint8 consecutiveMax;

        bool bInt32LittleEndian = true;

    } DeviceSettings;

    typedef struct _GeneralSettings
    {
        QList<ConnectionSettings> connectionSettings;
        QList<DeviceSettings> deviceSettings;
        LogSettings logSettings;

    } GeneralSettings;

    typedef struct
    {
        GeneralSettings general;
        ScopeSettings scope;
        ViewSettings view;
    } ProjectSettings;
}

#endif // PROJECTFILEDATA

