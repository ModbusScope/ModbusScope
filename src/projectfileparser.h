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
        _RegisterSettings() : bActive(false), bUnsigned(false), scaleFactor(1), bitmask(0xFFFF), bColor(false) {}

        quint16 address;
        QString text;
        bool bActive;
        bool bUnsigned;
        double scaleFactor;
        quint16 bitmask;

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
        bool bLegendPosition;
        quint32 legendPosition;
    } LegendSettings;

    typedef struct _GeneralSettings
    {
        _GeneralSettings() : bIp(false), bPort(false), bPollTime(false), bSlaveId(false), bTimeout(false), bConsecutiveMax(false) {}

        bool bIp;
        QString ip;

        bool bPort;
        quint32 port;

        bool bPollTime;
        quint32 pollTime;

        bool bSlaveId;
        quint8 slaveId;

        bool bTimeout;
        quint32 timeout;

        bool bConsecutiveMax;
        quint8 consecutiveMax;

    } GeneralSettings;

    typedef struct
    {
        GeneralSettings general;
        ScopeSettings scope;
        ScaleSettings scale;
        LegendSettings legend;
    } ProjectSettings;

    explicit ProjectFileParser();

    bool parseFile(QIODevice *device, ProjectSettings * pSettings);

signals:

public slots:

private:

    bool parseModbusTag(const QDomElement &element, GeneralSettings *pGeneralSettings);

    bool parseScopeTag(const QDomElement &element, ScopeSettings *pScopeSettings);
    bool parseVariableTag(const QDomElement &element, RegisterSettings *pRegisterSettings);

    bool parseScaleTag(const QDomElement &element, ScaleSettings *pScaleSettings);
    bool parseScaleXAxis(const QDomElement &element, ScaleSettings *pScaleSettings);
    bool parseScaleYAxis(const QDomElement &element, ScaleSettings *pScaleSettings);

    bool parseLegendTag(const QDomElement &element, LegendSettings *pLegendSettings);

    QDomDocument _domDocument;

    QMessageBox _msgBox;

};

#endif // PROJECTFILEPARSER_H
