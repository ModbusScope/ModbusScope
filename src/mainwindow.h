#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "modbussettings.h"

namespace Ui {
class MainWindow;
}

class ModbusScope;

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
    ModbusScope * _scope;
    ModbusSettings _modbusSettings;
    QList<quint16> _registerList;
};

#endif // MAINWINDOW_H
