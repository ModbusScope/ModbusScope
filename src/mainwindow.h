#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QButtonGroup>
#include <QTimer>

#include "registermodel.h"
#include "modbussettings.h"
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
    void addRegister();
    void removeRegister();
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

public slots:
    void updateStats(quint32 successCount, quint32 errorCount);
    void changeXAxisScaling(int id);
    void changeYAxisScaling(int id);

private:

    static bool sortRegistersLastFirst(const QModelIndex &s1, const QModelIndex &s2);

    void setSettingsObjectsState(bool bState);
    void updateBoxes(ProjectFileParser::ProjectSettings *pProjectSettings);
    void loadProjectFile(QString dataFilePath);
    void loadDataFile(QString dataFilePath);

    Ui::MainWindow * _pUi;
    ScopeData * _pScope;
    ScopeGui * _pGui;
    ModbusSettings _commSettings;
    RegisterModel * _pRegisterModel;

    QLabel * _pStatusStats;
    QLabel * _pStatusState;
    QLabel * _pStatusRuntime;
    QButtonGroup * _pXAxisScaleGroup;
    QButtonGroup * _pYAxisScaleGroup;
    QString _projectFilePath;
    QString _lastDataFilePath;

    QTimer _runtimeTimer;

    static const QString _cWindowTitle;
    static const QString _cStateRunning;
    static const QString _cStateStopped;
    static const QString _cStatsTemplate;
    static const QString _cRuntime;
};

#endif // MAINWINDOW_H
