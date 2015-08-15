#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qcustomplot.h"
#include "communicationmanager.h"
#include "registerdata.h"
#include "registermodel.h"
#include "registerdialog.h"
#include "connectiondialog.h"
#include "settingsmodel.h"
#include "logdialog.h"

#include "guimodel.h"
#include "extendedgraphview.h"
#include "util.h"

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

    _pGuiModel = new GuiModel();

    _pSettingsModel = new SettingsModel();

    _pConnectionDialog = new ConnectionDialog(_pSettingsModel);
    _pLogDialog = new LogDialog(_pSettingsModel, _pGuiModel);

    _pRegisterModel = new RegisterModel();
    _pRegisterDialog = new RegisterDialog(_pRegisterModel);

    _pConnMan = new CommunicationManager(_pSettingsModel, _pGuiModel, _pSettingsModel);
    _pGraphView = new ExtendedGraphView(_pConnMan, _pGuiModel, _pSettingsModel, _pUi->customPlot, this);

    _pDataFileExporter = new DataFileExporter(_pGuiModel, _pSettingsModel, _pGraphView);

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
    connect(_pUi->actionLogSettings, SIGNAL(triggered()), this, SLOT(showLogDialog()));
    connect(_pUi->actionRegisterSettings, SIGNAL(triggered()), this, SLOT(showRegisterDialog()));
    connect(_pUi->actionShowLegend, SIGNAL(triggered(bool)), _pGuiModel, SLOT(setLegendVisibility(bool)));

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
    connect(_pGuiModel, SIGNAL(graphsAddData(QList<double>, QList<QList<double> >)), _pGraphView, SLOT(addData(QList<double>, QList<QList<double> >)));
    connect(_pGuiModel, SIGNAL(graphsAdded()), this, SLOT(addGraphMenu()));
    connect(_pGuiModel, SIGNAL(windowTitleChanged()), this, SLOT(updateWindowTitle()));
    connect(_pGuiModel, SIGNAL(communicationStateChanged()), this, SLOT(updateCommunicationState()));
    connect(_pGuiModel, SIGNAL(projectFilePathChanged()), this, SLOT(projectFileLoaded()));
    connect(_pGuiModel, SIGNAL(dataFilePathChanged()), this, SLOT(dataFileLoaded()));
    connect(_pGuiModel, SIGNAL(legendPositionChanged()), this, SLOT(updateLegendPositionMenu()));
    connect(_pGuiModel, SIGNAL(legendPositionChanged()), _pGraphView, SLOT(updateLegendPosition()));
    connect(_pGuiModel, SIGNAL(legendVisibilityChanged()), this, SLOT(updateLegendMenu()));
    connect(_pGuiModel, SIGNAL(legendVisibilityChanged()), _pGraphView, SLOT(showHideLegend()));

    connect(_pGuiModel, SIGNAL(xAxisScalingChanged()), this, SLOT(updatexAxisSlidingMode()));
    connect(_pGuiModel, SIGNAL(xAxisScalingChanged()), _pGraphView, SLOT(rescalePlot()));
    connect(_pGuiModel, SIGNAL(xAxisSlidingIntervalChanged()), this, SLOT(updatexAxisSlidingInterval()));
    connect(_pGuiModel, SIGNAL(xAxisSlidingIntervalChanged()), _pGraphView, SLOT(rescalePlot()));

    connect(_pGuiModel, SIGNAL(yAxisScalingChanged()), this, SLOT(updateyAxisSlidingMode()));
    connect(_pGuiModel, SIGNAL(yAxisScalingChanged()), _pGraphView, SLOT(rescalePlot()));
    connect(_pGuiModel, SIGNAL(yAxisMinMaxchanged()), this, SLOT(updateyAxisMinMax()));
    connect(_pGuiModel, SIGNAL(yAxisMinMaxchanged()), _pGraphView, SLOT(rescalePlot()));
    connect(_pGuiModel, SIGNAL(communicationStatsChanged()), this, SLOT(updateStats()));

    _pGraphShowHide = _pUi->menuShowHide;
    _pGraphBringToFront = _pUi->menuBringToFront;
    _pBringToFrontGroup = new QActionGroup(this);

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

    this->setAcceptDrops(true);

    // For scale dock undock
    connect(_pUi->scaleOptionsDock, SIGNAL(topLevelChanged(bool)), this, SLOT(scaleWidgetUndocked(bool)));

    // For rightclick menu
    _pUi->customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_pUi->customPlot, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));

    connect(_pUi->spinSlidingXInterval, SIGNAL(valueChanged(int)), _pGuiModel, SLOT(setxAxisSlidingInterval(qint32)));
    connect(_pUi->spinYMin, SIGNAL(valueChanged(int)), _pGuiModel, SLOT(setyAxisMin(qint32)));
    connect(_pUi->spinYMax, SIGNAL(valueChanged(int)), _pGuiModel, SLOT(setyAxisMax(qint32)));

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
    connect(_pXAxisScaleGroup, SIGNAL(buttonClicked(int)), this, SLOT(xAxisScaleGroupClicked(int)));

    // Create button group for Y axis scaling options
    _pYAxisScaleGroup = new QButtonGroup();
    _pYAxisScaleGroup->setExclusive(true);
    _pYAxisScaleGroup->addButton(_pUi->radioYFullScale, BasicGraphView::SCALE_AUTO);
    _pYAxisScaleGroup->addButton(_pUi->radioYMinMax, BasicGraphView::SCALE_MINMAX);
    _pYAxisScaleGroup->addButton(_pUi->radioYManual, BasicGraphView::SCALE_MANUAL);
    connect(_pYAxisScaleGroup, SIGNAL(buttonClicked(int)), this, SLOT(yAxisScaleGroupClicked(int)));

    // Default to full auto scaling
    _pGuiModel->setxAxisScale(BasicGraphView::SCALE_AUTO);
    _pGuiModel->setyAxisScale(BasicGraphView::SCALE_AUTO);

    connect(_pConnMan, SIGNAL(handleReceivedData(QList<bool>, QList<double>)), _pGraphView, SLOT(plotResults(QList<bool>, QList<double>)));

    /* Update interface via model */
    _pGuiModel->triggerUpdate();
    _pSettingsModel->triggerUpdate();

    if (cmdArguments.size() > 1)
    {
        loadProjectFile(cmdArguments[1]);
    }
}

