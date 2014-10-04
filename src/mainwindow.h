#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "modbussettings.h"
#include <QStringListModel>

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

private slots:
    void startScope();
    void stopScope();
    void addRegister();
    void removeRegister();
    void exitApplication();

public slots:

private:
    Ui::MainWindow * _ui;
    ScopeData * _scope;
    ScopeGui * _gui;
    ModbusSettings _commSettings;

    QStringListModel _modelReg;
};

#endif // MAINWINDOW_H
