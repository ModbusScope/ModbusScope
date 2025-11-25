#ifndef PROJECTFILEPARSER_H
#define PROJECTFILEPARSER_H

#include "importexport/projectfiledata.h"
#include "util/generalerror.h"

#include <QColor>
#include <QDomDocument>
#include <QObject>

class ProjectFileParser : public QObject
{
    Q_OBJECT
public:

    explicit ProjectFileParser();

    GeneralError parseFile(QString &fileContent, ProjectFileData::ProjectSettings * pSettings);

private:

    GeneralError parseModbusTag(const QDomElement &element, ProjectFileData::GeneralSettings *pGeneralSettings);

    GeneralError parseConnectionTag(const QDomElement &element, ProjectFileData::ConnectionSettings *pConnectionSettings);
    GeneralError parseLegacyConnectionTag(const QDomElement& element,
                                          ProjectFileData::ConnectionSettings* pConnectionSettings,
                                          ProjectFileData::DeviceSettings* pDeviceSettings);
    GeneralError parseDeviceTag(const QDomElement& element, ProjectFileData::DeviceSettings* pDeviceSettings);
    GeneralError parseLogTag(const QDomElement &element, ProjectFileData::LogSettings *pLogSettings);
    GeneralError parseLogToFile(const QDomElement &element, ProjectFileData::LogSettings *pLogSettings);

    GeneralError parseScopeTag(const QDomElement &element, ProjectFileData::ScopeSettings *pScopeSettings);
    GeneralError parseRegisterTag(const QDomElement &element, ProjectFileData::RegisterSettings *pRegisterSettings);

    GeneralError parseViewTag(const QDomElement &element, ProjectFileData::ViewSettings *pViewSettings);
    GeneralError parseScaleTag(const QDomElement &element, ProjectFileData::ScaleSettings *pScaleSettings);
    GeneralError parseScaleXAxis(const QDomElement &element, ProjectFileData::ScaleSettings *pScaleSettings);
    GeneralError parseScaleYAxis(const QDomElement &element, ProjectFileData::YAxisSettings *pYAxisSettings);

    QDomDocument _domDocument;
    quint32 _dataLevel;
};

#endif // PROJECTFILEPARSER_H
