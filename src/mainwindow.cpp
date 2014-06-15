#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qcustomplot.h"
#include "settingsdialog.h"
#include "scopedata.h"
#include "scopegui.h"
#include "QDebug"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _scope(NULL),
    _gui(NULL)
{
    _ui->setupUi(this);

    _scope = new ScopeData();
    _gui = new ScopeGui(_ui->customPlot, this);

    qRegisterMetaType<ModbusSettings>("ModbusSettings");

    connect(_ui->btnStartModbus, SIGNAL(released()), this, SLOT(StartScope()));
    connect(_ui->btnStopModbus, SIGNAL(released()), this, SLOT(StopScope()));

    connect(_ui->chkAutoScale, SIGNAL(stateChanged(int)), _gui, SLOT(SetAutoScale(int)));

    connect(_scope, SIGNAL(PropagateNewData(bool, QList<quint16>)), _gui, SLOT(PlotResults(bool, QList<quint16>)));



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

        if (_scope->StartCommunication(&_modbusSettings, &_registerList))
        {
            _gui->ResetGraph(_registerList.size());
        }
    }
}

void MainWindow::StopScope()
{
    _scope->StopCommunication();
}
