#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qcustomplot.h"
#include "scopedata.h"
#include "scopegui.h"
#include "datafileparser.h"

#include "QDebug"
#include "QDateTime"


const QString MainWindow::_cWindowTitle = QString("ModbusScope");
const QString MainWindow::_cStateRunning = QString("Running");
const QString MainWindow::_cStateStopped = QString("Stopped");
const QString MainWindow::_cStatsTemplate = QString("Success: %1\tErrors: %2");
const QString MainWindow::_cRuntime = QString("Runtime: %1");

MainWindow::MainWindow(QStringList cmdArguments, QWidget *parent) :
    QMainWindow(parent),
    _pUi(new Ui::MainWindow),
    _pScope(NULL),
    _pGui(NULL)
{
    _pUi->setupUi(this);

    // Set window title
    this->setWindowTitle(_cWindowTitle);
    this->setAcceptDrops(true);

    _pGraphShowHide = _pUi->menuShowHide;
    _pGraphBringToFront = _pUi->menuBringToFront;
    _pBringToFrontGroup = new QActionGroup(this);

    // Add multipart status bar
    _pStatusState = new QLabel(_cStateStopped, this);
    _pStatusState->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _pStatusStats = new QLabel("", this);
    _pStatusStats->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _pStatusRuntime = new QLabel(_cRuntime.arg("0 hours, 0 minutes 0 seconds"), this);
    _pStatusRuntime->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    _pUi->statusBar->addPermanentWidget(_pStatusState, 0);
    _pUi->statusBar->addPermanentWidget(_pStatusRuntime, 0);
    _pUi->statusBar->addPermanentWidget(_pStatusStats, 2);

    // Setup registerView
    _pRegisterModel = new RegisterModel();
    _pUi->registerView->setModel(_pRegisterModel);
    _pUi->registerView->verticalHeader()->hide();

    _pUi->registerView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    _pUi->registerView->horizontalHeader()->setStretchLastSection(true);

    // Select using click, shift and control
     _pUi->registerView->setSelectionBehavior(QAbstractItemView::SelectRows);
     _pUi->registerView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    _pScope = new ScopeData();
    _pGui = new ScopeGui(this, _pScope, _pUi->customPlot, this);

    _projectFilePath = QString("");
    _lastDataFilePath = QString("");

    // For rightclick menu
    _pUi->customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_pUi->customPlot, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));


    connect(_pUi->spinSlidingXInterval, SIGNAL(valueChanged(int)), _pGui, SLOT(setxAxisSlidingInterval(int)));
    connect(_pUi->spinYMin, SIGNAL(valueChanged(int)), this, SLOT(updateYMin(int)));
    connect(_pUi->spinYMax, SIGNAL(valueChanged(int)), this, SLOT(updateYMax(int)));

    //valueChanged is only send when done editing...
    _pUi->spinSlidingXInterval->setKeyboardTracking(false);
    _pUi->spinYMin->setKeyboardTracking(false);
    _pUi->spinYMax->setKeyboardTracking(false);

    // Create button group for X axis scaling options
    _pXAxisScaleGroup = new QButtonGroup();
    _pXAxisScaleGroup->setExclusive(true);
    _pXAxisScaleGroup->addButton(_pUi->radioXFullScale, ScopeGui::SCALE_AUTO);
    _pXAxisScaleGroup->addButton(_pUi->radioXSliding, ScopeGui::SCALE_SLIDING);
    _pXAxisScaleGroup->addButton(_pUi->radioXManual, ScopeGui::SCALE_MANUAL);
    connect(_pXAxisScaleGroup, SIGNAL(buttonClicked(int)), this, SLOT(changeXAxisScaling(int)));

    // Default to full auto scaling
    changeXAxisScaling(ScopeGui::SCALE_AUTO);


    // Create button group for Y axis scaling options
    _pYAxisScaleGroup = new QButtonGroup();
    _pYAxisScaleGroup->setExclusive(true);
    _pYAxisScaleGroup->addButton(_pUi->radioYFullScale, ScopeGui::SCALE_AUTO);
    _pYAxisScaleGroup->addButton(_pUi->radioYMinMax, ScopeGui::SCALE_MINMAX);
    _pYAxisScaleGroup->addButton(_pUi->radioYManual, ScopeGui::SCALE_MANUAL);
    connect(_pYAxisScaleGroup, SIGNAL(buttonClicked(int)), this, SLOT(changeYAxisScaling(int)));

    // Default to full auto scaling
    changeYAxisScaling(ScopeGui::SCALE_AUTO);

    connect(_pScope, SIGNAL(handleReceivedData(QList<bool>, QList<double>)), _pGui, SLOT(plotResults(QList<bool>, QList<double>)));
    connect(_pScope, SIGNAL(triggerStatUpdate(quint32, quint32)), this, SLOT(updateStats(quint32, quint32)));
    connect(this, SIGNAL(dataExport(QString)), _pGui, SLOT(exportDataCsv(QString)));

    // Setup handler for buttons
    connect(_pUi->btnAdd, SIGNAL(released()), this, SLOT(addRegister()));
    connect(_pUi->btnRemove, SIGNAL(released()), this, SLOT(removeRegister()));

    // Handler for menu bar
    connect(_pUi->actionStart, SIGNAL(triggered()), this, SLOT(startScope()));
    connect(_pUi->actionStop, SIGNAL(triggered()), this, SLOT(stopScope()));
    connect(_pUi->actionExit, SIGNAL(triggered()), this, SLOT(exitApplication()));
    connect(_pUi->actionExportDataCsv, SIGNAL(triggered()), this, SLOT(prepareDataExport()));
    connect(_pUi->actionLoadProjectFile, SIGNAL(triggered()), this, SLOT(loadProjectSettings()));
    connect(_pUi->actionReloadProjectFile, SIGNAL(triggered()), this, SLOT(reloadProjectSettings()));
    connect(_pUi->actionImportDataFile, SIGNAL(triggered()), this, SLOT(importData()));
    connect(_pUi->actionExportImage, SIGNAL(triggered()), this, SLOT(prepareImageExport()));
    connect(_pUi->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));

    if (cmdArguments.size() > 1)
    {
        loadProjectFile(cmdArguments[1]);
    }

}

