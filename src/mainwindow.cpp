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

    _ui->listReg->addItem("40001");

    _ui->listReg->setEditTriggers(QAbstractItemView::NoEditTriggers); // non-editable

    qRegisterMetaType<ModbusSettings>("ModbusSettings");

    connect(_ui->chkXAxisAutoScale, SIGNAL(stateChanged(int)), _gui, SLOT(setXAxisAutoScale(int)));
    connect(_ui->chkYAxisAutoScale, SIGNAL(stateChanged(int)), _gui, SLOT(setYAxisAutoScale(int)));

    connect(_scope, SIGNAL(propagateNewData(bool, QList<quint16>)), _gui, SLOT(plotResults(bool, QList<quint16>)));

    connect(_ui->listReg, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(addRemoveRegisterFromScopeList(QListWidgetItem *)));
    connect(this, SIGNAL(registerStateChange(quint16)), _scope, SLOT(toggleRegister(quint16)));
    connect(this, SIGNAL(registerRemove(quint16)), _scope, SLOT(removedRegister(quint16)));
    connect(this, SIGNAL(dataExport(QString)), _gui, SLOT(exportDataCsv(QString)));

    // Setup handler for buttons
    connect(_ui->btnAdd, SIGNAL(released()), this, SLOT(addRegister()));
    connect(_ui->btnRemove, SIGNAL(released()), this, SLOT(removeRegister()));

    // Handler for menu bar
    connect(_ui->actionStart, SIGNAL(triggered()), this, SLOT(startScope()));
    connect(_ui->actionStop, SIGNAL(triggered()), this, SLOT(stopScope()));
    connect(_ui->actionExit, SIGNAL(triggered()), this, SLOT(exitApplication()));
    connect(_ui->actionExportDataCsv, SIGNAL(triggered()), this, SLOT(prepareDataExport()));
}

MainWindow::~MainWindow()
{
    delete _scope;
    delete _ui;
}

void MainWindow::startScope()
{
    bool bCommunicationSettingsValid = true;

    const quint32 pollTime = _ui->spinPollTime->text().toInt();

    if (bCommunicationSettingsValid)
    {
        if ((pollTime < 100) || (pollTime > 90000))
        {
            QMessageBox::warning(this, "Poll time not valid!", "The poll time is invalid. Make sure the poll time is longer than 100 ms and shorter than 90000 ms.");
            bCommunicationSettingsValid = false;
        }
    }
    /* TODO: check other fields */


    if (bCommunicationSettingsValid)
    {
        if (_scope->getRegisterCount() != 0)
        {
            _commSettings.setIpAddress(_ui->lineIP->text());
            _commSettings.setPort(_ui->spinPort->text().toInt());
            _commSettings.setSlaveId(_ui->spinSlaveId->text().toInt());
            _commSettings.setPollTime(pollTime);

            _ui->actionStart->setEnabled(false);
            _ui->actionExportDataCsv->setEnabled(false);

            _ui->actionStop->setEnabled(true);

            setSettingsObjectsState(false);

            if (_scope->startCommunication(&_commSettings))
            {
                _gui->resetGraph();

                QList<quint16> regList;
                _scope->getRegisterList(&regList);

                for (quint32 i = 0; i < (quint32)regList.size(); i++)
                {
                    _gui->addGraph(regList[i]);
                }
            }
        }
        else
        {
            QMessageBox::warning(this, "No register in scope list!", "There are no register in the scope list. Please select at least one register.");
        }
    }

}

void MainWindow::stopScope()
{
    _scope->stopCommunication();
    _ui->actionStart->setEnabled(true);
    _ui->actionExportDataCsv->setEnabled(true);

    _ui->actionStop->setEnabled(false);

    setSettingsObjectsState(true);
}

void MainWindow::exitApplication()
{
    QApplication::quit();
}


void MainWindow::addRemoveRegisterFromScopeList(QListWidgetItem * item)
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

void MainWindow::prepareDataExport()
{
    QString filePath;
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setOption(QFileDialog::HideNameFilterDetails, false);
    dialog.setDefaultSuffix("csv");
    dialog.setWindowTitle(tr("Select csv file"));
    dialog.setNameFilter(tr("CSV files (*.csv)"));

    if (dialog.exec())
    {
        filePath = dialog.selectedFiles().first();
        emit dataExport(filePath);
    }
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
    if (_ui->listReg->selectedItems().count() != 0)
    {
        emit registerRemove((quint16)_ui->listReg->selectedItems()[0]->text().toInt());
        qDeleteAll(_ui->listReg->selectedItems());
    }
}

void MainWindow::setSettingsObjectsState(bool bState)
{
    _ui->spinPort->setEnabled(bState);
    _ui->btnAdd->setEnabled(bState);
    _ui->spinPollTime->setEnabled(bState);
    _ui->btnRemove->setEnabled(bState);
    _ui->spinReg->setEnabled(bState);
    _ui->spinSlaveId->setEnabled(bState);
    _ui->lineIP->setEnabled(bState);
    _ui->listReg->setEnabled(bState);
}
