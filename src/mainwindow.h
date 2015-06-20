#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QButtonGroup>
#include <QTimer>
#include <QMenu>

#include "datafileparser.h"
#include "projectfileparser.h"

namespace Ui {
class MainWindow;
}

// Forward declaration
class ScopeData;
class QCustomPlot;
class RegisterModel;
class RegisterDialog;
class ConnectionModel;
class ConnectionDialog;
class GuiModel;
class ExtendedGraphView;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QStringList cmdArguments, QWidget *parent = 0);
    ~MainWindow();

signals:

    void stopThread();
    void startModbus();
    void stopModbus();
    void registerStateChange(quint16 registerAddress);
    void dataExport(QString dataFile);

public slots:
    void updateStats(quint32 successCount, quint32 errorCount);
    void changeXAxisScaling(int id);
    void changeYAxisScaling(int id);

private slots:

    /* Menu handlers */
    void loadProjectSettings();
    void reloadProjectSettings();
    void importData();
    void exitApplication();
    void prepareDataExport();
    void prepareImageExport();
    void showAbout();
    void menuBringToFrontGraphClicked(bool bState);
    void menuShowHideGraphClicked(bool bState);
    void showConnectionDialog();
    void showRegisterDialog();
    void changeLegendPosition(QAction* pAction);
    void clearData();
    void startScope();
    void stopScope();

    /* Model change handlers */
    void showHideGraph(const quint32 index);
    void updateBringToFrontGrapMenu();
    void updateHighlightSampleMenu();
    void updateValueTooltipMenu();
    void clearGraphMenu();
    void addGraphMenu();
    void updateWindowTitle();


    /* Misc */
    void showContextMenu(const QPoint& pos);

    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);


    void updateYMin(int newMin);
    void updateYMax(int newMax);

    void updateRuntime();


private:

    void setSettingsObjectsState(bool bState);
    void updateConnectionSetting(ProjectFileParser::ProjectSettings *pProjectSettings);
    void loadProjectFile(QString dataFilePath);
    void loadDataFile(QString dataFilePath);
    void parseDataFile(DataFileParser::FileData * pData);

    Ui::MainWindow * _pUi;
    ScopeData * _pScope;
    ExtendedGraphView * _pGraphView;

    ConnectionModel * _pConnectionModel;
    ConnectionDialog * _pConnectionDialog;

    RegisterModel * _pRegisterModel;
    RegisterDialog * _pRegisterDialog;

    GuiModel * _pGuiModel;

    QLabel * _pStatusStats;
    QLabel * _pStatusState;
    QLabel * _pStatusRuntime;
    QButtonGroup * _pXAxisScaleGroup;
    QButtonGroup * _pYAxisScaleGroup;

    QTimer _runtimeTimer;

    QMenu * _pGraphBringToFront;
    QMenu * _pGraphShowHide;
    QActionGroup * _pBringToFrontGroup;
    QActionGroup * _pLegendPositionGroup;

    static const QString _cStateRunning;
    static const QString _cStateStopped;
    static const QString _cStatsTemplate;
    static const QString _cRuntime;
};

#endif // MAINWINDOW_H
