#ifndef TST_GUISTATECONTROLLER_H
#define TST_GUISTATECONTROLLER_H

#include "dialogs/guistatecontroller.h"

#include <QObject>

class GuiModel;
class SettingsModel;
class DataParserModel;
class QAction;

class TestGuiStateController : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void initState();
    void startedState();
    void stoppedStateNoProject();
    void stoppedStateWithProject();
    void dataLoadedState();
    void projectFileLoadedTogglesActions();
    void initStateDisablesProjectActions();
    void mbcCompatibilityRespected();

private:
    GuiModel* _pGuiModel = nullptr;
    SettingsModel* _pSettingsModel = nullptr;
    DataParserModel* _pDataParserModel = nullptr;

    QAction* _pStart = nullptr;
    QAction* _pStop = nullptr;
    QAction* _pSettings = nullptr;
    QAction* _pRegisterSettings = nullptr;
    QAction* _pOpenDataFile = nullptr;
    QAction* _pImportFromMbcFile = nullptr;
    QAction* _pOpenProjectFile = nullptr;
    QAction* _pSaveDataFile = nullptr;
    QAction* _pExportImage = nullptr;
    QAction* _pSaveProjectFileAs = nullptr;
    QAction* _pSaveProjectFile = nullptr;
    QAction* _pReloadProjectFile = nullptr;
    QAction* _pClearData = nullptr;

    GuiStateController* _pController = nullptr;
};

#endif // TST_GUISTATECONTROLLER_H
