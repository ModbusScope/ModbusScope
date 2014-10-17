#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qcustomplot.h"
#include "scopedata.h"
#include "scopegui.h"

#include "QDebug"
#include "QAbstractItemView"


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

    _ui->listReg->addItem("40001");

    _ui->listReg->setEditTriggers(QAbstractItemView::NoEditTriggers); // non-editable

    qRegisterMetaType<ModbusSettings>("ModbusSettings");

    connect(_ui->chkXAxisAutoScale, SIGNAL(stateChanged(int)), _gui, SLOT(setXAxisAutoScale(int)));
    connect(_ui->chkYAxisAutoScale, SIGNAL(stateChanged(int)), _gui, SLOT(setYAxisAutoScale(int)));

    connect(_scope, SIGNAL(propagateNewData(bool, QList<quint16>)), _gui, SLOT(plotResults(bool, QList<quint16>)));

    connect(_ui->listReg, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(doubleClickedRegister(QListWidgetItem *)));
    connect(this, SIGNAL(registerStateChange(quint16)), _scope, SLOT(toggleRegister(quint16)));
    connect(this, SIGNAL(registerRemove(quint16)), _scope, SLOT(removedRegister(quint16)));

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
    _commSettings.setIpAddress(_ui->lineIP->text());
    _commSettings.setPort(_ui->spinPort->text().toInt());
    _commSettings.setSlaveId(_ui->spinSlaveId->text().toInt());

    _ui->actionStart->setEnabled(false);
    _ui->actionStop->setEnabled(true);

    if (_scope->startCommunication(&_commSettings))
    {
        _gui->resetGraph(_scope->getRegisterCount());
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

void MainWindow::doubleClickedRegister(QListWidgetItem * item)
{
    // Set background color
    QBrush selectedColor;
    selectedColor.setColor(QColor("green"));
    selectedColor.setStyle(Qt::SolidPattern);

    if (item->background() != selectedColor)
    {
         item->setBackground(selectedColor);
    }
    else
    {
        item->setBackground(QBrush(QColor("white"), Qt::SolidPattern));
    }

    emit registerStateChange((quint16)item->text().toInt());
}

void MainWindow::addRegister()
{
    bool bFound = false;
    QString addr = _ui->spinReg->text();

    for (qint32 i = 0; i < (qint32)_ui->listReg->count(); i++)
    {
        if (_ui->listReg->item(i)->text() == addr)
        {
            bFound = true;
            break;
        }
    }

    if (bFound)
    {
        QMessageBox::warning(this, "Duplicate register!", "The register is already present in the list.");
    }
    else
    {
        _ui->listReg->addItem(addr);
    }
}


void MainWindow::removeRegister()
{
    if (_ui->listReg->count() != 0)
    {
        _ui->listReg->removeItemWidget(_ui->listReg->selectedItems()[0]);
        emit registerRemove((quint16)_ui->listReg->selectedItems()[0]->text().toInt());
    }
}
