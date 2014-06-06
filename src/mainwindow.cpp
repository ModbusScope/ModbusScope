#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include "modbusscope.h"
#include "QDebug"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _scope(NULL)
{
    _ui->setupUi(this);

    _modbusSettings.SetIpAddress("127.0.0.1");
    _modbusSettings.SetPort(1502);

    _scope = new ModbusScope(_ui->customPlot);

    qRegisterMetaType<ModbusSettings>("ModbusSettings");

    connect(_ui->btnStartModbus, SIGNAL(released()), this, SLOT(StartScope()));
    connect(_ui->btnStopModbus, SIGNAL(released()), this, SLOT(StopScope()));

}

MainWindow::~MainWindow()
{
    delete _scope;
    delete _ui;
}

void MainWindow::StartScope()
{
    SettingsDialog dialog;
    dialog.setModal(true);
    dialog.exec();
    if (dialog.result() == QDialog::Accepted)
    {
        dialog.GetModbusSettings(&_modbusSettings);
        dialog.GetRegisterList(&_registerList);

        _scope->StartCommunication(&_modbusSettings, &_registerList);
    }
}

void MainWindow::StopScope()
{
    _scope->StopCommunication();
}
