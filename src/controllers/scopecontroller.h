#ifndef SCOPECONTROLLER_H
#define SCOPECONTROLLER_H

#include "datahandling/graphdatahandler.h"
#include "util/result.h"

#include <QObject>

// Forward declaration
class GuiModel;
class SettingsModel;
class GraphDataModel;
class CommunicationStatsModel;
class NoteModel;
class DataParserModel;
class AdapterPoll;
class AdapterHub;
class CommunicationStats;
class DataFileHandler;
class ProjectFileHandler;
class ExpressionStatus;

//! Session/data-handling controller: the UI/CLI seam for scope logging (file-open error reporting
//! is not yet fully headless — see DataFileHandler/ProjectFileHandler, which still use QMessageBox).
class ScopeController : public QObject
{
    Q_OBJECT

public:
    explicit ScopeController(GuiModel* pGuiModel,
                             SettingsModel* pSettingsModel,
                             GraphDataModel* pGraphDataModel,
                             CommunicationStatsModel* pCommunicationStatsModel,
                             NoteModel* pNoteModel,
                             DataParserModel* pDataParserModel,
                             QObject* parent = nullptr);
    ~ScopeController();

    void openFile(QString filename);
    void initAdapter();
    bool isPolling() const;

    AdapterHub* adapterHub() const;
    DataFileHandler* dataFileHandler() const;
    ProjectFileHandler* projectFileHandler() const;
    CommunicationStats* communicationStats() const;

public slots:
    void start();
    void stop();
    void clear();

signals:
    void dataProcessed(const ResultDoubleList& results);
    void dataCleared();
    void mbcImportRequested();
    void errorOccurred(QString message);

private slots:
    void onRegisterDataReady(const ResultDoubleList& results);
    void updateDataFileNotes();

private:
    GuiModel* _pGuiModel;
    SettingsModel* _pSettingsModel;
    GraphDataModel* _pGraphDataModel;
    CommunicationStatsModel* _pCommunicationStatsModel;
    NoteModel* _pNoteModel;
    DataParserModel* _pDataParserModel;

    AdapterPoll* _pAdapterPoll;
    GraphDataHandler _graphDataHandler;
    CommunicationStats* _pCommunicationStats;
    DataFileHandler* _pDataFileHandler;
    ProjectFileHandler* _pProjectFileHandler;
    ExpressionStatus* _pExpressionStatus;
};

#endif // SCOPECONTROLLER_H
