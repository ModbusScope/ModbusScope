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

    // Set window title
    QString windowTitle;
    windowTitle.append("ModbusScope (V");
    windowTitle.append(APP_VERSION);
    windowTitle.append(")");
    this->setWindowTitle(windowTitle);

    _scope = new ScopeData();
    _gui = new ScopeGui(_ui->customPlot, this);

    qRegisterMetaType<ModbusSettings>("ModbusSettings");

    connect(_ui->btnStartModbus, SIGNAL(released()), this, SLOT(startScope()));
    connect(_ui->btnStopModbus, SIGNAL(released()), this, SLOT(stopScope()));

    connect(_ui->chkXAxisAutoScale, SIGNAL(stateChanged(int)), _gui, SLOT(setXAxisAutoScale(int)));
    connect(_ui->chkYAxisAutoScale, SIGNAL(stateChanged(int)), _gui, SLOT(setYAxisAutoScale(int)));

    connect(_scope, SIGNAL(propagateNewData(bool, QList<quint16>)), _gui, SLOT(plotResults(bool, QList<quint16>)));

}

MainWindow::~MainWindow()
{
    delete _scope;
    delete _ui;
}

void MainWindow::startScope()
{
    SettingsDialog dialog;
    dialog.setModal(true);
    dialog.exec();
    if (dialog.result() == QDialog::Accepted)
    {
        dialog.getModbusSettings(&_modbusSettings);
        dialog.getRegisterList(&_registerList);

        _ui->btnStartModbus->setEnabled(false);
        _ui->btnStopModbus->setEnabled(true);

        if (_scope->startCommunication(&_modbusSettings, &_registerList))
        {
            _gui->resetGraph(_registerList.size());
        }
    }
}

void MainWindow::stopScope()
{
    _scope->stopCommunication();
    _ui->btnStartModbus->setEnabled(true);
    _ui->btnStopModbus->setEnabled(false);

}
