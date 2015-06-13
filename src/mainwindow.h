#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QButtonGroup>
#include <QTimer>
#include <QMenu>

#include "registermodel.h"
#include "registerdialog.h"
#include "connectiondialog.h"
#include "connectionmodel.h"
#include "projectfileparser.h"

namespace Ui {
class MainWindow;
}

// Forward declaration
class ScopeData;
class QCustomPlot;
class ScopeGui;

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

private slots:
    void startScope();
    void stopScope();
    void exitApplication();
    void prepareDataExport();
    void loadProjectSettings();
    void reloadProjectSettings();
    void prepareImageExport();
    void updateYMin(int newMin);
    void updateYMax(int newMax);
    void showAbout();
    void importData();
    void updateRuntime();
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void bringToFrontGraph(bool bState);
    void showHideGraph(bool bState);
    void showContextMenu(const QPoint& pos);
    void clearData();
    void showConnectionDialog();
    void showRegisterDialog();
    void changeLegendPosition(QAction* pAction);

public slots:
    void updateStats(quint32 successCount, quint32 errorCount);
    void changeXAxisScaling(int id);
    void changeYAxisScaling(int id);

private:

    void setSettingsObjectsState(bool bState);
    void updateConnectionSetting(ProjectFileParser::ProjectSettings *pProjectSettings);
    void loadProjectFile(QString dataFilePath);
    void loadDataFile(QString dataFilePath);

    Ui::MainWindow * _pUi;
    ScopeData * _pScope;
    ScopeGui * _pGui;

    ConnectionModel * _pConnectionModel;
    ConnectionDialog * _pConnectionDialog;

    RegisterModel * _pRegisterModel;
    RegisterDialog * _pRegisterDialog;

    QLabel * _pStatusStats;
    QLabel * _pStatusState;
    QLabel * _pStatusRuntime;
    QButtonGroup * _pXAxisScaleGroup;
    QButtonGroup * _pYAxisScaleGroup;
    QString _projectFilePath;
    QString _lastDataFilePath;

    QTimer _runtimeTimer;

    QMenu * _pGraphBringToFront;
    QMenu * _pGraphShowHide;
    QActionGroup* _pBringToFrontGroup;
    QActionGroup* _pLegendPositionGroup;

    static const QString _cWindowTitle;
    static const QString _cStateRunning;
    static const QString _cStateStopped;
    static const QString _cStatsTemplate;
    static const QString _cRuntime;
};

#endif // MAINWINDOW_H
