#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QButtonGroup>
#include <QTimer>
#include <QLabel>
#include <QMenu>

#include "updatenotify.h"

namespace Ui {
class MainWindow;
}

// Forward declaration
class ModbusPoll;
class GraphDataHandler;
class QCustomPlot;
class GraphDataModel;
class NoteModel;
class DiagnosticModel;
class ConnectionDialog;
class SettingsModel;
class DataParserModel;
class LogDialog;
class DiagnosticDialog;
class NotesDock;
class GuiModel;
class GraphView;
class MarkerInfo;
class DataFileHandler;
class ProjectFileHandler;
class Legend;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QStringList cmdArguments, GuiModel *pGuiModel, SettingsModel *pSettingsModel,
                        GraphDataModel *pGraphDataModel, NoteModel *pNoteModel,
                        DiagnosticModel *pDiagnosticModel,
                        DataParserModel *pDataParserModel, QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void closeEvent(QCloseEvent* event);

private slots:

    /* Menu handlers */
    void exitApplication();
    void selectImageExportFile();
    void showAbout();
    void openOnlineDoc();
    void openUpdateUrl();
    void menuBringToFrontGraphClicked(bool bState);
    void menuShowHideGraphClicked(bool bState);
    void showConnectionDialog();
    void showLogSettingsDialog();
    void handleShowRegisterDialog(bool checked);
    void addNoteToGraph();
    void toggleZoom(bool checked);
    void clearData();
    void startScope();
    void stopScope();
    void showDiagnostic();
    void showNotesDialog();
    void toggleMarkersState();
    void handleYMinChange();
    void handleY2MinChange();
    void handleYMaxChange();
    void handleY2MaxChange();

    /* Model change handlers */
    void handleGraphVisibilityChange(quint32 graphIdx);
    void handleGraphColorChange(const quint32 graphIdx);
    void handleGraphLabelChange(const quint32 graphIdx);

    void updateBringToFrontGrapMenu();
    void updateHighlightSampleMenu();
    void handleZoomStateChanged();
    void rebuildGraphMenu();
    void updateWindowTitle();
    void updatexAxisSlidingMode();
    void updatexAxisSlidingInterval();
    void updateyAxisSlidingMode();
    void updatey2AxisSlidingMode();
    void updateyAxisMinMax();
    void updatey2AxisMinMax();
    void projectFileLoaded();
    void updateGuiState();
    void updateStats();
    void updateMarkerDockVisibility();

    /* Misc */
    void scaleWidgetUndocked(bool bFloat);
    void legendWidgetUndocked(bool bFloat);
    void showContextMenu(const QPoint& pos);
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void appFocusChanged(QWidget *old, QWidget *now);
    void xAxisScaleGroupClicked(int id);
    void yAxisScaleGroupClicked(int id);
    void y2AxisScaleGroupClicked(int id);
    void updateRuntime();
    void updateCommunicationStats(QList<bool> successList);
    void updateDataFileNotes();

    void showVersionUpdate(UpdateNotify::UpdateState result);

private:
    void setAxisToAuto();
    void showRegisterDialog(QString mbcFile);
    void handleCommandLineArguments(QStringList cmdArguments);

    Ui::MainWindow * _pUi;
    ModbusPoll * _pModbusPoll;
    GraphView * _pGraphView;

    GuiModel * _pGuiModel;
    SettingsModel * _pSettingsModel;
    GraphDataModel * _pGraphDataModel;
    NoteModel * _pNoteModel;
    DiagnosticModel * _pDiagnosticModel;
    DataParserModel * _pDataParserModel;

    UpdateNotify* _pUpdateNotify;
    GraphDataHandler* _pGraphDataHandler;

    ConnectionDialog * _pConnectionDialog;
    LogDialog * _pLogDialog;
    DiagnosticDialog * _pDiagnosticDialog;

    DataFileHandler* _pDataFileHandler;
    ProjectFileHandler* _pProjectFileHandler;

    NotesDock * _pNotesDock;
    MarkerInfo * _pMarkerInfo;
    Legend * _pLegend;

    QLabel * _pStatusStats;
    QLabel * _pStatusState;
    QLabel * _pStatusRuntime;
    QButtonGroup * _pXAxisScaleGroup;
    QButtonGroup * _pYAxisScaleGroup;
    QButtonGroup * _pY2AxisScaleGroup;

    QTimer _runtimeTimer;

    QMenu _menuRightClick;

    QMenu * _pGraphBringToFront;
    QMenu * _pGraphShowHide;
    QActionGroup * _pBringToFrontGroup;

    QPointF _lastRightClickPos;

    static const QString _cStateRunning;
    static const QString _cStateStopped;
    static const QString _cStatsTemplate;
    static const QString _cStateDataLoaded;
    static const QString _cRuntime;
};

#endif // MAINWINDOW_H
