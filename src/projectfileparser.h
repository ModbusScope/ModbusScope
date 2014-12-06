#ifndef PROJECTFILEPARSER_H
#define PROJECTFILEPARSER_H

#include <QObject>
#include <QMessageBox>
#include <QDomDocument>

class ProjectFileParser : public QObject
{
    Q_OBJECT
public:

    typedef struct
    {
        quint16 address;
        QString text;
        bool bActive;
    } RegisterSettings;

    typedef struct
    {
        QList<RegisterSettings> registerList;
    } ScopeSettings;

    typedef struct
    {
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

    } GeneralSettings;

    typedef struct
    {
        GeneralSettings general;
        ScopeSettings scope;
    } ProjectSettings;

    explicit ProjectFileParser();

    bool parseFile(QIODevice *device, ProjectSettings * pSettings);

signals:

public slots:

private:

    bool parseModbusTag(const QDomElement &element, GeneralSettings *pGeneralSettings);
    bool parseScopeTag(const QDomElement &element, ScopeSettings *pScopeSettings);
    bool parseVariableTag(const QDomElement &element, RegisterSettings *pRegisterSettings);

    QDomDocument _domDocument;

    QMessageBox _msgBox;

};

#endif // PROJECTFILEPARSER_H
