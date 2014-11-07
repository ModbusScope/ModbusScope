#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QButtonGroup>

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
    void registerRemove(quint16 registerAddress);
    void dataExport(QString dataFile);

private slots:
    void startScope();
    void stopScope();
    void addRegister();
    void removeRegister();
    void exitApplication();
    void addRemoveRegisterFromScopeList(QListWidgetItem * item);
    void prepareDataExport();
    void loadProjectSettings();
    void prepareImageExport();

public slots:
    void updateStats(quint32 successCount, quint32 errorCount);
    void changeXAxisScaling(int id);

private:

    void setSettingsObjectsState(bool bState);
    void updateBoxes(ProjectFileParser::ProjectSettings *pProjectSettings);
    void loadProjectFile(QString dataFilePath);

    Ui::MainWindow * _ui;
    ScopeData * _scope;
    ScopeGui * _gui;
    ModbusSettings _commSettings;

    QLabel * _statusStats;
    QLabel * _statusState;
    QButtonGroup * _xAxisScaleGroup;

    static const QString _cStateRunning;
    static const QString _cStateStopped;
    static const QString _cStatsTemplate;
};

#endif // MAINWINDOW_H
