#ifndef PROJECTFILEPARSER_H
#define PROJECTFILEPARSER_H

#include <QObject>
#include <QColor>
#include <QDomDocument>

class ProjectFileParser : public QObject
{
    Q_OBJECT
public:

    typedef struct _RegisterSettings
    {
        _RegisterSettings() : address(40001), text(""), bActive(true), bUnsigned(false), b32Bit(false), divideFactor(1),
                              multiplyFactor(1), bitmask(0xFFFFFFFF), shift(0), connectionId(0),
                              bColor(false) {}

        quint16 address;
        QString text;
        bool bActive;
        bool bUnsigned;
        bool b32Bit;
        double divideFactor;
        double multiplyFactor;
        quint32 bitmask;
        quint32 shift;
        quint8 connectionId;

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
        qint32 scaleMin;
        qint32 scaleMax;
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
        _ConnectionSettings() : bIp(false), bConnectionId(false), bPort(false), bSlaveId(false), bTimeout(false), bConsecutiveMax(false) {}

        bool bIp;
        QString ip;

        bool bConnectionId;
        quint8 connectionId;

        bool bPort;
        quint16 port;

        bool bSlaveId;
        quint8 slaveId;

        bool bTimeout;
        quint32 timeout;

        bool bConsecutiveMax;
        quint8 consecutiveMax;

    } ConnectionSettings;

    typedef struct _GeneralSettings
    {
        QList<ConnectionSettings> connectionSettings;
        LogSettings logSettings;

    } GeneralSettings;

    typedef struct
    {
        GeneralSettings general;
        ScopeSettings scope;
        ViewSettings view;
    } ProjectSettings;

    explicit ProjectFileParser();

    bool parseFile(QIODevice *device, ProjectSettings * pSettings);

signals:

public slots:

private:

    bool parseModbusTag(const QDomElement &element, GeneralSettings *pGeneralSettings);

    bool parseConnectionTag(const QDomElement &element, ConnectionSettings *pConnectionSettings);
    bool parseLogTag(const QDomElement &element, LogSettings *pLogSettings);
    bool parseLogToFile(const QDomElement &element, LogSettings *pLogSettings);

    bool parseScopeTag(const QDomElement &element, ScopeSettings *pScopeSettings);
    bool parseRegisterTag(const QDomElement &element, RegisterSettings *pRegisterSettings);

    bool parseViewTag(const QDomElement &element, ViewSettings *pViewSettings);
    bool parseScaleTag(const QDomElement &element, ScaleSettings *pScaleSettings);
    bool parseScaleXAxis(const QDomElement &element, ScaleSettings *pScaleSettings);
    bool parseScaleYAxis(const QDomElement &element, ScaleSettings *pScaleSettings);

    QDomDocument _domDocument;

};

#endif // PROJECTFILEPARSER_H
