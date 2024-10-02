#ifndef PROJECTFILEPARSER_H
#define PROJECTFILEPARSER_H

#include <QObject>
#include <QColor>
#include <QDomDocument>

#include "generalerror.h"
#include "projectfiledata.h"

class ProjectFileParser : public QObject
{
    Q_OBJECT
public:

    explicit ProjectFileParser();

    GeneralError parseFile(QString &fileContent, ProjectFileData::ProjectSettings * pSettings);

private:

    GeneralError parseModbusTag(const QDomElement &element, ProjectFileData::GeneralSettings *pGeneralSettings);

    GeneralError parseConnectionTag(const QDomElement &element, ProjectFileData::ConnectionSettings *pConnectionSettings);
    GeneralError parseLogTag(const QDomElement &element, ProjectFileData::LogSettings *pLogSettings);
    GeneralError parseLogToFile(const QDomElement &element, ProjectFileData::LogSettings *pLogSettings);

    GeneralError parseScopeTag(const QDomElement &element, ProjectFileData::ScopeSettings *pScopeSettings);
    GeneralError parseRegisterTag(const QDomElement &element, ProjectFileData::RegisterSettings *pRegisterSettings);

    GeneralError parseViewTag(const QDomElement &element, ProjectFileData::ViewSettings *pViewSettings);
    GeneralError parseScaleTag(const QDomElement &element, ProjectFileData::ScaleSettings *pScaleSettings);
    GeneralError parseScaleXAxis(const QDomElement &element, ProjectFileData::ScaleSettings *pScaleSettings);
    GeneralError parseScaleYAxis(const QDomElement &element, ProjectFileData::YAxisSettings *pYAxisSettings);

    QDomDocument _domDocument;

};

#endif // PROJECTFILEPARSER_H
