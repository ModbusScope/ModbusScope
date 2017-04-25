#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qcustomplot.h"
#include "communicationmanager.h"
#include "graphdatamodel.h"
#include "graphdata.h"
#include "registerdialog.h"
#include "connectiondialog.h"
#include "settingsmodel.h"
#include "logdialog.h"
#include "aboutdialog.h"
#include "markerinfo.h"
#include "guimodel.h"
#include "extendedgraphview.h"
#include "util.h"

#include <QDebug>
#include <QDateTime>

const QString MainWindow::_cStateRunning = QString("Running");
const QString MainWindow::_cStateStopped = QString("Stopped");
const QString MainWindow::_cStateDataLoaded = QString("Data File loaded");
const QString MainWindow::_cStatsTemplate = QString("Success: %1\tErrors: %2");
const QString MainWindow::_cRuntime = QString("Runtime: %1");

MainWindow::MainWindow(QStringList cmdArguments, QWidget *parent) :
    QMainWindow(parent),
    _pUi(new Ui::MainWindow)
{
    _pUi->setupUi(this);

    _pGuiModel = new GuiModel();

    _pSettingsModel = new SettingsModel();

    _pConnectionDialog = new ConnectionDialog(_pSettingsModel, this);
    _pLogDialog = new LogDialog(_pSettingsModel, _pGuiModel, this);

    _pGraphDataModel = new GraphDataModel();

    _pConnMan = new CommunicationManager(_pSettingsModel, _pGuiModel, _pGraphDataModel);
    _pGraphView = new ExtendedGraphView(_pConnMan, _pGuiModel, _pSettingsModel, _pGraphDataModel, _pUi->customPlot, this);

    _pDataFileExporter = new DataFileExporter(_pGuiModel, _pSettingsModel, _pGraphView, _pGraphDataModel);
    _pProjectFileExporter = new ProjectFileExporter(_pGuiModel, _pSettingsModel, _pGraphDataModel);

    _pLegend = _pUi->legend;
    _pLegend->setModels(_pGuiModel, _pGraphDataModel);
    _pLegend->setGraphview(_pGraphView);

    _pMarkerInfo = _pUi->markerInfo;
    _pMarkerInfo->setModel(_pGuiModel, _pGraphDataModel);

    /*-- Connect menu actions --*/
    connect(_pUi->actionStart, SIGNAL(triggered()), this, SLOT(startScope()));
    connect(_pUi->actionStop, SIGNAL(triggered()), this, SLOT(stopScope()));
    connect(_pUi->actionExit, SIGNAL(triggered()), this, SLOT(exitApplication()));
    connect(_pUi->actionExportDataCsv, SIGNAL(triggered()), this, SLOT(selectDataExportFile()));
    connect(_pUi->actionLoadProjectFile, SIGNAL(triggered()), this, SLOT(selectProjectSettingFile()));
    connect(_pUi->actionReloadProjectFile, SIGNAL(triggered()), this, SLOT(reloadProjectSettings()));
    connect(_pUi->actionImportDataFile, SIGNAL(triggered()), this, SLOT(selecDataImportFile()));
    connect(_pUi->actionExportImage, SIGNAL(triggered()), this, SLOT(selectImageExportFile()));
    connect(_pUi->actionExportSettings, SIGNAL(triggered()), this, SLOT(selectSettingsExportFile()));
    connect(_pUi->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    connect(_pUi->actionHighlightSamplePoints, SIGNAL(toggled(bool)), _pGuiModel, SLOT(setHighlightSamples(bool)));
    connect(_pUi->actionClearData, SIGNAL(triggered()), this, SLOT(clearData()));
    connect(_pUi->actionClearMarkers, SIGNAL(triggered()), _pGuiModel, SLOT(clearMarkersState()));
    connect(_pUi->actionConnectionSettings, SIGNAL(triggered()), this, SLOT(showConnectionDialog()));
    connect(_pUi->actionLogSettings, SIGNAL(triggered()), this, SLOT(showLogDialog()));
    connect(_pUi->actionRegisterSettings, SIGNAL(triggered()), this, SLOT(showRegisterDialog()));

    /*-- connect model to view --*/
    connect(_pGuiModel, SIGNAL(frontGraphChanged()), this, SLOT(updateBringToFrontGrapMenu()));
    connect(_pGuiModel, SIGNAL(frontGraphChanged()), _pGraphView, SLOT(bringToFront()));
    connect(_pGuiModel, SIGNAL(highlightSamplesChanged()), this, SLOT(updateHighlightSampleMenu()));
    connect(_pGuiModel, SIGNAL(highlightSamplesChanged()), _pGraphView, SLOT(enableSamplePoints()));
    connect(_pGuiModel, SIGNAL(cursorValuesChanged()), _pGraphView, SLOT(updateTooltip()));
    connect(_pGuiModel, SIGNAL(cursorValuesChanged()), _pGraphView, SLOT(updateDataInLegend()));

    connect(_pGuiModel, SIGNAL(windowTitleChanged()), this, SLOT(updateWindowTitle()));
    connect(_pGuiModel, SIGNAL(projectFilePathChanged()), this, SLOT(projectFileLoaded()));
    connect(_pGuiModel, SIGNAL(dataFilePathChanged()), this, SLOT(dataFileLoaded()));
    connect(_pGuiModel, SIGNAL(guiStateChanged()), this, SLOT(updateGuiState()));

    connect(_pGuiModel, SIGNAL(xAxisScalingChanged()), this, SLOT(updatexAxisSlidingMode()));
    connect(_pGuiModel, SIGNAL(xAxisScalingChanged()), _pGraphView, SLOT(rescalePlot()));
    connect(_pGuiModel, SIGNAL(xAxisSlidingIntervalChanged()), this, SLOT(updatexAxisSlidingInterval()));
    connect(_pGuiModel, SIGNAL(xAxisSlidingIntervalChanged()), _pGraphView, SLOT(rescalePlot()));

    connect(_pGuiModel, SIGNAL(yAxisScalingChanged()), this, SLOT(updateyAxisSlidingMode()));
    connect(_pGuiModel, SIGNAL(yAxisScalingChanged()), _pGraphView, SLOT(rescalePlot()));
    connect(_pGuiModel, SIGNAL(yAxisMinMaxchanged()), this, SLOT(updateyAxisMinMax()));
    connect(_pGuiModel, SIGNAL(yAxisMinMaxchanged()), _pGraphView, SLOT(rescalePlot()));
    connect(_pGuiModel, SIGNAL(communicationStatsChanged()), this, SLOT(updateStats()));

    connect(_pGuiModel, SIGNAL(markerStateChanged()), _pGraphView, SLOT(updateMarkersVisibility()));
    connect(_pGuiModel, SIGNAL(markerStateChanged()), this, SLOT(updateMarkerDockVisibility()));
    connect(_pGuiModel, SIGNAL(startMarkerPosChanged()), _pGraphView, SLOT(setStartMarker()));
    connect(_pGuiModel, SIGNAL(endMarkerPosChanged()), _pGraphView, SLOT(setEndMarker()));

    connect(_pGraphDataModel, SIGNAL(visibilityChanged(quint32)), this, SLOT(handleGraphVisibilityChange(const quint32)));
    connect(_pGraphDataModel, SIGNAL(visibilityChanged(quint32)), _pGraphView, SLOT(showGraph(const quint32)));
    connect(_pGraphDataModel, SIGNAL(graphsAddData(QList<double>, QList<QList<double> >)), _pGraphView, SLOT(addData(QList<double>, QList<QList<double> >)));
    connect(_pGraphDataModel, SIGNAL(activeChanged(quint32)), this, SLOT(rebuildGraphMenu()));
    connect(_pGraphDataModel, SIGNAL(activeChanged(quint32)), _pGraphView, SLOT(updateGraphs()));
    connect(_pGraphDataModel, SIGNAL(colorChanged(quint32)), this, SLOT(handleGraphColorChange(quint32)));
    connect(_pGraphDataModel, SIGNAL(colorChanged(quint32)), _pGraphView, SLOT(changeGraphColor(quint32)));
    connect(_pGraphDataModel, SIGNAL(labelChanged(quint32)), this, SLOT(handleGraphLabelChange(quint32)));
    connect(_pGraphDataModel, SIGNAL(labelChanged(quint32)), _pGraphView, SLOT(changeGraphLabel(quint32)));

    connect(_pGraphDataModel, SIGNAL(added(quint32)), this, SLOT(rebuildGraphMenu()));
    connect(_pGraphDataModel, SIGNAL(added(quint32)), _pGraphView, SLOT(updateGraphs()));

    connect(_pGraphDataModel, SIGNAL(removed(quint32)), this, SLOT(rebuildGraphMenu()));
    connect(_pGraphDataModel, SIGNAL(removed(quint32)), _pGraphView, SLOT(updateGraphs()));

    connect(_pGraphDataModel, SIGNAL(unsignedChanged(quint32)), _pGraphView, SLOT(clearGraph(quint32)));
    connect(_pGraphDataModel, SIGNAL(multiplyFactorChanged(quint32)), _pGraphView, SLOT(clearGraph(quint32)));
    connect(_pGraphDataModel, SIGNAL(divideFactorChanged(quint32)), _pGraphView, SLOT(clearGraph(quint32)));
    connect(_pGraphDataModel, SIGNAL(registerAddressChanged(quint32)), _pGraphView, SLOT(clearGraph(quint32)));
    connect(_pGraphDataModel, SIGNAL(bitmaskChanged(quint32)), _pGraphView, SLOT(clearGraph(quint32)));
    connect(_pGraphDataModel, SIGNAL(shiftChanged(quint32)), _pGraphView, SLOT(clearGraph(quint32)));

    connect(_pGraphDataModel, SIGNAL(activeChanged(quint32)), _pDataFileExporter, SLOT(rewriteDataFile()));
    connect(_pGraphDataModel, SIGNAL(unsignedChanged(quint32)), _pDataFileExporter, SLOT(rewriteDataFile()));
    connect(_pGraphDataModel, SIGNAL(multiplyFactorChanged(quint32)), _pDataFileExporter, SLOT(rewriteDataFile()));
    connect(_pGraphDataModel, SIGNAL(divideFactorChanged(quint32)), _pDataFileExporter, SLOT(rewriteDataFile()));
    connect(_pGraphDataModel, SIGNAL(registerAddressChanged(quint32)), _pDataFileExporter, SLOT(rewriteDataFile()));
    connect(_pGraphDataModel, SIGNAL(bitmaskChanged(quint32)), _pDataFileExporter, SLOT(rewriteDataFile()));
    connect(_pGraphDataModel, SIGNAL(shiftChanged(quint32)), _pDataFileExporter, SLOT(rewriteDataFile()));
    connect(_pGraphDataModel, SIGNAL(added(quint32)), _pDataFileExporter, SLOT(rewriteDataFile()));
    connect(_pGraphDataModel, SIGNAL(removed(quint32)), _pDataFileExporter, SLOT(rewriteDataFile()));

    // Update cursor values in legend
    connect(_pGraphView, SIGNAL(cursorValueUpdate()), _pLegend, SLOT(updateDataInLegend()));

    _pGraphShowHide = _pUi->menuShowHide;
    _pGraphBringToFront = _pUi->menuBringToFront;
    _pBringToFrontGroup = new QActionGroup(this);

    // Add multipart status bar
    _pStatusState = new QLabel(_cStateStopped, this);
    _pStatusState->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _pStatusStats = new QLabel("", this);
    _pStatusStats->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _pStatusRuntime = new QLabel("", this);
    _pStatusRuntime->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    _pUi->statusBar->addPermanentWidget(_pStatusState, 1);
    _pUi->statusBar->addPermanentWidget(_pStatusRuntime, 2);
    _pUi->statusBar->addPermanentWidget(_pStatusStats, 3);

    this->setAcceptDrops(true);

    // For dock undock
    connect(_pUi->scaleOptionsDock, SIGNAL(topLevelChanged(bool)), this, SLOT(scaleWidgetUndocked(bool)));
    connect(_pUi->legendDock, SIGNAL(topLevelChanged(bool)), this, SLOT(legendWidgetUndocked(bool)));

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
    _pYAxisScaleGroup->addButton(_pUi->radioYWindowScale, BasicGraphView::SCALE_WINDOW_AUTO);
    _pYAxisScaleGroup->addButton(_pUi->radioYMinMax, BasicGraphView::SCALE_MINMAX);
    _pYAxisScaleGroup->addButton(_pUi->radioYManual, BasicGraphView::SCALE_MANUAL);
    connect(_pYAxisScaleGroup, SIGNAL(buttonClicked(int)), this, SLOT(yAxisScaleGroupClicked(int)));

    // Default to full auto scaling
    _pGuiModel->setxAxisScale(BasicGraphView::SCALE_AUTO);
    _pGuiModel->setyAxisScale(BasicGraphView::SCALE_AUTO);

    connect(_pConnMan, SIGNAL(handleReceivedData(QList<bool>, QList<double>)), _pGraphView, SLOT(plotResults(QList<bool>, QList<double>)));
    connect(_pConnMan, SIGNAL(handleReceivedData(QList<bool>, QList<double>)), _pLegend, SLOT(addLastReceivedDataToLegend(QList<bool>, QList<double>)));

    /* Update interface via model */
    _pGuiModel->triggerUpdate();
    _pSettingsModel->triggerUpdate();

    QCommandLineParser parser;
    parser.setApplicationDescription("Log data through the Modbus protocol");
    parser.addHelpOption();
    parser.addPositionalArgument("project file", QCoreApplication::translate("main", "Project file (.mbs) to open"));

    QCommandLineOption openGlOption("opengl", QCoreApplication::translate("main", "Use openGL to render plot"));
    parser.addOption(openGlOption);


    // Process the actual command line arguments given by the user
    parser.process(cmdArguments);

    const QStringList args = parser.positionalArguments();
    // project file is args.at(0)

    bool bOpenGl = parser.isSet(openGlOption);
    _pGraphView->setOpenGl(bOpenGl);

    if (args.size() > 0)
    {
        QFileInfo fileInfo(args[0]);
        _pGuiModel->setLastDir(fileInfo.dir().absolutePath());
        loadProjectFile(args[0]);
    }
}

MainWindow::~MainWindow()
{
    delete _pGraphView;
    delete _pConnectionDialog;
    delete _pConnMan;
    delete _pSettingsModel;
    delete _pGuiModel;
    delete _pGraphShowHide;
    delete _pGraphBringToFront;
    delete _pUi;
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier)
    {
        _pGuiModel->setCursorValues(true);
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    if (!(event->modifiers() & Qt::ControlModifier))
    {
        _pGuiModel->setCursorValues(false);
    }

    QMainWindow::keyReleaseEvent(event);
}

void MainWindow::selectProjectSettingFile()
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

void MainWindow::selecDataImportFile()
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

void MainWindow::selectDataExportFile()
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

void MainWindow::selectSettingsExportFile()
{
    QString filePath;
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setOption(QFileDialog::HideNameFilterDetails, false);
    dialog.setDefaultSuffix("mbs");
    dialog.setWindowTitle(tr("Select mbs file"));
    dialog.setNameFilter(tr("MBS files (*.mbs)"));
    dialog.setDirectory(_pGuiModel->lastDir());

    if (dialog.exec())
    {
        filePath = dialog.selectedFiles().first();
        _pGuiModel->setLastDir(QFileInfo(filePath).dir().absolutePath());
        _pProjectFileExporter->exportProjectFile(filePath);
    }
}

void MainWindow::selectImageExportFile()
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

    /* Add question wether to save when legend is undocked */
    bool bProceed = false;
    if (_pUi->legendDock->isFloating())
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Save screenshot?", "The legend dock is floating, it won't be included in the screenshot. \n\nAre you sure want to proceed?", QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            bProceed = true;
        }
        else
        {
            bProceed = false;
        }
    }
    else
    {
        bProceed = true;
    }

    if (bProceed)
    {
        if (dialog.exec())
        {
            filePath = dialog.selectedFiles().first();
            _pGuiModel->setLastDir(QFileInfo(filePath).dir().absolutePath());

            QPixmap pixMap = this->window()->grab();
            pixMap.save(filePath);
        }
    }
}

