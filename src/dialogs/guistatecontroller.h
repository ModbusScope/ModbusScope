#ifndef GUISTATECONTROLLER_H
#define GUISTATECONTROLLER_H

#include <QObject>

class QAction;
class GuiModel;
class SettingsModel;
class DataParserModel;

//! Actions whose enabled state depends on the current GuiState/project state
struct GuiStateActions
{
    QAction* pStart = nullptr;
    QAction* pStop = nullptr;
    QAction* pSettings = nullptr;
    QAction* pRegisterSettings = nullptr;
    QAction* pOpenDataFile = nullptr;
    QAction* pImportFromMbcFile = nullptr;
    QAction* pOpenProjectFile = nullptr;
    QAction* pSaveDataFile = nullptr;
    QAction* pExportImage = nullptr;
    QAction* pSaveProjectFileAs = nullptr;
    QAction* pSaveProjectFile = nullptr;
    QAction* pReloadProjectFile = nullptr;
    QAction* pClearData = nullptr;
};

class GuiStateController : public QObject
{
    Q_OBJECT

public:
    explicit GuiStateController(GuiModel* pGuiModel,
                                SettingsModel* pSettingsModel,
                                DataParserModel* pDataParserModel,
                                const GuiStateActions& actions,
                                QObject* parent = nullptr);

private slots:
    void updateGuiState();
    void projectFileLoaded();

private:
    GuiModel* _pGuiModel;
    SettingsModel* _pSettingsModel;
    DataParserModel* _pDataParserModel;
    GuiStateActions _actions;
};

#endif // GUISTATECONTROLLER_H
