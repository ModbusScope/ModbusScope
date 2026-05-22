#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "datahandling/graphdatahandler.h"
#include "util/graphindex.h"
#include "util/recentfilemodule.h"
#include "util/result.h"
#include "util/updatenotify.h"

#include <QButtonGroup>
#include <QLabel>
#include <QListWidgetItem>
#include <QMainWindow>
#include <QMenu>

namespace Ui {
class MainWindow;
}

// Forward declaration
class AdapterPoll;
class QCustomPlot;
class GraphDataModel;
class NoteModel;
class DiagnosticModel;
class SettingsModel;
class DataParserModel;
class DiagnosticDialog;
class NotesDock;
class GuiModel;
class GraphView;
class MarkerInfo;
class DataFileHandler;
class ProjectFileHandler;
class Legend;
class StatusBar;
class ExpressionStatus;
class MostRecentMenu;
class CommunicationStats;
class CommunicationStatsModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QStringList cmdArguments,
                        GuiModel* pGuiModel,
                        SettingsModel* pSettingsModel,
                        GraphDataModel* pGraphDataModel,
                        CommunicationStatsModel* pCommunicationStatsModel,
                        NoteModel* pNoteModel,
                        DiagnosticModel* pDiagnosticModel,
                        DataParserModel* pDataParserModel,
                        QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* e) override;
    void dropEvent(QDropEvent* e) override;

private slots:

    /* Menu handlers */
    void exitApplication();
    void selectImageExportFile();
    void showAbout();
    void openOnlineDoc();
    void openUpdateUrl();
    void menuShowHideGraphClicked(bool bState);
    void showSettingsDialog();
    void handleShowRegisterDialog(bool checked);
    void addNoteToGraph();
    void toggleZoom(bool checked);
    void clearData();
    void startScope();
    void stopScope();
    void showDiagnostic();
    void showNotesDialog();
    void showMbcImportDialog();
    void toggleMarkersState();
    void handleOpenRecentProject(QString projectFile);

    /* Model change handlers */
    void handleGraphVisibilityChange(GraphIdx graphIdx);
    void handleGraphColorChange(GraphIdx graphIdx);
    void handleGraphLabelChange(GraphIdx graphIdx);

    void updateHighlightSampleMenu();
    void handleZoomStateChanged();
    void rebuildGraphMenu();
    void handleGraphsCountChanged();
    void updateWindowTitle();
    void projectFileLoaded();
    void updateGuiState();
    void updateMarkerDockVisibility();

    /* Misc */
    void scaleWidgetUndocked(bool bFloat);
    void legendWidgetUndocked(bool bFloat);
    void showContextMenu(const QPoint& pos);
    void appFocusChanged(QWidget* old, QWidget* now);
    void updateDataFileNotes();

    void showVersionUpdate(UpdateNotify::UpdateState result);
    void onRegisterDataReady(const ResultDoubleList& results);

private:
    void setAxisToAuto();
    void showRegisterDialog();
    void handleCommandLineArguments(QStringList cmdArguments);
    void handleFileOpen(QString filename);

    Ui::MainWindow* _pUi;
    AdapterPoll* _pAdapterPoll;
    GraphView* _pGraphView;

    GuiModel* _pGuiModel;
    SettingsModel* _pSettingsModel;
    GraphDataModel* _pGraphDataModel;
    CommunicationStatsModel* _pCommunicationStatsModel;
    NoteModel* _pNoteModel;
    DiagnosticModel* _pDiagnosticModel;
    DataParserModel* _pDataParserModel;

    UpdateNotify* _pUpdateNotify;
    GraphDataHandler _graphDataHandler;
    ExpressionStatus* _pExpressionStatus;
    CommunicationStats* _pCommunicationStats;

    DiagnosticDialog* _pDiagnosticDialog;

    DataFileHandler* _pDataFileHandler;
    ProjectFileHandler* _pProjectFileHandler;

    NotesDock* _pNotesDock;
    MarkerInfo* _pMarkerInfo;
    Legend* _pLegend;
    StatusBar* _pStatusBar;

    QLabel* _pEmptyStateLabel;

    QMenu _menuRightClick;
    QMenu* _pGraphShowHide;

    MostRecentMenu* _pMostRecentMenu;
    RecentFileModule _recentFileModule;

    QPointF _lastRightClickPos;
};

#endif // MAINWINDOW_H
