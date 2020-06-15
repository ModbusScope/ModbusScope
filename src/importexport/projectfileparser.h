#ifndef PROJECTFILEPARSER_H
#define PROJECTFILEPARSER_H

#include <QObject>
#include <QColor>
#include <QDomDocument>

#include "projectfiledata.h"

class ProjectFileParser : public QObject
{
    Q_OBJECT
public:


    explicit ProjectFileParser();

    bool parseFile(QIODevice *device, ProjectFileData::ProjectSettings * pSettings);

signals:

public slots:

private:

    bool parseModbusTag(const QDomElement &element, ProjectFileData::GeneralSettings *pGeneralSettings);

    bool parseConnectionTag(const QDomElement &element, ProjectFileData::ConnectionSettings *pConnectionSettings);
    bool parseLogTag(const QDomElement &element, ProjectFileData::LogSettings *pLogSettings);
    bool parseLogToFile(const QDomElement &element, ProjectFileData::LogSettings *pLogSettings);

    bool parseScopeTag(const QDomElement &element, ProjectFileData::ScopeSettings *pScopeSettings);
    bool parseRegisterTag(const QDomElement &element, ProjectFileData::RegisterSettings *pRegisterSettings);

    bool parseViewTag(const QDomElement &element, ProjectFileData::ViewSettings *pViewSettings);
    bool parseScaleTag(const QDomElement &element, ProjectFileData::ScaleSettings *pScaleSettings);
    bool parseScaleXAxis(const QDomElement &element, ProjectFileData::ScaleSettings *pScaleSettings);
    bool parseScaleYAxis(const QDomElement &element, ProjectFileData::ScaleSettings *pScaleSettings);

    QDomDocument _domDocument;

};

#endif // PROJECTFILEPARSER_H
