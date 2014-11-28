#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qcustomplot.h"
#include "scopedata.h"
#include "scopegui.h"

#include "QDebug"


const QString MainWindow::_cStateRunning = QString("Running");
const QString MainWindow::_cStateStopped = QString("Stopped");
const QString MainWindow::_cStatsTemplate = QString("Success: %1\tErrors: %2");

MainWindow::MainWindow(QStringList cmdArguments, QWidget *parent) :
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

    // Add multipart status bar
    _statusState = new QLabel(_cStateStopped, this);
    _statusState->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _statusStats = new QLabel("", this);
    _statusStats->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    _ui->statusBar->addPermanentWidget(_statusState, 0);
    _ui->statusBar->addPermanentWidget(_statusStats, 2);

    // Setup registerView
    _pRegisterModel = new RegisterModel();
    _ui->registerView->setModel(_pRegisterModel);
    _ui->registerView->verticalHeader()->hide();
    _ui->registerView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    _ui->registerView->horizontalHeader()->setStretchLastSection(true);

    // Select using click, shift and control
     _ui->registerView->setSelectionBehavior(QAbstractItemView::SelectRows);
     _ui->registerView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    _scope = new ScopeData();
    _gui = new ScopeGui(this, _ui->customPlot, this);

    connect(_ui->spinSlidingXInterval, SIGNAL(valueChanged(int)), _gui, SLOT(setxAxisSlidingInterval(int)));
    connect(_ui->spinYMin, SIGNAL(valueChanged(int)), this, SLOT(updateYMin(int)));
    connect(_ui->spinYMax, SIGNAL(valueChanged(int)), this, SLOT(updateYMax(int)));

    //valueChanged is only send when done editing...
    _ui->spinSlidingXInterval->setKeyboardTracking(false);
    _ui->spinYMin->setKeyboardTracking(false);
    _ui->spinYMax->setKeyboardTracking(false);

    // Create button group for X axis scaling options
    _xAxisScaleGroup = new QButtonGroup();
    _xAxisScaleGroup->setExclusive(true);
    _xAxisScaleGroup->addButton(_ui->radioXFullScale, ScopeGui::SCALE_AUTO);
    _xAxisScaleGroup->addButton(_ui->radioXSliding, ScopeGui::SCALE_SLIDING);
    _xAxisScaleGroup->addButton(_ui->radioXManual, ScopeGui::SCALE_MANUAL);
    connect(_xAxisScaleGroup, SIGNAL(buttonClicked(int)), this, SLOT(changeXAxisScaling(int)));

    // Default to full auto scaling
    changeXAxisScaling(ScopeGui::SCALE_AUTO);


    // Create button group for Y axis scaling options
    _yAxisScaleGroup = new QButtonGroup();
    _yAxisScaleGroup->setExclusive(true);
    _yAxisScaleGroup->addButton(_ui->radioYFullScale, ScopeGui::SCALE_AUTO);
    _yAxisScaleGroup->addButton(_ui->radioYMinMax, ScopeGui::SCALE_MINMAX);
    _yAxisScaleGroup->addButton(_ui->radioYManual, ScopeGui::SCALE_MANUAL);
    connect(_yAxisScaleGroup, SIGNAL(buttonClicked(int)), this, SLOT(changeYAxisScaling(int)));

    // Default to full auto scaling
    changeYAxisScaling(ScopeGui::SCALE_AUTO);

    connect(_scope, SIGNAL(handleReceivedData(QList<bool>, QList<quint16>)), _gui, SLOT(plotResults(QList<bool>, QList<quint16>)));
    connect(_scope, SIGNAL(triggerStatUpdate(quint32, quint32)), this, SLOT(updateStats(quint32, quint32)));
    connect(this, SIGNAL(dataExport(QString)), _gui, SLOT(exportDataCsv(QString)));

    // Setup handler for buttons
    connect(_ui->btnAdd, SIGNAL(released()), this, SLOT(addRegister()));
    connect(_ui->btnRemove, SIGNAL(released()), this, SLOT(removeRegister()));

    // Handler for menu bar
    connect(_ui->actionStart, SIGNAL(triggered()), this, SLOT(startScope()));
    connect(_ui->actionStop, SIGNAL(triggered()), this, SLOT(stopScope()));
    connect(_ui->actionExit, SIGNAL(triggered()), this, SLOT(exitApplication()));
    connect(_ui->actionExportDataCsv, SIGNAL(triggered()), this, SLOT(prepareDataExport()));
    connect(_ui->actionLoadProjectFile, SIGNAL(triggered()), this, SLOT(loadProjectSettings()));
    connect(_ui->actionExportImage, SIGNAL(triggered()), this, SLOT(prepareImageExport()));
    connect(_ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));

    if (cmdArguments.size() > 1)
    {
        loadProjectFile(cmdArguments[1]);
    }

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
        /* TODO: check fields */
    }

    if (bCommunicationSettingsValid)
    {

        if (_pRegisterModel->checkedRegisterCount() != 0)
        {
            _commSettings.setIpAddress(_ui->lineIP->text());
            _commSettings.setPort(_ui->spinPort->text().toInt());
            _commSettings.setSlaveId(_ui->spinSlaveId->text().toInt());
            _commSettings.setPollTime(pollTime);

            _ui->actionStart->setEnabled(false);
            _ui->actionStop->setEnabled(true);

            setSettingsObjectsState(false);

            _statusState->setText(_cStateRunning);

            QList<quint16> regList;
            _pRegisterModel->getCheckedRegisterList(&regList);

            if (_scope->startCommunication(&_commSettings, regList))
            {
                QList<QString> regTextList;
                _pRegisterModel->getCheckedRegisterTextList(&regTextList);

                _gui->resetGraph();
                _gui->setupGraph(regTextList);
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
    _ui->actionStop->setEnabled(false);

    _statusState->setText(_cStateStopped);

    setSettingsObjectsState(true);
}

void MainWindow::exitApplication()
{
    QApplication::quit();
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

void MainWindow::loadProjectSettings()
{
    QString projectFilePath;
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setOption(QFileDialog::HideNameFilterDetails, false);
    dialog.setWindowTitle(tr("Select mbs file"));
    dialog.setNameFilter(tr("mbs files (*.mbs)"));

    if (dialog.exec())
    {
        projectFilePath = dialog.selectedFiles().first();
        loadProjectFile(projectFilePath);
    }

}

void MainWindow::prepareImageExport()
{
    QString filePath;
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setOption(QFileDialog::HideNameFilterDetails, false);
    dialog.setDefaultSuffix("png");
    dialog.setWindowTitle(tr("Select png file"));
    dialog.setNameFilter(tr("PNG files (*.png)"));

    if (dialog.exec())
    {
        filePath = dialog.selectedFiles().first();
        _gui->exportGraphImage(filePath);
    }
}


void MainWindow::updateYMin(int newMin)
{
    const qint32 min = _gui->getyAxisMin();
    const qint32 max = _gui->getyAxisMax();
    const qint32 diff = max - min;
    qint32 newMax = max;

    if (newMin >= max)
    {
        newMax = newMin + diff;
    }

    _gui->setyAxisMinMax(newMin, newMax);
    _ui->spinYMax->setValue(newMax);
}


void MainWindow::updateYMax(int newMax)
{
    const qint32 min = _gui->getyAxisMin();
    const qint32 max = _gui->getyAxisMax();
    const qint32 diff = max - min;

    qint32 newMin = min;

    if (newMax <= min)
    {
        newMin = newMax - diff;
    }

    _gui->setyAxisMinMax(newMin, newMax);
    _ui->spinYMin->setValue(newMin);
}



void MainWindow::updateStats(quint32 successCount, quint32 errorCount)
{
    // Update statistics
    _statusStats->setText(_cStatsTemplate.arg(successCount).arg(errorCount));
}

void MainWindow::changeXAxisScaling(int id)
{
    if (id == ScopeGui::SCALE_AUTO)
    {
        // Full auto scaling
        _ui->radioXFullScale->setChecked(true);
        _gui->setxAxisScale(ScopeGui::SCALE_AUTO);
    }
    else if (id == ScopeGui::SCALE_SLIDING)
    {
        // Sliding window
        _ui->radioXSliding->setChecked(true);
        _gui->setxAxisScale(ScopeGui::SCALE_SLIDING, _ui->spinSlidingXInterval->text().toUInt());
    }
    else
    {
        // manual
        _ui->radioXManual->setChecked(true);
        _gui->setxAxisScale(ScopeGui::SCALE_MANUAL);
    }
}



void MainWindow::changeYAxisScaling(int id)
{
    if (id == ScopeGui::SCALE_AUTO)
    {
        // Full auto scaling
        _ui->radioYFullScale->setChecked(true);
        _gui->setyAxisScale(ScopeGui::SCALE_AUTO);
    }
    else if (id == ScopeGui::SCALE_MINMAX)
    {
        // Min and max selected
        _ui->radioYMinMax->setChecked(true);
        _gui->setyAxisScale(ScopeGui::SCALE_MINMAX, _ui->spinYMin->text().toInt(), _ui->spinYMax->text().toInt());
    }
    else
    {
        // manual
        _ui->radioYManual->setChecked(true);
        _gui->setyAxisScale(ScopeGui::SCALE_MANUAL);
    }
}

void MainWindow::addRegister()
{
    _pRegisterModel->insertRow(_pRegisterModel->rowCount());
}


void MainWindow::removeRegister()
{
    // get list of selected rows
    QItemSelectionModel *selected = _ui->registerView->selectionModel();
    QModelIndexList rowList = selected->selectedRows();

    // sort QModelIndexList
    // We need to remove the highest rows first
    qSort(rowList.begin(), rowList.end(), &MainWindow::sortRegistersLastFirst);

    foreach(QModelIndex rowIndex, rowList)
    {
        _pRegisterModel->removeRow(rowIndex.row(), rowIndex.parent());
    }
}

void MainWindow::setSettingsObjectsState(bool bState)
{
    _ui->spinPort->setEnabled(bState);
    _ui->btnAdd->setEnabled(bState);
    _ui->spinPollTime->setEnabled(bState);
    _ui->btnRemove->setEnabled(bState);
    _ui->spinSlaveId->setEnabled(bState);
    _ui->lineIP->setEnabled(bState);
    _ui->registerView->setEnabled(bState);
    _ui->actionLoadProjectFile->setEnabled(bState);
    _ui->actionExportDataCsv->setEnabled(bState);
    _ui->actionExportImage->setEnabled(bState);
}

void MainWindow::updateBoxes(ProjectFileParser::ProjectSettings * pProjectSettings)
{
    if (pProjectSettings->general.bIp)
    {
        _ui->lineIP->setText(pProjectSettings->general.ip);
    }

    if (pProjectSettings->general.bPort)
    {
        _ui->spinPort->setValue(pProjectSettings->general.port);
    }

    if (pProjectSettings->general.bPollTime)
    {
        _ui->spinPollTime->setValue(pProjectSettings->general.pollTime);
    }

    if (pProjectSettings->general.bSlaveId)
    {
        _ui->spinSlaveId->setValue(pProjectSettings->general.slaveId);
    }

    /*
     * TODO: add register data to view
    _ui->listReg->clear();
    for (qint32 i = 0; i < pProjectSettings->scope.registerList.size(); i++)
    {
        _ui->listReg->addItem(QString::number(pProjectSettings->scope.registerList[i].address));
    }
    */

}

void MainWindow::loadProjectFile(QString dataFilePath)
{
    ProjectFileParser fileParser;
    ProjectFileParser::ProjectSettings loadedSettings;
    QFile* file = new QFile(dataFilePath);

    /* If we can't open it, let's show an error message. */
    if (file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (fileParser.parseFile(file, &loadedSettings))
        {
            updateBoxes(&loadedSettings);

            // Set scaling to default
            // TODO: get scaling values from project file
            changeYAxisScaling(ScopeGui::SCALE_AUTO);
            changeXAxisScaling(ScopeGui::SCALE_AUTO);
        }
    }
    else
    {
        QMessageBox::critical(this,
                              "ModbusScope",
                              tr("Couldn't open project file: %1").arg(dataFilePath),
                              QMessageBox::Ok);
    }
}

void MainWindow::showAbout()
{
    QString lnkAuthor("<a href='mailto:jensgeudens@hotmail.com'>jgeudens</a>");
    QString lnkGpl("<a href='http://www.gnu.org/licenses/gpl.html#content'>GPL</a>");
    QString lnkGitHub("<a href='https://github.com/jgeudens/ModbusScope'>GitHub</a>");

    QString lnkQt("<a href='http://qt-project.org/'>Qt</a>");
    QString lnkLibModbus("<a href='http://libmodbus.org/'>libmodbus</a>");
    QString lnkQCustomPlot("<a href='http://www.qcustomplot.com/'>QCustomPlot</a>");


    QString aboutTxt = tr(
                        "ModbusScope is created and maintained by %1. This software is released under the %2 license. "
                        "The source is freely available at %3.<br><br>"
                        "ModbusScope uses following libraries:<br>"
                        "%4<br>"
                        "%5<br>"
                        "%6<br>").arg(lnkAuthor, lnkGpl, lnkGitHub, lnkQt, lnkLibModbus, lnkQCustomPlot);

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("About");
    msgBox.setTextFormat(Qt::RichText);   //this is what makes the links clickable
    msgBox.setText(aboutTxt);
    msgBox.exec();
}

bool MainWindow::sortRegistersLastFirst(const QModelIndex &s1, const QModelIndex &s2)
{
    return s1.row() > s2.row();
}

