#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qcustomplot.h"
#include "scopedata.h"
#include "registerdata.h"
#include "registermodel.h"
#include "registerdialog.h"
#include "connectiondialog.h"
#include "connectionmodel.h"
#include "guimodel.h"
#include "extendedgraphview.h"

#include <QDebug>
#include <QDateTime>

const QString MainWindow::_cStateRunning = QString("Running");
const QString MainWindow::_cStateStopped = QString("Stopped");
const QString MainWindow::_cStatsTemplate = QString("Success: %1\tErrors: %2");
const QString MainWindow::_cRuntime = QString("Runtime: %1");

MainWindow::MainWindow(QStringList cmdArguments, QWidget *parent) :
    QMainWindow(parent),
    _pUi(new Ui::MainWindow)
{
    _pUi->setupUi(this);


    _pConnectionModel = new ConnectionModel();
    _pConnectionDialog = new ConnectionDialog(_pConnectionModel);

    _pRegisterModel = new RegisterModel();
    _pRegisterDialog = new RegisterDialog(_pRegisterModel);

    _pGraphView = new ExtendedGraphView(_pGuiModel, _pUi->customPlot, this);
    _pScope = new ScopeData(_pConnectionModel);

    /*-- Connect menu actions --*/
    connect(_pUi->actionStart, SIGNAL(triggered()), this, SLOT(startScope()));
    connect(_pUi->actionStop, SIGNAL(triggered()), this, SLOT(stopScope()));
    connect(_pUi->actionExit, SIGNAL(triggered()), this, SLOT(exitApplication()));
    connect(_pUi->actionExportDataCsv, SIGNAL(triggered()), this, SLOT(prepareDataExport()));
    connect(_pUi->actionLoadProjectFile, SIGNAL(triggered()), this, SLOT(loadProjectSettings()));
    connect(_pUi->actionReloadProjectFile, SIGNAL(triggered()), this, SLOT(reloadProjectSettings()));
    connect(_pUi->actionImportDataFile, SIGNAL(triggered()), this, SLOT(importData()));
    connect(_pUi->actionExportImage, SIGNAL(triggered()), this, SLOT(prepareImageExport()));
    connect(_pUi->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    connect(_pUi->actionShowValueTooltip, SIGNAL(toggled(bool)), _pGuiModel, SLOT(setValueTooltip(bool)));
    connect(_pUi->actionHighlightSamplePoints, SIGNAL(toggled(bool)), _pGuiModel, SLOT(setHighlightSamples(bool)));
    connect(_pUi->actionClearData, SIGNAL(triggered()), this, SLOT(clearData()));
    connect(_pUi->actionConnectionSettings, SIGNAL(triggered()), this, SLOT(showConnectionDialog()));
    connect(_pUi->actionRegisterSettings, SIGNAL(triggered()), this, SLOT(showRegisterDialog()));

    /*-- connect model to view --*/
    connect(_pGuiModel, SIGNAL(graphVisibilityChanged(const quint32)), this, SLOT(showHideGraph(const quint32)));
    connect(_pGuiModel, SIGNAL(graphVisibilityChanged(const quint32)), _pGraphView, SLOT(showGraph(const quint32)));
    connect(_pGuiModel, SIGNAL(frontGraphChanged()), this, SLOT(updateBringToFrontGrapMenu()));
    connect(_pGuiModel, SIGNAL(frontGraphChanged()), _pGraphView, SLOT(bringToFront()));
    connect(_pGuiModel, SIGNAL(highlightSamplesChanged()), this, SLOT(updateHighlightSampleMenu()));
    connect(_pGuiModel, SIGNAL(highlightSamplesChanged()), _pGraphView, SLOT(enableSamplePoints()));
    connect(_pGuiModel, SIGNAL(valueTooltipChanged()), this, SLOT(updateValueTooltipMenu()));
    connect(_pGuiModel, SIGNAL(valueTooltipChanged()), _pGraphView, SLOT(enableValueTooltip()));
    connect(_pGuiModel, SIGNAL(graphCleared()), _pGraphView, SLOT(clearGraphs()));
    connect(_pGuiModel, SIGNAL(graphCleared()), this, SLOT(clearGraphMenu()));
    connect(_pGuiModel, SIGNAL(graphsAdded()), _pGraphView, SLOT(addGraphs()));
    connect(_pGuiModel, SIGNAL(graphsAddedWithData(QList<QList<double> >)), _pGraphView, SLOT(addGraphs(QList<QList<double> >)));
    connect(_pGuiModel, SIGNAL(graphsAdded()), this, SLOT(addGraphMenu()));
    connect(_pGuiModel, SIGNAL(windowTitleChanged()), this, SLOT(updateWindowTitle()));
    //connect(_pGuiModel, SIGNAL(loadedFileChanged()), this, SLOT(enableGlobalMenu()));
    //connect(_pGuiModel, SIGNAL(legendVisibilityChanged()), _pGraphView, SLOT(showHideLegend()));

    connect(_pGuiModel, SIGNAL(xAxisScalingChanged()), this, SLOT(updatexAxisSlidingMode()));
    connect(_pGuiModel, SIGNAL(xAxisScalingChanged()), _pGraphView, SLOT(rescalePlot()));
    connect(_pGuiModel, SIGNAL(xAxisSlidingIntervalChanged()), this, SLOT(updatexAxisSlidingInterval()));
    connect(_pGuiModel, SIGNAL(xAxisSlidingIntervalChanged()), _pGraphView, SLOT(rescalePlot()));

    connect(_pGuiModel, SIGNAL(yAxisScalingChanged()), this, SLOT(updateyAxisSlidingMode()));
    connect(_pGuiModel, SIGNAL(yAxisScalingChanged()), _pGraphView, SLOT(rescalePlot()));
    connect(_pGuiModel, SIGNAL(yAxisMinMaxchanged()), this, SLOT(updateyAxisMinMax()));
    connect(_pGuiModel, SIGNAL(yAxisMinMaxchanged()), _pGraphView, SLOT(rescalePlot()));

    _pGraphShowHide = _pUi->menuShowHide;
    _pGraphBringToFront = _pUi->menuBringToFront;
    _pBringToFrontGroup = new QActionGroup(this);

    _pGuiModel->setWindowTitleDetail("");


    // TODO; mode to correct model code
    _pLegendPositionGroup = new QActionGroup(this);
    _pUi->actionLegendLeft->setActionGroup(_pLegendPositionGroup);
    _pUi->actionLegendMiddle->setActionGroup(_pLegendPositionGroup);
    _pUi->actionLegendRight->setActionGroup(_pLegendPositionGroup);
    connect(_pLegendPositionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeLegendPosition(QAction*)));


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

    _pGuiModel->setProjectFilePath(QString(""));
    _pGuiModel->setLastDataFilePath(QString(""));

    this->setAcceptDrops(true);

    // For rightclick menu
    _pUi->customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_pUi->customPlot, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));

    /* Update interface via model */
    _pGuiModel->triggerUpdate();

    connect(_pUi->spinSlidingXInterval, SIGNAL(valueChanged(int)), _pGuiModel, SLOT(setxAxisSlidingInterval(quint32)));
    connect(_pUi->spinYMin, SIGNAL(valueChanged(int)), _pGuiModel, SLOT(setyAxisMin(quint32)));
    connect(_pUi->spinYMax, SIGNAL(valueChanged(int)), _pGuiModel, SLOT(setyAxisMax(quint32)));

    //valueChanged is only send when done editing...
    _pUi->spinSlidingXInterval->setKeyboardTracking(false);
    _pUi->spinYMin->setKeyboardTracking(false);
    _pUi->spinYMax->setKeyboardTracking(false);

    // Create button group for X axis scaling options
    _pXAxisScaleGroup = new QButtonGroup();
    _pXAxisScaleGroup->setExclusive(true);
    _pXAxisScaleGroup->addButton(_pUi->radioXFullScale, BasicGraphView::SCALE_AUTO);
    _pXAxisScaleGroup->addButton(_pUi->radioXSliding, BasicGraphView::SCALE_SLIDING);
    _pXAxisScaleGroup->addButton(_pUi->radioXManual, BasicGraphView::SCALE_MANUAL);
    connect(_pXAxisScaleGroup, SIGNAL(buttonClicked(int)), _pGuiModel, SLOT(setxAxisScale(BasicGraphView::AxisScaleOptions)));

    // Create button group for Y axis scaling options
    _pYAxisScaleGroup = new QButtonGroup();
    _pYAxisScaleGroup->setExclusive(true);
    _pYAxisScaleGroup->addButton(_pUi->radioYFullScale, BasicGraphView::SCALE_AUTO);
    _pYAxisScaleGroup->addButton(_pUi->radioYMinMax, BasicGraphView::SCALE_MINMAX);
    _pYAxisScaleGroup->addButton(_pUi->radioYManual, BasicGraphView::SCALE_MANUAL);
    connect(_pYAxisScaleGroup, SIGNAL(buttonClicked(int)), this, SLOT(setyAxisScale(BasicGraphView::AxisScaleOptions)));

    // Default to full auto scaling
    _pGuiModel->setxAxisScale(BasicGraphView::SCALE_AUTO);
    _pGuiModel->setyAxisScale(BasicGraphView::SCALE_AUTO);

    connect(_pScope, SIGNAL(handleReceivedData(QList<bool>, QList<double>)), _pGraphView, SLOT(plotResults(QList<bool>, QList<double>)));
    connect(_pScope, SIGNAL(triggerStatUpdate(quint32, quint32)), this, SLOT(updateStats(quint32, quint32)));
    connect(this, SIGNAL(dataExport(QString)), _pGraphView, SLOT(exportDataCsv(QString)));

    if (cmdArguments.size() > 1)
    {
        loadProjectFile(cmdArguments[1]);
    }
}

