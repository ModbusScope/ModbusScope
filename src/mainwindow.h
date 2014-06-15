#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "modbussettings.h"

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

    void StopThread();
    void StartModbus();
    void StopModbus();

private slots:
    void StartScope();
    void StopScope();

public slots:

private:
    Ui::MainWindow * _ui;
    ScopeData * _scope;
    ScopeGui * _gui;
    ModbusSettings _modbusSettings;
    QList<quint16> _registerList;
};

#endif // MAINWINDOW_H
