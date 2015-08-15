#ifndef PROJECTFILEPARSER_H
#define PROJECTFILEPARSER_H

#include <QObject>
#include <QMessageBox>
#include <QDomDocument>

class ProjectFileParser : public QObject
{
    Q_OBJECT
public:

    typedef struct _RegisterSettings
    {
        _RegisterSettings() : address(40001), text(""), bActive(true), bUnsigned(false), divideFactor(1), multiplyFactor(1), bitmask(0xFFFF), shift(0), bColor(false) {}

        quint16 address;
        QString text;
        bool bActive;
        bool bUnsigned;
        double divideFactor;
        double multiplyFactor;
        quint16 bitmask;
        quint32 shift;

        bool bColor;
        QColor color;

    } RegisterSettings;

    typedef struct
    {
        QList<RegisterSettings> registerList;
    } ScopeSettings;

    typedef struct _ScaleSettings
    {
        _ScaleSettings() : bSliding(false), bMinMax(false) {}

        bool bSliding;
        quint32 slidingInterval;

        bool bMinMax;
        qint32 scaleMin;
        qint32 scaleMax;
    } ScaleSettings;

    typedef struct
    {
        bool bPosition;
        quint32 position;

        bool bVisible;

    } LegendSettings;

    typedef struct
    {
        LegendSettings legendSettings;
        ScaleSettings scaleSettings;

    } ViewSettings;

    typedef struct _LogSettings
    {
        _LogSettings() : bPollTime(false), bAbsoluteTimes(false) {}

        bool bPollTime;
        quint32 pollTime;

        bool bAbsoluteTimes;

    } LogSettings;

    typedef struct _ConnectionSettings
    {
        _ConnectionSettings() : bIp(false), bPort(false), bSlaveId(false), bTimeout(false), bConsecutiveMax(false) {}

        bool bIp;
        QString ip;

        bool bPort;
        quint32 port;

        bool bSlaveId;
        quint8 slaveId;

        bool bTimeout;
        quint32 timeout;

        bool bConsecutiveMax;
        quint8 consecutiveMax;

    } ConnectionSettings;

    typedef struct _GeneralSettings
    {
        ConnectionSettings connectionSettings;
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

    bool parseConnectionTag(const QDomElement &element, ConnectionSettings *pGeneralSettings);
    bool parseLogTag(const QDomElement &element, LogSettings *pGeneralSettings);

    bool parseScopeTag(const QDomElement &element, ScopeSettings *pScopeSettings);
    bool parseRegisterTag(const QDomElement &element, RegisterSettings *pRegisterSettings);

    bool parseViewTag(const QDomElement &element, ViewSettings *pViewSettings);
    bool parseScaleTag(const QDomElement &element, ScaleSettings *pScaleSettings);
    bool parseScaleXAxis(const QDomElement &element, ScaleSettings *pScaleSettings);
    bool parseScaleYAxis(const QDomElement &element, ScaleSettings *pScaleSettings);
    bool parseLegendTag(const QDomElement &element, LegendSettings *pLegendSettings);

    QDomDocument _domDocument;

    QMessageBox _msgBox;

};

#endif // PROJECTFILEPARSER_H