MainWindow::~MainWindow()
{
    delete _pGraphView;
    delete _pGuiModel;
    delete _pConnectionDialog;
    delete _pRegisterDialog;
    delete _pRegisterModel;
    delete _pConnectionModel;
    delete _pScope;
    delete _pGraphShowHide;
    delete _pGraphBringToFront;
    delete _pUi;
}

void MainWindow::updateStats(quint32 successCount, quint32 errorCount)
{
    // Update statistics
    _pStatusStats->setText(_cStatsTemplate.arg(successCount).arg(errorCount));
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

    if (_pGuiModel->projectFilePath().trimmed().isEmpty())
    {
        QStringList docPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
        if (docPath.size() > 0)
        {
            dialog.setDirectory(docPath[0]);
        }
    }
    else
    {
        dialog.setDirectory(QFileInfo(_pGuiModel->projectFilePath()).dir());
    }

    if (dialog.exec())
    {
        filePath = dialog.selectedFiles().first();
        loadProjectFile(filePath);
    }

}

void MainWindow::reloadProjectSettings()
{
    loadProjectFile(_pGuiModel->projectFilePath());
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

    if (_pGuiModel->lastDataFilePath().trimmed().isEmpty())
    {
        QStringList docPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
        if (docPath.size() > 0)
        {
            dialog.setDirectory(docPath[0]);
        }
    }
    else
    {
        dialog.setDirectory(QFileInfo(_pGuiModel->lastDataFilePath()).dir());
    }

    if (dialog.exec())
    {
        filePath = dialog.selectedFiles().first();
        loadDataFile(filePath);
    }
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

    if (_pGuiModel->lastDataFilePath().trimmed().isEmpty())
    {
        QStringList docPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
        if (docPath.size() > 0)
        {
            dialog.setDirectory(docPath[0]);
        }
    }
    else
    {
        dialog.setDirectory(QFileInfo(_pGuiModel->lastDataFilePath()).dir());
    }

    if (dialog.exec())
    {
        filePath = dialog.selectedFiles().first();
        _pGuiModel->setLastDataFilePath(filePath);
        emit dataExport(filePath);
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

    if (_pGuiModel->lastDataFilePath().trimmed().isEmpty())
    {
        QStringList docPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
        if (docPath.size() > 0)
        {
            dialog.setDirectory(docPath[0]);
        }
    }
    else
    {
        dialog.setDirectory(QFileInfo(_pGuiModel->lastDataFilePath()).dir());
    }

    if (dialog.exec())
    {
        filePath = dialog.selectedFiles().first();
        _pGuiModel->setLastDataFilePath(filePath);
        _pGraphView->exportGraphImage(filePath);
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

void MainWindow::menuBringToFrontGraphClicked(bool bState)
{
    QAction * pAction = qobject_cast<QAction *>(QObject::sender());

    if (bState)
    {
        _pGuiModel->setFrontGraph(pAction->data().toInt());
    }
}

void MainWindow::menuShowHideGraphClicked(bool bState)
{
    QAction * pAction = qobject_cast<QAction *>(QObject::sender());

    _pGuiModel->setGraphVisibility(pAction->data().toInt(), bState);

}

void MainWindow::showConnectionDialog()
{
    _pConnectionDialog->exec();
}

void MainWindow::showRegisterDialog()
{
    _pRegisterDialog->exec();
}

void MainWindow::changeLegendPosition(QAction* pAction)
{
    if (pAction == _pUi->actionLegendLeft)
    {
        _pGraphView->setLegendPosition(BasicGraphView::LEGEND_LEFT);
    }
    else if (pAction == _pUi->actionLegendMiddle)
    {
        _pGraphView->setLegendPosition(BasicGraphView::LEGEND_MIDDLE);
    }
    else if (pAction == _pUi->actionLegendRight)
    {
        _pGraphView->setLegendPosition(BasicGraphView::LEGEND_RIGHT);
    }
}

void MainWindow::clearData()
{
    _pScope->resetCommunicationStats();
    _pGraphView->clearResults();
}

void MainWindow::startScope()
{
    if (_pRegisterModel->checkedRegisterCount() != 0)
    {
        _pUi->actionStart->setEnabled(false);
        _pUi->actionStop->setEnabled(true);
        _pUi->actionImportDataFile->setEnabled(false);
        _pUi->actionShowValueTooltip->setEnabled(true);
        _pUi->actionHighlightSamplePoints->setEnabled(true);
        _pUi->actionClearData->setEnabled(true);
        _pUi->menuLegendPosition->setEnabled(true);
        setSettingsObjectsState(false);

        _pStatusState->setText(_cStateRunning);
        _pStatusRuntime->setText(_cRuntime.arg("0 hours, 0 minutes 0 seconds"));

        _runtimeTimer.singleShot(250, this, SLOT(updateRuntime()));

        QList<RegisterData> regList;
        _pRegisterModel->getCheckedRegisterList(&regList);

        if (_pScope->startCommunication(regList))
        {
            QList<RegisterData> regList;
            _pRegisterModel->getCheckedRegisterList(&regList);

            _pGuiModel->clearGraph();
            _pGuiModel->addGraphs(regList);
        }
    }
    else
    {
        QMessageBox::warning(this, "No register in scope list!", "There are no register in the scope list. Please select at least one register.");
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

void MainWindow::showHideGraph(const quint32 index)
{
    _pGraphShowHide->actions().at(index)->setChecked(_pGuiModel->graphVisibility(index));

    // Show/Hide corresponding "BringToFront" action
    _pGraphBringToFront->actions().at(index)->setVisible(_pGuiModel->graphVisibility((index)));

    // Enable/Disable global BringToFront menu
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

void MainWindow::updateBringToFrontGrapMenu()
{
    if (_pBringToFrontGroup->actions().size() > 0)
    {
        _pBringToFrontGroup->actions().at(_pGuiModel->frontGraph())->setChecked(true);
    }
}

void MainWindow::updateHighlightSampleMenu()
{
    /* set menu to checked */
    _pUi->actionHighlightSamplePoints->setChecked(_pGuiModel->highlightSamples());
}

void MainWindow::updateValueTooltipMenu()
{
    /* set menu to checked */
    _pUi->actionShowValueTooltip->setChecked(_pGuiModel->valueTooltip());
}

void MainWindow::clearGraphMenu()
{
    // Clear actions
    _pGraphShowHide->clear();
    _pBringToFrontGroup->actions().clear();
    _pGraphBringToFront->clear();
}

void MainWindow::addGraphMenu()
{

    for (quint32 idx = 0; idx < _pGuiModel->graphCount(); idx++)
    {
        QString label = _pGuiModel->graphLabel(idx);
        QAction * pShowHideAction = _pGraphShowHide->addAction(label);
        QAction * pBringToFront = _pGraphBringToFront->addAction(label);

        QPixmap pixmap(20,5);
        pixmap.fill(_pGuiModel->graphColor(idx));
        QIcon * pBringToFrontIcon = new QIcon(pixmap);
        QIcon * pShowHideIcon = new QIcon(pixmap);

        pShowHideAction->setData(idx);
        pShowHideAction->setIcon(*pBringToFrontIcon);
        pShowHideAction->setCheckable(true);
        pShowHideAction->setChecked(_pGuiModel->graphVisibility(idx));

        pBringToFront->setData(idx);
        pBringToFront->setIcon(*pShowHideIcon);
        pBringToFront->setCheckable(true);
        pBringToFront->setActionGroup(_pBringToFrontGroup);

        QObject::connect(pShowHideAction, SIGNAL(toggled(bool)), this, SLOT(menuShowHideGraphClicked(bool)));
        QObject::connect(pBringToFront, SIGNAL(toggled(bool)), this, SLOT(menuBringToFrontGraphClicked(bool)));
    }

    _pGraphShowHide->setEnabled(true);
    _pGraphBringToFront->setEnabled(true);
}

void MainWindow::updateWindowTitle()
{
    setWindowTitle(_pGuiModel->windowTitle());
}

void MainWindow::updatexAxisSlidingMode()
{
    if (_pGuiModel->xAxisScalingMode() == BasicGraphView::SCALE_AUTO)
    {
        // Full auto scaling
        _pUi->radioXFullScale->setChecked(true);
    }
    else if (_pGuiModel->xAxisScalingMode() == BasicGraphView::SCALE_SLIDING)
    {
        // Sliding window
        _pUi->radioXSliding->setChecked(true);
    }
    else
    {
        // manual
        _pUi->radioXManual->setChecked(true);
    }
}

void MainWindow::updatexAxisSlidingInterval()
{
    _pUi->spinSlidingXInterval->setValue(_pGuiModel->xAxisSlidingSec());
}

void MainWindow::updateyAxisSlidingMode()
{
    if (_pGuiModel->yAxisScalingMode() == BasicGraphView::SCALE_AUTO)
    {
        // Full auto scaling
        _pUi->radioYFullScale->setChecked(true);
    }
    else if (_pGuiModel->yAxisScalingMode() == BasicGraphView::SCALE_MINMAX)
    {
        // Min and max selected
        _pUi->radioYMinMax->setChecked(true);
    }
    else
    {
        // manual
        _pUi->radioYManual->setChecked(true);
    }
}

void MainWindow::updateyAxisMinMax()
{
    _pUi->spinYMin->setValue(_pGuiModel->yAxisMin());
    _pUi->spinYMax->setValue(_pGuiModel->yAxisMax());
}

void MainWindow::showContextMenu(const QPoint& pos)
{
    _pUi->menuView->popup(_pUi->customPlot->mapToGlobal(pos));
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
    if (!_pScope->isActive())
    {
        const QString filename(e->mimeData()->urls().last().toLocalFile());
        QFileInfo fileInfo(filename);
        if (fileInfo.completeSuffix() == QString("mbs"))
        {
            loadProjectFile(filename);
        }
        else if (fileInfo.completeSuffix() == QString("csv"))
        {
            loadDataFile(filename);
        }
        else
        {
            // ignore drop
        }
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

void MainWindow::setSettingsObjectsState(bool bState)
{
    _pUi->actionLoadProjectFile->setEnabled(bState);

    // if a project file is previously loaded, then it can be reloaded
    if (_pGuiModel->projectFilePath().isEmpty())
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

void MainWindow::updateConnectionSetting(ProjectFileParser::ProjectSettings * pProjectSettings)
{
    _pConnectionModel->clearData();

    if (pProjectSettings->general.bIp)
    {
        _pConnectionModel->setIpAddress(pProjectSettings->general.ip);
    }

    if (pProjectSettings->general.bPort)
    {
         _pConnectionModel->setPort(pProjectSettings->general.port);
    }

    if (pProjectSettings->general.bPollTime)
    {
        _pConnectionModel->setPollTime(pProjectSettings->general.pollTime);
    }

    if (pProjectSettings->general.bSlaveId)
    {
        _pConnectionModel->setSlaveId(pProjectSettings->general.slaveId);
    }

    if (pProjectSettings->general.bTimeout)
    {
        _pConnectionModel->setTimeout(pProjectSettings->general.timeout);
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
        RegisterData rowData;
        rowData.setActive(pProjectSettings->scope.registerList[i].bActive);
        rowData.setUnsigned(pProjectSettings->scope.registerList[i].bUnsigned);
        rowData.setRegisterAddress(pProjectSettings->scope.registerList[i].address);
        rowData.setText(pProjectSettings->scope.registerList[i].text);
        rowData.setScaleFactor(pProjectSettings->scope.registerList[i].scaleFactor);
        rowData.setColor(pProjectSettings->scope.registerList[i].color);

        _pRegisterModel->appendRow(rowData);
    }
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
            updateConnectionSetting(&loadedSettings);

            _pGuiModel->setProjectFilePath(dataFilePath);
            _pGuiModel->setWindowTitleDetail(QFileInfo(_pGuiModel->projectFilePath()).fileName());

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

    _pGuiModel->setLastDataFilePath(dataFilePath);

    /* If we can't open it, let's show an error message. */
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        DataFileParser dataParser(&file);
        DataFileParser::FileData data;
        if (dataParser.processDataFile(&data))
        {
            // If success, disable export data
            _pUi->actionExportDataCsv->setEnabled(false);
            _pUi->actionShowValueTooltip->setEnabled(true);
            _pUi->actionHighlightSamplePoints->setEnabled(true);
            _pUi->actionClearData->setEnabled(true);

            _pStatusRuntime->setText(_cRuntime.arg("0 hours, 0 minutes 0 seconds"));
            _pStatusStats->setText(_cStatsTemplate.arg(0).arg(0));

            // Set to full auto scaling
            _pGuiModel->setxAxisScale(BasicGraphView::SCALE_AUTO);

            // Set to full auto scaling
             _pGuiModel->setyAxisScale(BasicGraphView::SCALE_AUTO);

            parseDataFile(&data);
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

void MainWindow::parseDataFile(DataFileParser::FileData * pData)
{
    // Init graphs, remove first label because is the time scale
    QList<RegisterData> graphList;

    // Include time data (first column)
    for (qint32 i = 0; i < pData->dataLabel.size(); i++)
    {
        RegisterData graph;
        graph.setText(pData->dataLabel[i]);
        graphList.append(graph);
    }

    _pGuiModel->clearGraph();
    _pGuiModel->addGraphs(graphList, pData->dataRows);
    _pGuiModel->setFrontGraph(0);
    // TODO_pGuiModel->setLoadedFile(QFileInfo(_pDataFileParser->getDataParseSettings()->getPath()).fileName());
    _pGuiModel->setWindowTitleDetail(_pGuiModel->loadedFile());
}
