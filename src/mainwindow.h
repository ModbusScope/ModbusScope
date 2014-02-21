#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ModbusThread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

protected:
     void closeEvent(QCloseEvent *event);

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void closing();

    void stopThread();

    void startModbus();
    void stopModbus();

private slots:
    void selectionChanged();
    void mousePress();
    void mouseWheel();

public slots:
    void workerStopped();
    void errorString(QString error);
    void plotResults(unsigned short result0, unsigned short result1);

private:
    Ui::MainWindow *ui;
    ModbusThread* worker;
};

#endif // MAINWINDOW_H
