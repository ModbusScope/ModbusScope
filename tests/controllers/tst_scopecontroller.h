#ifndef TST_SCOPECONTROLLER_H
#define TST_SCOPECONTROLLER_H

#include <QObject>

class GuiModel;
class SettingsModel;
class GraphDataModel;
class CommunicationStatsModel;
class NoteModel;
class DataParserModel;
class ScopeController;

class TestScopeController : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void startWithoutRegistersEmitsError();
    void startFromDataLoadedResetsSession();
    void clearEmitsDataClearedAndResets();
    void openFileSetsLastDir();
    void openMbcIncompatibleEmitsError();
    void stopSetsStoppedState();

private:
    GuiModel* _pGuiModel = nullptr;
    SettingsModel* _pSettingsModel = nullptr;
    GraphDataModel* _pGraphDataModel = nullptr;
    CommunicationStatsModel* _pCommunicationStatsModel = nullptr;
    NoteModel* _pNoteModel = nullptr;
    DataParserModel* _pDataParserModel = nullptr;

    ScopeController* _pController = nullptr;
};

#endif // TST_SCOPECONTROLLER_H
