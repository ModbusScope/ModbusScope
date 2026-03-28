#ifndef PROJECTFILEJSONEXPORTER_H
#define PROJECTFILEJSONEXPORTER_H

#include "importexport/projectfiledata.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>

// Forward declarations
class GuiModel;
class SettingsModel;
class GraphDataModel;

/*!
 * \brief Exports the current project settings to a JSON-format MBS file (version 6).
 *
 * Adapter-specific connection and device settings are written from the supplied
 * adapter and device lists; the top-level devices array contains only generic
 * device information (id, name, adapterId, adapter.type).
 */
class ProjectFileJsonExporter : public QObject
{
    Q_OBJECT
public:
    explicit ProjectFileJsonExporter(GuiModel* pGuiModel,
                                     SettingsModel* pSettingsModel,
                                     GraphDataModel* pGraphDataModel,
                                     QObject* parent = nullptr);

    /*!
     * \brief Serialise current settings and write to \a projectFile.
     * \param projectFile Destination file path.
     * \param adapters    Adapter list loaded from the project file (round-tripped as-is).
     * \param devices     Generic device list loaded from the project file (round-tripped as-is).
     */
    void exportProjectFile(const QString& projectFile,
                           const QList<ProjectFileData::AdapterFileSettings>& adapters,
                           const QList<ProjectFileData::DeviceSettings>& devices);

private:
    QJsonArray createAdaptersArray(const QList<ProjectFileData::AdapterFileSettings>& adapters);
    QJsonArray createDevicesArray(const QList<ProjectFileData::DeviceSettings>& devices);
    QJsonObject createLogObject();
    QJsonArray createScopeArray();
    QJsonObject createViewObject();

    void showExportError(const QString& projectFile);

    GuiModel* _pGuiModel;
    SettingsModel* _pSettingsModel;
    GraphDataModel* _pGraphDataModel;
};

#endif // PROJECTFILEJSONEXPORTER_H
