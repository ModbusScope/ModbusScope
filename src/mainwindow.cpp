#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qcustomplot.h"
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

    _modelReg.insertRow(_modelReg.rowCount());
    QModelIndex index = _modelReg.index(_modelReg.rowCount() - 1);
    _modelReg.setData(index, "40001");

    _ui->listReg->setModel(&_modelReg);
    _ui->listReg->show();

    qRegisterMetaType<ModbusSettings>("ModbusSettings");

    connect(_ui->chkXAxisAutoScale, SIGNAL(stateChanged(int)), _gui, SLOT(setXAxisAutoScale(int)));
    connect(_ui->chkYAxisAutoScale, SIGNAL(stateChanged(int)), _gui, SLOT(setYAxisAutoScale(int)));

    connect(_scope, SIGNAL(propagateNewData(bool, QList<quint16>)), _gui, SLOT(plotResults(bool, QList<quint16>)));

    // Setup handler for buttons
    connect(_ui->btnAdd, SIGNAL(released()), this, SLOT(addRegister()));
    connect(_ui->btnRemove, SIGNAL(released()), this, SLOT(removeRegister()));

    // Handler for menu bar
    connect(_ui->actionStart, SIGNAL(triggered()), this, SLOT(startScope()));
    connect(_ui->actionStop, SIGNAL(triggered()), this, SLOT(stopScope()));
    connect(_ui->actionExit, SIGNAL(triggered()), this, SLOT(exitApplication()));

}

MainWindow::~MainWindow()
{
    delete _scope;
    delete _ui;
}

void MainWindow::startScope()
{
    QList<quint16> registerList;

    _commSettings.setIpAddress(_ui->lineIP->text());
    _commSettings.setPort(_ui->spinPort->text().toInt());
    _commSettings.setSlaveId(_ui->spinSlaveId->text().toInt());

    registerList.clear();
    for(qint32 i = 0; i < _modelReg.rowCount(); i++)
    {
        registerList.append(_modelReg.data(_modelReg.index(i), Qt::DisplayRole).toInt());
    }

    _ui->actionStart->setEnabled(false);
    _ui->actionStop->setEnabled(true);

    if (_scope->startCommunication(&_commSettings, &registerList))
    {
        _gui->resetGraph(registerList.size());
    }
}

void MainWindow::stopScope()
{
    _scope->stopCommunication();
    _ui->actionStart->setEnabled(true);
    _ui->actionStop->setEnabled(false);
}

void MainWindow::exitApplication()
{
    QApplication::quit();
}

void MainWindow::addRegister()
{
    _modelReg.insertRow(_modelReg.rowCount());
    QModelIndex index = _modelReg.index(_modelReg.rowCount() - 1);
    _modelReg.setData(index, _ui->spinReg->text());
}


void MainWindow::removeRegister()
{
    _modelReg.removeRows(_ui->listReg->currentIndex().row(), 1);
}
