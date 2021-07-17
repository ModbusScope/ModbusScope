#ifndef PROJECTFILEHANDLER_H
#define PROJECTFILEHANDLER_H

#include <QObject>

#include "guimodel.h"
#include "settingsmodel.h"
#include "graphdatamodel.h"

#include "projectfileparser.h"

class ProjectFileHandler : public QObject
{
    Q_OBJECT
public:
    explicit ProjectFileHandler(GuiModel* pGuiModel, SettingsModel* pSettingsModel, GraphDataModel* pGraphDataModel);

    void openProjectFile(QString projectFilePath);

signals:

public slots:
    void selectSettingsExportFile();
    void selectProjectSettingFile();

    void reloadProjectFile();

private:

    void updateProjectSetting(ProjectFileData::ProjectSettings *pProjectSettings);

    GuiModel* _pGuiModel;
    SettingsModel* _pSettingsModel;
    GraphDataModel* _pGraphDataModel;

};

#endif // PROJECTFILEHANDLER_H
