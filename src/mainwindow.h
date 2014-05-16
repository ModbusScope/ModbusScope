#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ModbusThread.h"
#include "modbussettings.h"

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

    void startScope(void);
    void stopScope(void);

public slots:
    void workerStopped();
    void errorString(QString error);
    void plotResults(QList<u_int16_t> values);

private:
    Ui::MainWindow *ui;
    ModbusThread* worker;
    ModbusSettings modbusSettings;
};

#endif // MAINWINDOW_H