MainWindow::~MainWindow()
{
    delete _pScope;
    delete _pGraphShowHide;
    delete _pGraphBringToFront;
    delete _pUi;
}

void MainWindow::startScope()
{
    bool bCommunicationSettingsValid = true;

    const quint32 pollTime = _pUi->spinPollTime->text().toInt();

    if (bCommunicationSettingsValid)
    {
        /* TODO: check fields */
    }

    if (bCommunicationSettingsValid)
    {

        if (_pRegisterModel->checkedRegisterCount() != 0)
        {
            _commSettings.setIpAddress(_pUi->lineIP->text());
            _commSettings.setPort(_pUi->spinPort->text().toInt());
            _commSettings.setSlaveId(_pUi->spinSlaveId->text().toInt());
            _commSettings.setTimeout(_pUi->spinTimeout->text().toUInt());
            _commSettings.setPollTime(pollTime);

            _pUi->actionStart->setEnabled(false);
            _pUi->actionStop->setEnabled(true);
            _pUi->actionImportDataFile->setEnabled(false);

            setSettingsObjectsState(false);

            _pStatusState->setText(_cStateRunning);
            _pStatusRuntime->setText(_cRuntime.arg("0 hours, 0 minutes 0 seconds"));

            _runtimeTimer.singleShot(250, this, SLOT(updateRuntime()));

            QList<ScopeData::RegisterData> regList;
            _pRegisterModel->getCheckedRegisterList(&regList);

            if (_pScope->startCommunication(&_commSettings, regList))
            {
                QList<QString> regTextList;
                _pRegisterModel->getCheckedRegisterTextList(&regTextList);

                _pGui->resetGraph();
                _pGui->setupGraph(regTextList);

                // Clear actions
                _pGraphShowHide->clear();
                _pBringToFrontGroup->actions().clear();
                _pGraphBringToFront->clear();

                // Add menu-items
                for (qint32 i = 0; i < regTextList.size(); i++)
                {
                    QAction * pShowHideAction = _pGraphShowHide->addAction(regTextList[i]);
                    QAction * pBringToFront = _pGraphBringToFront->addAction(regTextList[i]);

                    QPixmap pixmap(20,5);
                    pixmap.fill(_pGui->getGraphColor(i));
                    QIcon * pBringToFrontIcon = new QIcon(pixmap);
                    QIcon * pShowHideIcon = new QIcon(pixmap);

                    pShowHideAction->setData(i);
                    pShowHideAction->setIcon(*pBringToFrontIcon);
                    pShowHideAction->setCheckable(true);
                    pShowHideAction->setChecked(true);

                    pBringToFront->setData(i);
                    pBringToFront->setIcon(*pShowHideIcon);
                    pBringToFront->setCheckable(true);
                    pBringToFront->setActionGroup(_pBringToFrontGroup);

                    QObject::connect(pShowHideAction, SIGNAL(toggled(bool)), this, SLOT(showHideGraph(bool)));
                    QObject::connect(pBringToFront, SIGNAL(toggled(bool)), this, SLOT(bringToFrontGraph(bool)));
                }

                _pGraphShowHide->setEnabled(true);
                _pGraphBringToFront->setEnabled(true);
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
    _pScope->stopCommunication();

    _pUi->actionStart->setEnabled(true);
    _pUi->actionStop->setEnabled(false);
    _pUi->actionImportDataFile->setEnabled(true);

    _pStatusState->setText(_cStateStopped);

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

    if (_lastDataFilePath.trimmed().isEmpty())
    {
        QStringList docPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
        if (docPath.size() > 0)
        {
            dialog.setDirectory(docPath[0]);
        }
    }
    else
    {
        dialog.setDirectory(QFileInfo(_lastDataFilePath).dir());
    }

    if (dialog.exec())
    {
        filePath = dialog.selectedFiles().first();
        _lastDataFilePath = filePath;
        emit dataExport(filePath);
    }
}

void MainWindow::loadProjectSettings()
{
    QString filePath;
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setOption(QFileDialog::HideNameFilterDetails, false);
    dialog.setWindowTitle(tr("Select mbs file"));
    dialog.setNameFilter(tr("mbs files (*.mbs)"));

    if (_projectFilePath.trimmed().isEmpty())
    {
        QStringList docPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
        if (docPath.size() > 0)
        {
            dialog.setDirectory(docPath[0]);
        }
    }
    else
    {
        dialog.setDirectory(QFileInfo(_projectFilePath).dir());
    }

    if (dialog.exec())
    {
        filePath = dialog.selectedFiles().first();
        loadProjectFile(filePath);
    }

}


void MainWindow::reloadProjectSettings()
{
    loadProjectFile(_projectFilePath);
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

    if (_lastDataFilePath.trimmed().isEmpty())
    {
        QStringList docPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
        if (docPath.size() > 0)
        {
            dialog.setDirectory(docPath[0]);
        }
    }
    else
    {
        dialog.setDirectory(QFileInfo(_lastDataFilePath).dir());
    }

    if (dialog.exec())
    {
        filePath = dialog.selectedFiles().first();
        _lastDataFilePath  = filePath;
        _pGui->exportGraphImage(filePath);
    }
}


void MainWindow::updateYMin(int newMin)
{
    const qint32 min = _pGui->getyAxisMin();
    const qint32 max = _pGui->getyAxisMax();
    const qint32 diff = max - min;
    qint32 newMax = max;

    if (newMin >= max)
    {
        newMax = newMin + diff;
    }

    _pGui->setyAxisMinMax(newMin, newMax);
    _pUi->spinYMax->setValue(newMax);
}


void MainWindow::updateYMax(int newMax)
{
    const qint32 min = _pGui->getyAxisMin();
    const qint32 max = _pGui->getyAxisMax();
    const qint32 diff = max - min;

    qint32 newMin = min;

    if (newMax <= min)
    {
        newMin = newMax - diff;
    }

    _pGui->setyAxisMinMax(newMin, newMax);
    _pUi->spinYMin->setValue(newMin);
}



void MainWindow::updateStats(quint32 successCount, quint32 errorCount)
{
    // Update statistics
    _pStatusStats->setText(_cStatsTemplate.arg(successCount).arg(errorCount));
}

void MainWindow::changeXAxisScaling(int id)
{
    if (id == ScopeGui::SCALE_AUTO)
    {
        // Full auto scaling
        _pUi->radioXFullScale->setChecked(true);
        _pGui->setxAxisScale(ScopeGui::SCALE_AUTO);
    }
    else if (id == ScopeGui::SCALE_SLIDING)
    {
        // Sliding window
        _pUi->radioXSliding->setChecked(true);
        _pGui->setxAxisScale(ScopeGui::SCALE_SLIDING, _pUi->spinSlidingXInterval->text().toUInt());
    }
    else
    {
        // manual
        _pUi->radioXManual->setChecked(true);
        _pGui->setxAxisScale(ScopeGui::SCALE_MANUAL);
    }
}



void MainWindow::changeYAxisScaling(int id)
{
    if (id == ScopeGui::SCALE_AUTO)
    {
        // Full auto scaling
        _pUi->radioYFullScale->setChecked(true);
        _pGui->setyAxisScale(ScopeGui::SCALE_AUTO);
    }
    else if (id == ScopeGui::SCALE_MINMAX)
    {
        // Min and max selected
        _pUi->radioYMinMax->setChecked(true);
        _pGui->setyAxisScale(ScopeGui::SCALE_MINMAX, _pUi->spinYMin->text().toInt(), _pUi->spinYMax->text().toInt());
    }
    else
    {
        // manual
        _pUi->radioYManual->setChecked(true);
        _pGui->setyAxisScale(ScopeGui::SCALE_MANUAL);
    }
}

void MainWindow::addRegister()
{
    _pRegisterModel->insertRow(_pRegisterModel->rowCount());
}


void MainWindow::removeRegister()
{
    // get list of selected rows
    QItemSelectionModel *selected = _pUi->registerView->selectionModel();
    QModelIndexList rowList = selected->selectedRows();

    // sort QModelIndexList
    // We need to remove the highest rows first
    std::sort(rowList.begin(), rowList.end(), &MainWindow::sortRegistersLastFirst);

    foreach(QModelIndex rowIndex, rowList)
    {
        _pRegisterModel->removeRow(rowIndex.row(), rowIndex.parent());
    }
}

void MainWindow::setSettingsObjectsState(bool bState)
{
    _pUi->spinPort->setEnabled(bState);
    _pUi->btnAdd->setEnabled(bState);
    _pUi->spinPollTime->setEnabled(bState);
    _pUi->btnRemove->setEnabled(bState);
    _pUi->spinSlaveId->setEnabled(bState);
    _pUi->spinTimeout->setEnabled(bState);
    _pUi->lineIP->setEnabled(bState);
    _pUi->registerView->setEnabled(bState);
    _pUi->actionLoadProjectFile->setEnabled(bState);

    // if a project file is previously loaded, then it can be reloaded
    if (_projectFilePath.isEmpty())
    {
        _pUi->actionReloadProjectFile->setEnabled(false);
    }
    else
    {
        _pUi->actionReloadProjectFile->setEnabled(bState);
    }

    _pUi->actionExportDataCsv->setEnabled(bState);
    _pUi->actionExportImage->setEnabled(bState);
}

void MainWindow::updateBoxes(ProjectFileParser::ProjectSettings * pProjectSettings)
{
    if (pProjectSettings->general.bIp)
    {
        _pUi->lineIP->setText(pProjectSettings->general.ip);
    }

    if (pProjectSettings->general.bPort)
    {
        _pUi->spinPort->setValue(pProjectSettings->general.port);
    }

    if (pProjectSettings->general.bPollTime)
    {
        _pUi->spinPollTime->setValue(pProjectSettings->general.pollTime);
    }

    if (pProjectSettings->general.bSlaveId)
    {
        _pUi->spinSlaveId->setValue(pProjectSettings->general.slaveId);
    }

    if (pProjectSettings->general.bTimeout)
    {
        _pUi->spinTimeout->setValue(pProjectSettings->general.timeout);
    }

    if (pProjectSettings->scale.bSliding)
    {
        _pUi->spinSlidingXInterval->setValue(pProjectSettings->scale.slidingInterval);
        _pUi->radioXSliding->setChecked(true);
    }
    else
    {
        _pUi->radioXFullScale->setChecked(true);
    }

    if (pProjectSettings->scale.bMinMax)
    {
        _pUi->spinYMin->setValue(pProjectSettings->scale.scaleMin);
        _pUi->spinYMax->setValue(pProjectSettings->scale.scaleMax);
        _pUi->radioYMinMax->setChecked(true);
    }
    else
    {
        _pUi->radioYFullScale->setChecked(true);
    }

    _pRegisterModel->clear();
    for (qint32 i = 0; i < pProjectSettings->scope.registerList.size(); i++)
    {
        RegisterModel::RegisterData rowData;
        rowData.bActive = pProjectSettings->scope.registerList[i].bActive;
        rowData.bUnsigned = pProjectSettings->scope.registerList[i].bUnsigned;
        rowData.reg = pProjectSettings->scope.registerList[i].address;
        rowData.text = pProjectSettings->scope.registerList[i].text;
        rowData.scaleFactor = pProjectSettings->scope.registerList[i].scaleFactor;
        _pRegisterModel->appendRow(rowData);
    }

    // make registerview resize to content
    _pUi->registerView->resizeColumnsToContents();

}

void MainWindow::loadProjectFile(QString dataFilePath)
{
    ProjectFileParser fileParser;
    ProjectFileParser::ProjectSettings loadedSettings;
    QFile file(dataFilePath);

    /* If we can't open it, let's show an error message. */
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (fileParser.parseFile(&file, &loadedSettings))
        {
            updateBoxes(&loadedSettings);

            _projectFilePath = dataFilePath;
            setWindowTitle(QString(tr("%1 - %2")).arg(_cWindowTitle, QFileInfo(_projectFilePath).fileName()));

            // Enable reload menu item
            _pUi->actionReloadProjectFile->setEnabled(true);

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

void MainWindow::loadDataFile(QString dataFilePath)
{
    QFile file(dataFilePath);

    /* If we can't open it, let's show an error message. */
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        DataFileParser dataParser(&file);
        DataFileParser::FileData data;
        if (dataParser.processDataFile(&data))
        {
            // If success, disable export data
            _pUi->actionExportDataCsv->setEnabled(false);

            _pStatusRuntime->setText(_cRuntime.arg("0 hours, 0 minutes 0 seconds"));
            _pStatusStats->setText(_cStatsTemplate.arg(0).arg(0));

            // Set to full auto scaling
            changeXAxisScaling(ScopeGui::SCALE_AUTO);

            // Set to full auto scaling
            changeYAxisScaling(ScopeGui::SCALE_AUTO);

            _pGui->loadFileData(&data);
        }

    }
    else
    {
        QMessageBox::critical(this,
                              "ModbusScope",
                              tr("Couldn't open data file: %1").arg(dataFilePath),
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

    QString version = QString(tr("<b>ModbusScope v%1</b><br><br>")).arg(APP_VERSION);

    QString aboutTxt = tr(
                        "%1"
                        "ModbusScope is created and maintained by %2. This software is released under the %3 license. "
                        "The source is freely available at %4.<br><br>"
                        "ModbusScope uses following libraries:<br>"
                        "%5<br>"
                        "%6<br>"
                        "%7<br>").arg(version, lnkAuthor, lnkGpl, lnkGitHub, lnkQt, lnkLibModbus, lnkQCustomPlot);

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("About");
    msgBox.setTextFormat(Qt::RichText);   //this is what makes the links clickable
    msgBox.setText(aboutTxt);
    msgBox.exec();
}

void MainWindow::importData()
{
    QString filePath;
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setOption(QFileDialog::HideNameFilterDetails, false);
    dialog.setWindowTitle(tr("Select csv file"));
    dialog.setNameFilter(tr("csv files (*.csv)"));

    if (_lastDataFilePath.trimmed().isEmpty())
    {
        QStringList docPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
        if (docPath.size() > 0)
        {
            dialog.setDirectory(docPath[0]);
        }
    }
    else
    {
        dialog.setDirectory(QFileInfo(_lastDataFilePath).dir());
    }

    if (dialog.exec())
    {
        filePath = dialog.selectedFiles().first();
        _lastDataFilePath = filePath;
        loadDataFile(filePath);
    }
}

void MainWindow::updateRuntime()
{
    qint64 timePassed = QDateTime::currentMSecsSinceEpoch() - _pScope->getCommunicationStartTime();

    // Convert to s
    timePassed /= 1000;

    const quint32 h = (timePassed / 3600);
    timePassed = timePassed % 3600;

    const quint32 m = (timePassed / 60);
    timePassed = timePassed % 60;

    const quint32 s = timePassed;

    QString strTimePassed = QString("%1 hours, %2 minutes %3 seconds").arg(h).arg(m).arg(s);

    _pStatusRuntime->setText(_cRuntime.arg(strTimePassed));

    // restart timer
    if (_pScope->isActive())
    {
        _runtimeTimer.singleShot(250, this, SLOT(updateRuntime()));
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
    {
        e->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *e)
{
    foreach (const QUrl &url, e->mimeData()->urls())
    {
        loadProjectFile(url.toLocalFile());
    }
}

void MainWindow::bringToFrontGraph(bool bState)
{
    QAction * pAction = qobject_cast<QAction *>(QObject::sender());

    _pGui->bringToFront(pAction->data().toInt(), bState);
}

void MainWindow::showHideGraph(bool bState)
{
    QAction * pAction = qobject_cast<QAction *>(QObject::sender());

    _pGui->showGraph(pAction->data().toInt(), bState);

    // Show/Hide corresponding "BringToFront" action
    _pGraphBringToFront->actions().at(pAction->data().toInt())->setVisible(bState);

    // Enable/Disable BringToFront menu
    bool bVisible = false;
    foreach(QAction * pAction, _pGraphBringToFront->actions())
    {
        if (pAction->isVisible())
        {
            bVisible = true;
            break;
        }
    }

    _pGraphBringToFront->setEnabled(bVisible);
}

void MainWindow::showContextMenu(const QPoint& pos)
{
    _pUi->menuView->popup(_pUi->customPlot->mapToGlobal(pos));
}

bool MainWindow::sortRegistersLastFirst(const QModelIndex &s1, const QModelIndex &s2)
{
    return s1.row() > s2.row();
}

