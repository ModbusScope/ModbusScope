#ifndef PROJECTFILEJSONEXPORTER_H
#define PROJECTFILEJSONEXPORTER_H

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
 * Adapter-specific connection and device settings are written into the
 * adapters[].settings blob; the top-level devices array contains only generic
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
     */
    void exportProjectFile(const QString& projectFile);

private:
    QJsonArray createAdaptersArray();
    QJsonArray createDevicesArray();
    QJsonObject createLogObject();
    QJsonArray createScopeArray();
    QJsonObject createViewObject();

    GuiModel* _pGuiModel;
    SettingsModel* _pSettingsModel;
    GraphDataModel* _pGraphDataModel;
};

#endif // PROJECTFILEJSONEXPORTER_H
