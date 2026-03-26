#ifndef PROJECTFILEJSONPARSER_H
#define PROJECTFILEJSONPARSER_H

#include "importexport/generalerror.h"
#include "importexport/projectfiledata.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

/*!
 * \brief Parses a JSON-format MBS project file (version 6+).
 *
 * The adapter-specific settings blobs (adapters[].settings) are stored as
 * opaque QJsonObjects without further interpretation by the core application.
 */
class ProjectFileJsonParser
{
public:
    ProjectFileJsonParser();

    /*!
     * \brief Parse a JSON MBS project file into ProjectSettings.
     * \param fileContent Raw file contents.
     * \param pSettings   Output settings structure.
     * \return GeneralError — result() is true on success.
     */
    GeneralError parseFile(const QString& fileContent, ProjectFileData::ProjectSettings* pSettings);

private:
    GeneralError parseAdapters(const QJsonArray& adaptersArray, ProjectFileData::GeneralSettings* pGeneralSettings);
    GeneralError parseDevices(const QJsonArray& devicesArray, ProjectFileData::GeneralSettings* pGeneralSettings);
    GeneralError parseLog(const QJsonObject& logObject, ProjectFileData::LogSettings* pLogSettings);
    GeneralError parseScope(const QJsonArray& scopeArray, ProjectFileData::ScopeSettings* pScopeSettings);
    GeneralError parseView(const QJsonObject& viewObject, ProjectFileData::ViewSettings* pViewSettings);
    GeneralError parseScaleXAxis(const QJsonObject& xaxisObject, ProjectFileData::ScaleSettings* pScaleSettings);
    GeneralError parseScaleYAxis(const QJsonObject& yaxisObject, ProjectFileData::ScaleSettings* pScaleSettings);
};

#endif // PROJECTFILEJSONPARSER_H