void MainWindow::showAbout()
{
    AboutDialog aboutDialog(_pGraphView,this);

    aboutDialog.exec();
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

    const qint32 graphIdx = _pGraphDataModel->convertToGraphIndex(pAction->data().toInt());
    _pGraphDataModel->setVisible(graphIdx, bState);
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
    showRegisterDialog(QString(""));
}

void MainWindow::showRegisterDialog(QString mbcFile)
{
    if (_pGuiModel->guiState() == GuiModel::DATA_LOADED)
    {
        _pGraphDataModel->clear();

        _pGuiModel->setGuiState(GuiModel::INIT);
    }

    RegisterDialog registerDialog(_pGuiModel, _pGraphDataModel, this);

    if (mbcFile.isEmpty())
    {
        registerDialog.exec();
    }
    else
    {
        registerDialog.exec(mbcFile);
    }
}

void MainWindow::clearData()
{
    _pConnMan->resetCommunicationStats();
    _pGraphView->clearResults();
    _pGuiModel->clearMarkersState();
    _pDataFileExporter->rewriteDataFile();
}

void MainWindow::startScope()
{
    if (_pGuiModel->guiState() == GuiModel::DATA_LOADED)
    {
        _pGraphDataModel->clear();

        _pGuiModel->setGuiState(GuiModel::INIT);
    }

    if (_pGraphDataModel->activeCount() != 0)
    {

        _pGuiModel->setGuiState(GuiModel::STARTED);

        _runtimeTimer.singleShot(250, this, SLOT(updateRuntime()));

        if (_pConnMan->startCommunication())
        {
            clearData();
        }

        if (_pSettingsModel->writeDuringLog())
        {
            _pDataFileExporter->enableExporterDuringLog();
        }

        if (_pGuiModel->xAxisScalingMode() == BasicGraphView::SCALE_MANUAL)
        {
            _pGuiModel->setxAxisScale(BasicGraphView::SCALE_AUTO);
        }

        if (_pGuiModel->yAxisScalingMode() == BasicGraphView::SCALE_MANUAL)
        {
            _pGuiModel->setyAxisScale(BasicGraphView::SCALE_AUTO);
        }

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

    _pGuiModel->setGuiState(GuiModel::STOPPED);
}

void MainWindow::handleGraphVisibilityChange(const quint32 graphIdx)
{
    if (_pGraphDataModel->isActive(graphIdx))
    {
        const quint32 activeIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

        _pGraphShowHide->actions().at(activeIdx)->setChecked(_pGraphDataModel->isVisible(graphIdx));

        // Show/Hide corresponding "BringToFront" action
        _pGraphBringToFront->actions().at(activeIdx)->setVisible(_pGraphDataModel->isVisible((graphIdx)));

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
}

void MainWindow::handleGraphColorChange(const quint32 graphIdx)
{
    if (_pGraphDataModel->isActive(graphIdx))
    {
        const quint32 activeIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

        QPixmap pixmap(20,5);
        pixmap.fill(_pGraphDataModel->color(graphIdx));

        QIcon showHideIcon = QIcon(pixmap);

        _pGraphShowHide->actions().at(activeIdx)->setIcon(showHideIcon);
        _pGraphBringToFront->actions().at(activeIdx)->setIcon(showHideIcon);
    }
}

void MainWindow::handleGraphLabelChange(const quint32 graphIdx)
{
    if (_pGraphDataModel->isActive(graphIdx))
    {
        const quint32 activeIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

        _pGraphShowHide->actions().at(activeIdx)->setText(_pGraphDataModel->label(graphIdx));
        _pGraphBringToFront->actions().at(activeIdx)->setText(_pGraphDataModel->label(graphIdx));
    }
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

void MainWindow::rebuildGraphMenu()
{
    // Regenerate graph menu
    _pGraphShowHide->clear();
    _pGraphBringToFront->clear();

    QList<quint16> activeGraphList;
    _pGraphDataModel->activeGraphIndexList(&activeGraphList);

    for(qint32 activeIdx = 0; activeIdx < activeGraphList.size(); activeIdx++)
    {

        QString label = _pGraphDataModel->label(activeGraphList[activeIdx]);
        QAction * pShowHideAction = _pGraphShowHide->addAction(label);
        QAction * pBringToFront = _pGraphBringToFront->addAction(label);

        QPixmap pixmap(20,5);
        pixmap.fill(_pGraphDataModel->color(activeGraphList[activeIdx]));
        QIcon icon = QIcon(pixmap);

        pShowHideAction->setData(activeIdx);
        pShowHideAction->setIcon(icon);
        pShowHideAction->setCheckable(true);
        pShowHideAction->setChecked(_pGraphDataModel->isVisible(activeGraphList[activeIdx]));

        pBringToFront->setData(activeIdx);
        pBringToFront->setIcon(icon);
        pBringToFront->setCheckable(true);
        pBringToFront->setActionGroup(_pBringToFrontGroup);

        QObject::connect(pShowHideAction, SIGNAL(toggled(bool)), this, SLOT(menuShowHideGraphClicked(bool)));
        QObject::connect(pBringToFront, SIGNAL(toggled(bool)), this, SLOT(menuBringToFrontGraphClicked(bool)));
    }

    if (activeGraphList.size() > 0)
    {
        _pGraphShowHide->setEnabled(true);
        _pGraphBringToFront->setEnabled(true);
    }
    else
    {
        _pGraphShowHide->setEnabled(false);
        _pGraphBringToFront->setEnabled(false);
    }
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
    else if (_pGuiModel->yAxisScalingMode() == BasicGraphView::SCALE_WINDOW_AUTO)
    {
        // Window auto scale selected
        _pUi->radioYWindowScale->setChecked(true);
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

void MainWindow::updateGuiState()
{

    if (_pGuiModel->guiState() == GuiModel::INIT)
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
        _pUi->actionExportSettings->setEnabled(true);

        _pStatusRuntime->setText(_cRuntime.arg("0 hours, 0 minutes 0 seconds"));
        _pStatusRuntime->setVisible(true);

        _pStatusStats->setText(_cStatsTemplate.arg(0).arg(0));
        _pStatusRuntime->setVisible(true);

        _pGuiModel->setDataFilePath(QString(""));
        _pGuiModel->setProjectFilePath(QString(""));
    }
    else if (_pGuiModel->guiState() == GuiModel::STARTED)
    {
        // Communication active
        _pStatusState->setText(_cStateRunning);

        _pUi->actionStop->setEnabled(true);
        _pUi->actionConnectionSettings->setEnabled(false);
        _pUi->actionLogSettings->setEnabled(false);
        _pUi->actionRegisterSettings->setEnabled(false);
        _pUi->actionStart->setEnabled(false);
        _pUi->actionImportDataFile->setEnabled(false);
        _pUi->actionLoadProjectFile->setEnabled(false);
        _pUi->actionExportDataCsv->setEnabled(false);
        _pUi->actionExportSettings->setEnabled(false);
        _pUi->actionExportImage->setEnabled(false);
        _pUi->actionReloadProjectFile->setEnabled(false);

        _pStatusRuntime->setText(_cRuntime.arg("0 hours, 0 minutes 0 seconds"));
        _pStatusRuntime->setVisible(true);

        _pStatusStats->setText(_cStatsTemplate.arg(_pGuiModel->communicationSuccessCount()).arg(_pGuiModel->communicationErrorCount()));
        _pStatusRuntime->setVisible(true);
    }
    else if (_pGuiModel->guiState() == GuiModel::STOPPED)
    {
        _pStatusState->setText(_cStateStopped);

        // Communication not active
        _pUi->actionStop->setEnabled(false);
        _pUi->actionConnectionSettings->setEnabled(true);
        _pUi->actionLogSettings->setEnabled(true);
        _pUi->actionRegisterSettings->setEnabled(true);
        _pUi->actionStart->setEnabled(true);
        _pUi->actionImportDataFile->setEnabled(true);
        _pUi->actionLoadProjectFile->setEnabled(true);
        _pUi->actionExportDataCsv->setEnabled(true);
        _pUi->actionExportSettings->setEnabled(true);
        _pUi->actionExportImage->setEnabled(true);

        if (_pGuiModel->projectFilePath().isEmpty())
        {
            _pUi->actionReloadProjectFile->setEnabled(false);
        }
        else
        {
            _pUi->actionReloadProjectFile->setEnabled(true);
        }
    }
    else if (_pGuiModel->guiState() == GuiModel::DATA_LOADED)
    {

        _pStatusState->setText(_cStateDataLoaded);

        // Communication not active
        _pUi->actionStop->setEnabled(false);
        _pUi->actionConnectionSettings->setEnabled(true);
        _pUi->actionLogSettings->setEnabled(true);
        _pUi->actionRegisterSettings->setEnabled(true);
        _pUi->actionStart->setEnabled(true);
        _pUi->actionImportDataFile->setEnabled(true);
        _pUi->actionLoadProjectFile->setEnabled(true);
        _pUi->actionExportDataCsv->setEnabled(false); // Can't export data when viewing data
        _pUi->actionExportSettings->setEnabled(false); // Can't export data when viewing data
        _pUi->actionExportImage->setEnabled(true);

        _pStatusRuntime->setText(QString(""));
        _pStatusRuntime->setVisible(false);

        _pStatusStats->setText(QString(""));
        _pStatusStats->setVisible(false);

        _pUi->actionReloadProjectFile->setEnabled(false);
    }
}

void MainWindow::projectFileLoaded()
{
    // if a project file is previously loaded, then it can be reloaded
    if (_pGuiModel->projectFilePath().isEmpty())
    {
        _pGuiModel->setWindowTitleDetail("");
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
    if (_pGuiModel->dataFilePath().isEmpty())
    {
        _pGuiModel->setWindowTitleDetail("");
    }
    else
    {
        _pGuiModel->setWindowTitleDetail(QFileInfo(_pGuiModel->dataFilePath()).fileName());
    }
}

void MainWindow::updateStats()
{
    // Update statistics
    _pStatusStats->setText(_cStatsTemplate.arg(_pGuiModel->communicationSuccessCount()).arg(_pGuiModel->communicationErrorCount()));
}

void MainWindow::updateMarkerDockVisibility()
{
    if (_pGuiModel->markerState())
    {
        splitDockWidget(_pUi->legendDock, _pUi->markerInfoDock, Qt::Vertical);
    }

    _pUi->markerInfoDock->setVisible(_pGuiModel->markerState());
}

void MainWindow::scaleWidgetUndocked(bool bFloat)
{
    if (bFloat)
    {
        _pUi->scaleOptionsDock->adjustSize();
    }
}

void MainWindow::legendWidgetUndocked(bool bFloat)
{
    if (bFloat)
    {
        // TODO: fix scaling to minimum
        //_pUi->legendDock->adjustSize();
    }
}

void MainWindow::showContextMenu(const QPoint& pos)
{
    /* Don't show context menu when control key is pressed */
    if (!(QApplication::keyboardModifiers() & Qt::ControlModifier))
    {
        _pUi->menuView->popup(_pUi->customPlot->mapToGlobal(pos));
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
    if (!_pConnMan->isActive())
    {
        const QString filename(e->mimeData()->urls().last().toLocalFile());
        QFileInfo fileInfo(filename);
        _pGuiModel->setLastDir(fileInfo.dir().absolutePath());
        if (fileInfo.completeSuffix().toLower() == QString("mbs"))
        {
            loadProjectFile(filename);
        }
        else if (fileInfo.completeSuffix().toLower() == QString("csv"))
        {
            loadDataFile(filename);
        }
        else if (fileInfo.completeSuffix().toLower() == QString("mbc"))
        {
            showRegisterDialog(filename);
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
    if (pProjectSettings->general.connectionSettings.bIp)
    {
        _pSettingsModel->setIpAddress(pProjectSettings->general.connectionSettings.ip);
    }

    if (pProjectSettings->general.connectionSettings.bPort)
    {
         _pSettingsModel->setPort(pProjectSettings->general.connectionSettings.port);
    }

    if (pProjectSettings->general.connectionSettings.bSlaveId)
    {
        _pSettingsModel->setSlaveId(pProjectSettings->general.connectionSettings.slaveId);
    }

    if (pProjectSettings->general.connectionSettings.bTimeout)
    {
        _pSettingsModel->setTimeout(pProjectSettings->general.connectionSettings.timeout);
    }

    if (pProjectSettings->general.connectionSettings.bConsecutiveMax)
    {
        _pSettingsModel->setConsecutiveMax(pProjectSettings->general.connectionSettings.consecutiveMax);
    }

    if (pProjectSettings->general.logSettings.bPollTime)
    {
        _pSettingsModel->setPollTime(pProjectSettings->general.logSettings.pollTime);
    }

    _pSettingsModel->setAbsoluteTimes(pProjectSettings->general.logSettings.bAbsoluteTimes);

    _pSettingsModel->setWriteDuringLog(pProjectSettings->general.logSettings.bLogToFile);
    if (pProjectSettings->general.logSettings.bLogToFileFile)
    {
        _pSettingsModel->setWriteDuringLogFile(pProjectSettings->general.logSettings.logFile);
    }
    else
    {
         _pSettingsModel->setWriteDuringLogFileToDefault();
    }

    if (pProjectSettings->view.scaleSettings.bSliding)
    {
        _pGuiModel->setxAxisSlidingInterval(pProjectSettings->view.scaleSettings.slidingInterval);
        _pGuiModel->setxAxisScale(BasicGraphView::SCALE_SLIDING);
    }
    else
    {
        _pGuiModel->setxAxisScale(BasicGraphView::SCALE_AUTO);
    }

    if (pProjectSettings->view.scaleSettings.bMinMax)
    {
        _pGuiModel->setyAxisMin(pProjectSettings->view.scaleSettings.scaleMin);
        _pGuiModel->setyAxisMax(pProjectSettings->view.scaleSettings.scaleMax);
        _pGuiModel->setyAxisScale(BasicGraphView::SCALE_MINMAX);
    }
    else if (pProjectSettings->view.scaleSettings.bWindowScale)
    {
        _pGuiModel->setyAxisScale(BasicGraphView::SCALE_WINDOW_AUTO);
    }
    else
    {
        _pGuiModel->setyAxisScale(BasicGraphView::SCALE_AUTO);
    }

    _pGraphDataModel->clear();
    for (qint32 i = 0; i < pProjectSettings->scope.registerList.size(); i++)
    {
        GraphData rowData;
        rowData.setActive(pProjectSettings->scope.registerList[i].bActive);
        rowData.setUnsigned(pProjectSettings->scope.registerList[i].bUnsigned);
        rowData.setRegisterAddress(pProjectSettings->scope.registerList[i].address);
        rowData.setBitmask(pProjectSettings->scope.registerList[i].bitmask);
        rowData.setLabel(pProjectSettings->scope.registerList[i].text);
        rowData.setDivideFactor(pProjectSettings->scope.registerList[i].divideFactor);
        rowData.setMultiplyFactor(pProjectSettings->scope.registerList[i].multiplyFactor);
        rowData.setColor(pProjectSettings->scope.registerList[i].color);
        rowData.setShift(pProjectSettings->scope.registerList[i].shift);

        _pGraphDataModel->add(rowData);
    }

    _pGuiModel->setFrontGraph(-1);
}

void MainWindow::loadProjectFile(QString projectFilePath)
{
    ProjectFileParser fileParser;
    ProjectFileParser::ProjectSettings loadedSettings;
    QFile file(projectFilePath);

    /* If we can't open it, let's show an error message. */
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (fileParser.parseFile(&file, &loadedSettings))
        {
            updateConnectionSetting(&loadedSettings);

            _pGuiModel->setDataFilePath("");
            _pGuiModel->setProjectFilePath(projectFilePath);
            _pGuiModel->setGuiState(GuiModel::STOPPED);
        }
    }
    else
    {
        QMessageBox::critical(this,
                              "ModbusScope",
                              tr("Couldn't open project file: %1").arg(projectFilePath),
                              QMessageBox::Ok);
    }
}

void MainWindow::loadDataFile(QString dataFilePath)
{
    DataFileParser dataParser;

    DataFileParser::FileData data;
    if (dataParser.processDataFile(dataFilePath, &data))
    {
        // Set to full auto scaling
        _pGuiModel->setxAxisScale(BasicGraphView::SCALE_AUTO);

        // Set to full auto scaling
        _pGuiModel->setyAxisScale(BasicGraphView::SCALE_AUTO);

        _pGraphDataModel->clear();
        _pGuiModel->setFrontGraph(-1);

        _pGraphDataModel->add(data.dataLabel, data.timeRow, data.dataRows);

        if (!data.colors.isEmpty())
        {
            for (int idx = 0; idx < data.dataLabel.size(); idx++)
            {
                _pGraphDataModel->setColor((quint32)idx, data.colors[idx]);
            }
        }

        _pGuiModel->setFrontGraph(0);

        _pGuiModel->setProjectFilePath("");
        _pGuiModel->setDataFilePath(dataFilePath);

        _pGuiModel->setGuiState(GuiModel::DATA_LOADED);

    }
}
