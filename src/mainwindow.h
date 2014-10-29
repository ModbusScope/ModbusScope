#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "modbussettings.h"
#include <QListWidgetItem>

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
    explicit MainWindow(QWidget *parent = 0);
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

public slots:

private:

    void setSettingsObjectsState(bool bState);

    Ui::MainWindow * _ui;
    ScopeData * _scope;
    ScopeGui * _gui;
    ModbusSettings _commSettings;
};

#endif // MAINWINDOW_H
