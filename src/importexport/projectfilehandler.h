#ifndef PROJECTFILEHANDLER_H
#define PROJECTFILEHANDLER_H

#include "projectfiledata.h"

#include <QObject>

// Forward declaration
class GuiModel;
class SettingsModel;
class GraphDataModel;

class ProjectFileHandler : public QObject
{
    Q_OBJECT
public:
    explicit ProjectFileHandler(GuiModel* pGuiModel, SettingsModel* pSettingsModel, GraphDataModel* pGraphDataModel);

    void openProjectFile(QString projectFilePath);

signals:

public slots:
    void selectProjectSaveFile();
    void saveProjectFile();
    void selectProjectOpenFile();

    void reloadProjectFile();

private:

    void updateProjectSetting(ProjectFileData::ProjectSettings *pProjectSettings);

    GuiModel* _pGuiModel;
    SettingsModel* _pSettingsModel;
    GraphDataModel* _pGraphDataModel;

};

#endif // PROJECTFILEHANDLER_H