MainWindow::~MainWindow()
{
    delete _pGraphView;
    delete _pConnectionDialog;
    delete _pRegisterDialog;
    delete _pConnMan;
    delete _pSettingsModel;
    delete _pGuiModel;
    delete _pGraphShowHide;
    delete _pGraphBringToFront;
    delete _pUi;
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
    dialog.setDirectory(_pGuiModel->lastDir());

    if (dialog.exec())
    {
        filePath = dialog.selectedFiles().first();
        _pGuiModel->setLastDir(QFileInfo(filePath).dir().absolutePath());
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
    dialog.setDirectory(_pGuiModel->lastDir());

    if (dialog.exec())
    {
        filePath = dialog.selectedFiles().first();
        _pGuiModel->setLastDir(QFileInfo(filePath).dir().absolutePath());
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
    dialog.setDirectory(_pGuiModel->lastDir());

    if (dialog.exec())
    {
        filePath = dialog.selectedFiles().first();
        _pGuiModel->setLastDir(QFileInfo(filePath).dir().absolutePath());
        _pDataFileExporter->exportDataFile(filePath);
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
    dialog.setDirectory(_pGuiModel->lastDir());

    if (dialog.exec())
    {
        filePath = dialog.selectedFiles().first();
        _pGuiModel->setLastDir(QFileInfo(filePath).dir().absolutePath());
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

    QString appVersion = QString(tr("v%1")).arg(APP_VERSION);

#ifdef DEBUG
    appVersion.append(QString(tr(" (git: %1:%2)")).arg(GIT_BRANCH).arg(GIT_HASH));
#endif

    QString version = QString(tr("<b>GraphViewer %1</b><br><br>")).arg(appVersion);

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

void MainWindow::showLogDialog()
{
    _pLogDialog->exec();
}

void MainWindow::showRegisterDialog()
{
    _pRegisterDialog->exec();
    _pGuiModel->setGraphReset(true);
}

void MainWindow::changeLegendPosition(QAction* pAction)
{
    if (pAction == _pUi->actionLegendLeft)
    {
        _pGuiModel->setLegendPosition(BasicGraphView::LEGEND_LEFT);
    }
    else if (pAction == _pUi->actionLegendMiddle)
    {
        _pGuiModel->setLegendPosition(BasicGraphView::LEGEND_MIDDLE);
    }
    else if (pAction == _pUi->actionLegendRight)
    {
        _pGuiModel->setLegendPosition(BasicGraphView::LEGEND_RIGHT);
    }
}

void MainWindow::clearData()
{
    _pConnMan->resetCommunicationStats();
    _pGraphView->clearResults();
}

void MainWindow::startScope()
{
    if (_pRegisterModel->checkedRegisterCount() != 0)
    {

        _pGuiModel->setCommunicationState(GuiModel::STARTED);

        _pStatusRuntime->setText(_cRuntime.arg("0 hours, 0 minutes 0 seconds"));

        _runtimeTimer.singleShot(250, this, SLOT(updateRuntime()));

        QList<RegisterData> regList;
        _pRegisterModel->checkedRegisterList(&regList);

        if (_pConnMan->startCommunication(regList))
        {
            if (_pGuiModel->graphReset())
            {
                QList<RegisterData> regList;
                _pRegisterModel->checkedRegisterList(&regList);

                _pGuiModel->clearGraph();
                _pGuiModel->addGraphs(regList);

                _pGuiModel->setGraphReset(false);
            }
            else
            {
                clearData();
            }
        }

        if (_pSettingsModel->writeDuringLog())
        {
            _pDataFileExporter->enableExporterDuringLog();
        }

        if (_pGuiModel->xAxisScalingMode() == BasicGraphView::SCALE_MANUAL)
        {
            _pGuiModel->setxAxisScale(BasicGraphView::SCALE_AUTO); // TODO: use scaling settings from project file?
        }

        if (_pGuiModel->yAxisScalingMode() == BasicGraphView::SCALE_MANUAL)
        {
            _pGuiModel->setyAxisScale(BasicGraphView::SCALE_AUTO); // TODO: use scaling settings from project file?
        }

        _pGuiModel->setLegendVisibility(true);

    }
    else
    {
        QMessageBox::warning(this, "No register in scope list!", "There are no register in the scope list. Please select at least one register.");
    }
}

void MainWindow::stopScope()
{
    _pConnMan->stopCommunication();

    if (_pSettingsModel->writeDuringLog())
    {
        _pDataFileExporter->disableExporterDuringLog();
    }

    _pGuiModel->setCommunicationState(GuiModel::STOPPED);
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

    _pUi->actionShowValueTooltip->setEnabled(true);
    _pUi->actionHighlightSamplePoints->setEnabled(true);
    _pUi->actionClearData->setEnabled(true);
    _pUi->menuLegend->setEnabled(true);

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

void MainWindow::updateCommunicationState()
{

    if (_pGuiModel->communicationState() == GuiModel::INIT)
    {
        _pStatusState->setText(_cStateStopped);

        _pUi->actionStop->setEnabled(false);
        _pUi->actionConnectionSettings->setEnabled(true);
        _pUi->actionLogSettings->setEnabled(true);
        _pUi->actionRegisterSettings->setEnabled(true);
        _pUi->actionStart->setEnabled(true);
        _pUi->actionImportDataFile->setEnabled(true);
        _pUi->actionLoadProjectFile->setEnabled(true);
        _pUi->actionExportDataCsv->setEnabled(false);
        _pUi->actionExportImage->setEnabled(false);

    }
    else if (_pGuiModel->communicationState() == GuiModel::STARTED)
    {
        // Communication active
        _pUi->actionStop->setEnabled(true);
        _pUi->actionConnectionSettings->setEnabled(false);
        _pUi->actionLogSettings->setEnabled(false);
        _pUi->actionRegisterSettings->setEnabled(false);
        _pUi->actionStart->setEnabled(false);
        _pUi->actionImportDataFile->setEnabled(false);
        _pUi->actionLoadProjectFile->setEnabled(false);
        _pUi->actionExportDataCsv->setEnabled(false);
        _pUi->actionExportImage->setEnabled(false);

        _pStatusState->setText(_cStateRunning);

        if (_pGuiModel->projectFilePath().isEmpty())
        {
            _pGuiModel->setWindowTitleDetail("");
        }
    }
    else if (_pGuiModel->communicationState() == GuiModel::STOPPED)
    {
        // Communication not active
        _pUi->actionStop->setEnabled(false);
        _pUi->actionConnectionSettings->setEnabled(true);
        _pUi->actionLogSettings->setEnabled(true);
        _pUi->actionRegisterSettings->setEnabled(true);
        _pUi->actionStart->setEnabled(true);
        _pUi->actionImportDataFile->setEnabled(true);
        _pUi->actionLoadProjectFile->setEnabled(true);
        _pUi->actionExportDataCsv->setEnabled(true);
        _pUi->actionExportImage->setEnabled(true);

        _pStatusState->setText(_cStateStopped);
    }
}

void MainWindow::projectFileLoaded()
{
    // if a project file is previously loaded, then it can be reloaded
    if (_pGuiModel->projectFilePath().isEmpty())
    {
        _pUi->actionReloadProjectFile->setEnabled(false);
    }
    else
    {
        _pGuiModel->setWindowTitleDetail(QFileInfo(_pGuiModel->projectFilePath()).fileName());
        _pUi->actionReloadProjectFile->setEnabled(true);
    }
}

void MainWindow::dataFileLoaded()
{
    // if a project file is previously loaded, then it can be reloaded
    if (!_pGuiModel->dataFilePath().isEmpty())
    {
        _pGuiModel->setWindowTitleDetail(QFileInfo(_pGuiModel->dataFilePath()).fileName());
        _pUi->actionReloadProjectFile->setEnabled(false);
        _pUi->actionExportDataCsv->setEnabled(false);
    }
}

void MainWindow::updateLegendPositionMenu()
{
    if (_pGuiModel->legendPosition() == BasicGraphView::LEGEND_LEFT)
    {
        _pUi->actionLegendLeft->setChecked(true);
    }
    else if (_pGuiModel->legendPosition() == BasicGraphView::LEGEND_MIDDLE)
    {
        _pUi->actionLegendMiddle->setChecked(true);
    }
    else if (_pGuiModel->legendPosition() == BasicGraphView::LEGEND_RIGHT)
    {
        _pUi->actionLegendRight->setChecked(true);
    }
}

void MainWindow::updateLegendMenu()
{
    _pUi->actionShowLegend->setChecked(_pGuiModel->legendVisibility());

    _pUi->actionLegendLeft->setEnabled(_pGuiModel->legendVisibility());
    _pUi->actionLegendMiddle->setEnabled(_pGuiModel->legendVisibility());
    _pUi->actionLegendRight->setEnabled(_pGuiModel->legendVisibility());
}

void MainWindow::updateStats()
{
    // Update statistics
    _pStatusStats->setText(_cStatsTemplate.arg(_pGuiModel->communicationSuccessCount()).arg(_pGuiModel->communicationErrorCount()));
}

void MainWindow::scaleWidgetUndocked(bool bFloat)
{
    if (bFloat)
    {
        _pUi->scaleOptionsDock->adjustSize();
    }
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
    if (!_pConnMan->isActive())
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

void MainWindow::xAxisScaleGroupClicked(int id)
{
    _pGuiModel->setxAxisScale((BasicGraphView::AxisScaleOptions)id);
}

void MainWindow::yAxisScaleGroupClicked(int id)
{
    _pGuiModel->setyAxisScale((BasicGraphView::AxisScaleOptions)id) ;
}

void MainWindow::updateRuntime()
{
    qint64 timePassed = QDateTime::currentMSecsSinceEpoch() - _pGuiModel->communicationStartTime();

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
    if (_pConnMan->isActive())
    {
        _runtimeTimer.singleShot(250, this, SLOT(updateRuntime()));
    }
}

void MainWindow::updateConnectionSetting(ProjectFileParser::ProjectSettings * pProjectSettings)
{
    if (pProjectSettings->general.bIp)
    {
        _pSettingsModel->setIpAddress(pProjectSettings->general.ip);
    }

    if (pProjectSettings->general.bPort)
    {
         _pSettingsModel->setPort(pProjectSettings->general.port);
    }

    if (pProjectSettings->general.bPollTime)
    {
        _pSettingsModel->setPollTime(pProjectSettings->general.pollTime);
    }

    if (pProjectSettings->general.bSlaveId)
    {
        _pSettingsModel->setSlaveId(pProjectSettings->general.slaveId);
    }

    if (pProjectSettings->general.bTimeout)
    {
        _pSettingsModel->setTimeout(pProjectSettings->general.timeout);
    }

    if (pProjectSettings->general.bConsecutiveMax)
    {
        _pSettingsModel->setConsecutiveMax(pProjectSettings->general.consecutiveMax);
    }

    if (pProjectSettings->scale.bSliding)
    {
        _pGuiModel->setxAxisSlidingInterval(pProjectSettings->scale.slidingInterval);
        _pGuiModel->setxAxisScale(BasicGraphView::SCALE_SLIDING);
    }
    else
    {
        _pGuiModel->setxAxisScale(BasicGraphView::SCALE_AUTO);
    }

    if (pProjectSettings->scale.bMinMax)
    {
        _pGuiModel->setyAxisMin(pProjectSettings->scale.scaleMin);
        _pGuiModel->setyAxisMax(pProjectSettings->scale.scaleMax);
        _pGuiModel->setyAxisScale(BasicGraphView::SCALE_MINMAX);
    }
    else
    {
        _pGuiModel->setyAxisScale(BasicGraphView::SCALE_AUTO);
    }

    if (pProjectSettings->legend.bLegendPosition)
    {
        if (pProjectSettings->legend.legendPosition == 0)
        {
            _pGuiModel->setLegendPosition(BasicGraphView::LEGEND_LEFT);
        }
        else if (pProjectSettings->legend.legendPosition == 1)
        {
            _pGuiModel->setLegendPosition(BasicGraphView::LEGEND_MIDDLE);
        }
        else if (pProjectSettings->legend.legendPosition == 2)
        {
            _pGuiModel->setLegendPosition(BasicGraphView::LEGEND_RIGHT);
        }

    }

    _pRegisterModel->clear();
    for (qint32 i = 0; i < pProjectSettings->scope.registerList.size(); i++)
    {
        RegisterData rowData;
        rowData.setActive(pProjectSettings->scope.registerList[i].bActive);
        rowData.setUnsigned(pProjectSettings->scope.registerList[i].bUnsigned);
        rowData.setRegisterAddress(pProjectSettings->scope.registerList[i].address);
        rowData.setBitmask(pProjectSettings->scope.registerList[i].bitmask);
        rowData.setText(pProjectSettings->scope.registerList[i].text);
        rowData.setScaleFactor(pProjectSettings->scope.registerList[i].scaleFactor);
        rowData.setColor(pProjectSettings->scope.registerList[i].color);
        rowData.setShift(pProjectSettings->scope.registerList[i].shift);

        _pRegisterModel->appendRow(rowData);
    }

    _pGuiModel->setGraphReset(true);
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

    _pGuiModel->setDataFilePath(dataFilePath);

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
    QList<RegisterData> graphList;

    for (qint32 i = 0; i < pData->dataLabel.size(); i++)
    {
        RegisterData graph;
        graph.setText(pData->dataLabel[i]);
        graphList.append(graph);
    }

    _pGuiModel->clearGraph();
    _pGuiModel->addGraphs(graphList, pData->timeRow, pData->dataRows);
    _pGuiModel->setFrontGraph(0);

    // Show legend
    _pGuiModel->setLegendVisibility(true);
}
