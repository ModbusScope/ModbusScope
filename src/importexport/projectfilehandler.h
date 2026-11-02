#ifndef PROJECTFILEHANDLER_H
#define PROJECTFILEHANDLER_H

#include "projectfiledata.h"

#include <QMap>
#include <QObject>
#include <QStringList>

// Forward declaration
class GuiModel;
class SettingsModel;
class GraphDataModel;

class ProjectFileHandler : public QObject
{
    Q_OBJECT
public:
    explicit ProjectFileHandler(GuiModel* pGuiModel,
                                SettingsModel* pSettingsModel,
                                GraphDataModel* pGraphDataModel,
                                QObject* parent = nullptr);

    void openProjectFile(QString projectFilePath);

signals:

public slots:
    void selectProjectSaveFile();
    void saveProjectFile();
    void selectProjectOpenFile();

    void reloadProjectFile();

private:
    void updateProjectSetting(ProjectFileData::ProjectSettings* pProjectSettings);

    void applyLogSettings(const ProjectFileData::LogSettings& logSettings);
    void applyViewSettings(const ProjectFileData::ViewSettings& viewSettings);
    void applyGraphData(const ProjectFileData::ScopeSettings& scopeSettings);
    void applyAdapterSettings(const QList<ProjectFileData::AdapterFileSettings>& adapters);

    static QMap<deviceId_t, Device> buildDeviceList(const QList<ProjectFileData::DeviceSettings>& deviceSettings);
    static void alignDevicesWithAdapterConfigs(QList<ProjectFileData::AdapterFileSettings>& adapters,
                                               QMap<deviceId_t, Device>& devices);
    static QMap<deviceId_t, QStringList> collectDeviceClaims(
      const QList<ProjectFileData::AdapterFileSettings>& adapters, const QMap<QString, qsizetype>& listingAdapters);

    QList<ProjectFileData::AdapterFileSettings> buildCurrentAdapters();
    QList<ProjectFileData::DeviceSettings> buildCurrentDevices(
      const QList<ProjectFileData::AdapterFileSettings>& adapters);

    GuiModel* _pGuiModel;
    SettingsModel* _pSettingsModel;
    GraphDataModel* _pGraphDataModel;

    QList<ProjectFileData::AdapterFileSettings> _storedAdapters;
};

#endif // PROJECTFILEHANDLER_H
