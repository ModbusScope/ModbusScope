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

    void stopThread();

    void startModbus();
    void stopModbus();

private slots:
    void selectionChanged();
    void mousePress();
    void mouseWheel();

    void startScope(void);
    void stopScope(void);

public slots:
    void plotResults(QList<u_int16_t> values);

private:
    Ui::MainWindow *ui;
    ModbusScope * _scope;
    ModbusSettings _modbusSettings;
    QList<quint16> _registerList;
};

#endif // MAINWINDOW_H
