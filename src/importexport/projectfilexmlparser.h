#ifndef PROJECTFILEXMLPARSER_H
#define PROJECTFILEXMLPARSER_H

#include "importexport/generalerror.h"
#include "importexport/projectfiledata.h"

#include <QDomDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>

/*!
 * \brief Parses a legacy XML-format MBS project file (data levels 3–5) and
 *        converts it into the same ProjectSettings structure used by the JSON parser.
 *
 * Connection and device settings from the XML \c \<modbus\> section are converted
 * into an adapter settings JSON blob so the result is indistinguishable from a
 * file loaded via ProjectFileJsonParser.
 */
class ProjectFileXmlParser
{
public:
    ProjectFileXmlParser();

    /*!
     * \brief Parse a legacy XML MBS project file into ProjectSettings.
     * \param fileContent Raw file contents.
     * \param pSettings   Output settings structure.
     * \return GeneralError — result() is true on success.
     */
    GeneralError parseFile(const QString& fileContent, ProjectFileData::ProjectSettings* pSettings);

private:
    GeneralError parseModbusTag(const QDomElement& element, ProjectFileData::GeneralSettings* pGeneralSettings);

    GeneralError parseConnectionTag(const QDomElement& element, QJsonObject* pConnectionJson);
    GeneralError parseLegacyConnectionTag(const QDomElement& element,
                                          QJsonObject* pConnectionJson,
                                          QJsonObject* pDeviceJson,
                                          ProjectFileData::DeviceSettings* pDeviceSettings);
    GeneralError parseDeviceTag(const QDomElement& element,
                                ProjectFileData::DeviceSettings* pDeviceSettings,
                                QJsonObject* pDeviceJson);
    GeneralError parseLogTag(const QDomElement& element, ProjectFileData::LogSettings* pLogSettings);
    GeneralError parseLogToFile(const QDomElement& element, ProjectFileData::LogSettings* pLogSettings);

    GeneralError parseScopeTag(const QDomElement& element, ProjectFileData::ScopeSettings* pScopeSettings);
    GeneralError parseRegisterTag(const QDomElement& element, ProjectFileData::RegisterSettings* pRegisterSettings);

    GeneralError parseViewTag(const QDomElement& element, ProjectFileData::ViewSettings* pViewSettings);
    GeneralError parseScaleTag(const QDomElement& element, ProjectFileData::ScaleSettings* pScaleSettings);
    GeneralError parseScaleXAxis(const QDomElement& element, ProjectFileData::ScaleSettings* pScaleSettings);
    GeneralError parseScaleYAxis(const QDomElement& element, ProjectFileData::YAxisSettings* pYAxisSettings);

    void parseConnectionFields(const QDomElement& child, QJsonObject* pConnectionJson);

    void buildAdapterSettings(const QJsonArray& connectionsArray,
                              const QJsonArray& devicesArray,
                              ProjectFileData::GeneralSettings* pGeneralSettings);

    QDomDocument _domDocument;
    quint32 _dataLevel;
};

#endif // PROJECTFILEXMLPARSER_H
