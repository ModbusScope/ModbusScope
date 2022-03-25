#ifndef PROJECTFILEDATA
#define PROJECTFILEDATA

#include <QColor>
#include <QList>

namespace ProjectFileData
{
    typedef struct _RegisterSettings
    {
        _RegisterSettings() : address(0xFFFFFFFF), text(""), bActive(true), bUnsigned(false), b32Bit(false), divideFactor(1),
                              multiplyFactor(1), bitmask(0xFFFFFFFF), shift(0), connectionId(0), bExpression(false),
                              bColor(false) {}

        quint32 address;
        QString text;
        bool bActive;
        bool bUnsigned;
        bool b32Bit;
        double divideFactor;
        double multiplyFactor;
        quint32 bitmask;
        qint32 shift;
        quint8 connectionId;

        bool bExpression;
        QString expression;

        bool bColor;
        QColor color;

    } RegisterSettings;

    typedef struct
    {
        QList<RegisterSettings> registerList;
    } ScopeSettings;

    typedef struct _ScaleSettings
    {
        _ScaleSettings() : bSliding(false), slidingInterval(60),
                           bWindowScale(false), bMinMax(false),
                           scaleMin(0), scaleMax(10){}

        bool bSliding;
        quint32 slidingInterval;

        bool bWindowScale;

        bool bMinMax;
        double scaleMin;
        double scaleMax;
    } ScaleSettings;

    typedef struct
    {
        ScaleSettings scaleSettings;

    } ViewSettings;

    typedef struct _LogSettings
    {
        _LogSettings() : bPollTime(false), pollTime (), bAbsoluteTimes(false), bLogToFile(true), bLogToFileFile(false) {}

        bool bPollTime;
        quint32 pollTime;

        bool bAbsoluteTimes;

        bool bLogToFile;
        bool bLogToFileFile;
        QString logFile;

    } LogSettings;

    typedef struct _ConnectionSettings
    {
        _ConnectionSettings() : bConnectionId(false),
                                bConnectionState(true), bConnectionType(false),
                                bIp(false), bPort(false),
                                bPortName(false), bBaudrate(false), bParity(false), bStopbits(false), bDatabits(false),
                                bSlaveId(false), bTimeout(false),
                                bConsecutiveMax(false), bInt32LittleEndian(true), bPersistentConnection(true) {}

        bool bConnectionId;
        quint8 connectionId;

        bool bConnectionState;

        bool bConnectionType;
        QString connectionType;

        bool bIp;
        QString ip;

        bool bPort;
        quint16 port;

        bool bPortName;
        QString portName;

        bool bBaudrate;
        quint32 baudrate;

        bool bParity;
        quint32 parity;

        bool bStopbits;
        quint32 stopbits;

        bool bDatabits;
        quint32 databits;

        bool bSlaveId;
        quint8 slaveId;

        bool bTimeout;
        quint32 timeout;

        bool bConsecutiveMax;
        quint8 consecutiveMax;

        bool bInt32LittleEndian;

        bool bPersistentConnection;

    } ConnectionSettings;

    typedef struct _GeneralSettings
    {
        QList<ConnectionSettings> connectionSettings;
        LogSettings logSettings;

    } GeneralSettings;

    typedef struct
    {
        quint32 dataLevel;
        GeneralSettings general;
        ScopeSettings scope;
        ViewSettings view;
    } ProjectSettings;
}

#endif // PROJECTFILEDATA

