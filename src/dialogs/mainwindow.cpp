#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qcustomplot.h"
#include "registervaluehandler.h"
#include "graphdatahandler.h"
#include "modbuspoll.h"
#include "graphdatamodel.h"
#include "notemodel.h"
#include "diagnosticmodel.h"
#include "graphdata.h"
#include "registerdialog.h"
#include "connectiondialog.h"
#include "notesdock.h"
#include "settingsmodel.h"
#include "dataparsermodel.h"
#include "logdialog.h"
#include "diagnosticdialog.h"
#include "aboutdialog.h"
#include "markerinfo.h"
#include "guimodel.h"
#include "graphview.h"
#include "datafilehandler.h"
#include "projectfilehandler.h"
#include "util.h"
#include "versiondownloader.h"
#include "fileselectionhelper.h"

#include <QDateTime>

const QString MainWindow::_cStateRunning = QString("Running");
const QString MainWindow::_cStateStopped = QString("Stopped");
const QString MainWindow::_cStateDataLoaded = QString("Data File loaded");
const QString MainWindow::_cStatsTemplate = QString("Success: %1\tErrors: %2");
const QString MainWindow::_cRuntime = QString("Runtime: %1");

MainWindow::MainWindow(QStringList cmdArguments, GuiModel* pGuiModel,
                       SettingsModel* pSettingsModel, GraphDataModel* pGraphDataModel,
                       NoteModel* pNoteModel, DiagnosticModel* pDiagnosticModel,
                       DataParserModel* pDataParserModel, QWidget *parent) :
    QMainWindow(parent),
    _pUi(new Ui::MainWindow),
    _pGuiModel(pGuiModel),
    _pSettingsModel(pSettingsModel),
    _pGraphDataModel(pGraphDataModel),
    _pNoteModel(pNoteModel),
    _pDiagnosticModel(pDiagnosticModel),
    _pDataParserModel(pDataParserModel)
{
    _pUi->setupUi(this);

    _pConnectionDialog = new ConnectionDialog(_pSettingsModel, this);
    _pLogDialog = new LogDialog(_pSettingsModel, _pGuiModel, this);
    _pDiagnosticDialog = new DiagnosticDialog(_pGuiModel, _pDiagnosticModel, this);

    _pNotesDock = new NotesDock(_pNoteModel, _pGuiModel, this);

    _pGraphDataHandler = new GraphDataHandler();
    _pModbusPoll = new ModbusPoll(_pSettingsModel);
    connect(_pModbusPoll, &ModbusPoll::registerDataReady, _pGraphDataHandler, &GraphDataHandler::handleRegisterData);

    _pGraphView = new GraphView(_pGuiModel, _pSettingsModel, _pGraphDataModel, _pNoteModel, _pUi->customPlot, this);
    _pDataFileHandler = new DataFileHandler(_pGuiModel, _pGraphDataModel, _pNoteModel, _pSettingsModel, _pDataParserModel, this);
    _pProjectFileHandler = new ProjectFileHandler(_pGuiModel, _pSettingsModel, _pGraphDataModel);

    _pLegend = _pUi->legend;
    _pLegend->setModels(_pGuiModel, _pGraphDataModel);
    _pLegend->setGraphview(_pGraphView);

    _pMarkerInfo = _pUi->markerInfo;
    _pMarkerInfo->setModel(_pGuiModel, _pGraphDataModel);

    /*-- Connect menu actions --*/
    connect(_pUi->actionStart, &QAction::triggered, this, &MainWindow::startScope);
    connect(_pUi->actionStop, &QAction::triggered, this, &MainWindow::stopScope);
    connect(_pUi->actionDiagnostic, &QAction::triggered, this, &MainWindow::showDiagnostic);
    connect(_pUi->actionManageNotes, &QAction::triggered, this, &MainWindow::showNotesDialog);
    connect(_pUi->actionExit, &QAction::triggered, this, &MainWindow::exitApplication);
    connect(_pUi->actionSaveDataFile, &QAction::triggered, _pDataFileHandler, &DataFileHandler::selectDataExportFile);
    connect(_pUi->actionOpenProjectFile, &QAction::triggered, _pProjectFileHandler, &ProjectFileHandler::selectProjectOpenFile);
    connect(_pUi->actionReloadProjectFile, &QAction::triggered, _pProjectFileHandler, &ProjectFileHandler::reloadProjectFile);
    connect(_pUi->actionOpenDataFile, &QAction::triggered, _pDataFileHandler, &DataFileHandler::selectDataImportFile);
    connect(_pUi->actionExportImage, &QAction::triggered, this, &MainWindow::selectImageExportFile);
    connect(_pUi->actionSaveProjectFileAs, &QAction::triggered, _pProjectFileHandler, &ProjectFileHandler::selectProjectSaveFile);
    connect(_pUi->actionSaveProjectFile, &QAction::triggered, _pProjectFileHandler, &ProjectFileHandler::saveProjectFile);
    connect(_pUi->actionAbout, &QAction::triggered, this, &MainWindow::showAbout);
    connect(_pUi->actionOnlineDocumentation, &QAction::triggered, this, &MainWindow::openOnlineDoc);
    connect(_pUi->actionUpdateAvailable, &QAction::triggered, this, &MainWindow::openUpdateUrl);
    connect(_pUi->actionHighlightSamplePoints, &QAction::toggled, _pGuiModel, &GuiModel::setHighlightSamples);
    connect(_pUi->actionClearData, &QAction::triggered, this, &MainWindow::clearData);
    connect(_pUi->actionToggleMarkers, &QAction::triggered, this, &MainWindow::toggleMarkersState);
    connect(_pUi->actionConnectionSettings, &QAction::triggered, this, &MainWindow::showConnectionDialog);
    connect(_pUi->actionLogSettings, &QAction::triggered, this, &MainWindow::showLogSettingsDialog);
    connect(_pUi->actionRegisterSettings, &QAction::triggered, this, &MainWindow::handleShowRegisterDialog);
    connect(_pUi->actionAddNote, &QAction::triggered, this, &MainWindow::addNoteToGraph);
    connect(_pUi->actionZoom, &QAction::triggered, this, &MainWindow::toggleZoom); /* Only called on GUI click, not on setChecked */

    /*-- connect model to view --*/
    connect(_pGuiModel, &GuiModel::frontGraphChanged, this, &MainWindow::updateBringToFrontGrapMenu);
    connect(_pGuiModel, &GuiModel::frontGraphChanged, _pGraphView, &GraphView::bringToFront);
    connect(_pGuiModel, &GuiModel::highlightSamplesChanged, this, &MainWindow::updateHighlightSampleMenu);
    connect(_pGuiModel, &GuiModel::highlightSamplesChanged, _pGraphView, &GraphView::enableSamplePoints);
    connect(_pGuiModel, &GuiModel::cursorValuesChanged, _pGraphView, &GraphView::updateTooltip);
    connect(_pGuiModel, &GuiModel::cursorValuesChanged, _pLegend, &Legend::updateDataInLegend);

    connect(_pGuiModel, &GuiModel::windowTitleChanged, this, &MainWindow::updateWindowTitle);
    connect(_pGuiModel, &GuiModel::projectFilePathChanged, this, &MainWindow::projectFileLoaded);
    connect(_pGuiModel, &GuiModel::guiStateChanged, this, &MainWindow::updateGuiState);

    connect(_pGuiModel, &GuiModel::xAxisScalingChanged, this, &MainWindow::updatexAxisSlidingMode);
    connect(_pGuiModel, &GuiModel::xAxisScalingChanged, _pGraphView, &GraphView::rescalePlot);
    connect(_pGuiModel, &GuiModel::xAxisSlidingIntervalChanged, this, &MainWindow::updatexAxisSlidingInterval);
    connect(_pGuiModel, &GuiModel::xAxisSlidingIntervalChanged, _pGraphView, &GraphView::rescalePlot);

    connect(_pGuiModel, &GuiModel::yAxisScalingChanged, this, &MainWindow::updateyAxisSlidingMode);
    connect(_pGuiModel, &GuiModel::y2AxisScalingChanged, this, &MainWindow::updatey2AxisSlidingMode);
    connect(_pGuiModel, &GuiModel::yAxisScalingChanged, _pGraphView, &GraphView::rescalePlot);
    connect(_pGuiModel, &GuiModel::y2AxisScalingChanged, _pGraphView, &GraphView::rescalePlot);
    connect(_pGuiModel, &GuiModel::yAxisMinMaxchanged, this, &MainWindow::updateyAxisMinMax);
    connect(_pGuiModel, &GuiModel::y2AxisMinMaxchanged, this, &MainWindow::updatey2AxisMinMax);
    connect(_pGuiModel, &GuiModel::yAxisMinMaxchanged, _pGraphView, &GraphView::rescalePlot);
    connect(_pGuiModel, &GuiModel::y2AxisMinMaxchanged, _pGraphView, &GraphView::rescalePlot);
    connect(_pGuiModel, &GuiModel::communicationStatsChanged, this, &MainWindow::updateStats);
    connect(_pGuiModel, &GuiModel::markerStateChanged, this, &MainWindow::updateMarkerDockVisibility);
    connect(_pGuiModel, &GuiModel::zoomStateChanged, this, &MainWindow::handleZoomStateChanged);

    connect(_pGraphDataModel, &GraphDataModel::visibilityChanged, this, &MainWindow::handleGraphVisibilityChange);
    connect(_pGraphDataModel, &GraphDataModel::visibilityChanged, _pGraphView, &GraphView::showGraph);
    connect(_pGraphDataModel, &GraphDataModel::graphsAddData, _pGraphView, &GraphView::addData);
    connect(_pGraphDataModel, &GraphDataModel::activeChanged, this, &MainWindow::rebuildGraphMenu);
    connect(_pGraphDataModel, &GraphDataModel::activeChanged, _pGraphView, &GraphView::updateGraphs);
    connect(_pGraphDataModel, &GraphDataModel::colorChanged, this, &MainWindow::handleGraphColorChange);
    connect(_pGraphDataModel, &GraphDataModel::colorChanged, _pGraphView, &GraphView::changeGraphColor);
    connect(_pGraphDataModel, &GraphDataModel::valueAxisChanged, _pGraphView, &GraphView::changeGraphAxis);
    connect(_pGraphDataModel, &GraphDataModel::labelChanged, this, &MainWindow::handleGraphLabelChange);

    connect(_pGraphDataModel, &GraphDataModel::added, this, &MainWindow::rebuildGraphMenu);
    connect(_pGraphDataModel, &GraphDataModel::added, _pGraphView, &GraphView::updateGraphs);

    connect(_pGraphDataModel, &GraphDataModel::moved, this, &MainWindow::rebuildGraphMenu);
    connect(_pGraphDataModel, &GraphDataModel::moved, _pGraphView, &GraphView::updateGraphs);

    connect(_pGraphDataModel, &GraphDataModel::removed, this, &MainWindow::rebuildGraphMenu);
    connect(_pGraphDataModel, &GraphDataModel::removed, _pGraphView, &GraphView::updateGraphs);

    connect(_pGraphDataModel, &GraphDataModel::expressionChanged, _pGraphView, &GraphView::clearGraph);

    // Update cursor values in legend
    connect(_pGraphView, &GraphView::cursorValueUpdate, _pLegend, &Legend::updateDataInLegend);
    connect(_pGraphView, &GraphView::dataAddedToPlot, _pDataFileHandler, &DataFileHandler::exportDataLine);

    _pGraphShowHide = _pUi->menuShowHide;
    _pGraphBringToFront = _pUi->menuBringToFront;
    _pBringToFrontGroup = new QActionGroup(this);

    // Compose rightclick menu
    _menuRightClick.addMenu(_pUi->menuBringToFront);
    _menuRightClick.addMenu(_pUi->menuShowHide);
    _menuRightClick.addSeparator();
    _menuRightClick.addAction(_pUi->actionHighlightSamplePoints);
    _menuRightClick.addAction(_pUi->actionClearData);
    _menuRightClick.addAction(_pUi->actionToggleMarkers);
    _menuRightClick.addSeparator();
    _menuRightClick.addAction(_pUi->actionAddNote);
    _menuRightClick.addAction(_pUi->actionManageNotes);

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

    this->winId(); // Ensures the native window has been created.
    connect(this->windowHandle(), &QWindow::screenChanged, _pGraphView, &GraphView::screenChanged);

    // For dock undock
    connect(_pUi->scaleOptionsDock, &QDockWidget::topLevelChanged, this, &MainWindow::scaleWidgetUndocked);
    connect(_pUi->legendDock, &QDockWidget::topLevelChanged, this, &MainWindow::legendWidgetUndocked);

    // For rightclick menu
    _pUi->customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_pUi->customPlot, &QCustomPlot::customContextMenuRequested, this, &MainWindow::showContextMenu);

    connect(_pUi->spinSlidingXInterval, QOverload<int>::of(&QSpinBox::valueChanged), _pGuiModel, &GuiModel::setxAxisSlidingInterval);

    QDoubleValidator* pValidator = new QDoubleValidator(-1000000, 1000000, 3); /* Util:formatdouble is also 3 decimals */
    _pUi->lineYMin->setValidator(pValidator);
    _pUi->lineY2Min->setValidator(pValidator);
    _pUi->lineYMax->setValidator(pValidator);
    _pUi->lineY2Max->setValidator(pValidator);
    connect(_pUi->lineYMin, &QLineEdit::editingFinished, this, &MainWindow::handleYMinChange);
    connect(_pUi->lineY2Min, &QLineEdit::editingFinished, this, &MainWindow::handleY2MinChange);
    connect(_pUi->lineYMax, &QLineEdit::editingFinished, this, &MainWindow::handleYMaxChange);
    connect(_pUi->lineY2Max, &QLineEdit::editingFinished, this, &MainWindow::handleY2MaxChange);

    //valueChanged is only send when done editing...
    _pUi->spinSlidingXInterval->setKeyboardTracking(false);

    // Create button group for X axis scaling options
    _pXAxisScaleGroup = new QButtonGroup();
    _pXAxisScaleGroup->setExclusive(true);
    _pXAxisScaleGroup->addButton(_pUi->radioXFullScale, AxisMode::SCALE_AUTO);
    _pXAxisScaleGroup->addButton(_pUi->radioXSliding, AxisMode::SCALE_SLIDING);
    _pXAxisScaleGroup->addButton(_pUi->radioXManual, AxisMode::SCALE_MANUAL);
    connect(_pXAxisScaleGroup, &QButtonGroup::idClicked, this, &MainWindow::xAxisScaleGroupClicked);

    _pUi->scaleTab->setCurrentIndex(0);

    // Create button group for Y axis scaling options
    _pYAxisScaleGroup = new QButtonGroup();
    _pYAxisScaleGroup->setExclusive(true);
    _pYAxisScaleGroup->addButton(_pUi->radioYFullScale, AxisMode::SCALE_AUTO);
    _pYAxisScaleGroup->addButton(_pUi->radioYWindowScale, AxisMode::SCALE_WINDOW_AUTO);
    _pYAxisScaleGroup->addButton(_pUi->radioYMinMax, AxisMode::SCALE_MINMAX);
    _pYAxisScaleGroup->addButton(_pUi->radioYManual, AxisMode::SCALE_MANUAL);
    connect(_pYAxisScaleGroup, &QButtonGroup::idClicked, this, &MainWindow::yAxisScaleGroupClicked);

    // Create button group for Y2 axis scaling options
    _pY2AxisScaleGroup = new QButtonGroup();
    _pY2AxisScaleGroup->setExclusive(true);
    _pY2AxisScaleGroup->addButton(_pUi->radioYFullScale_2, AxisMode::SCALE_AUTO);
    _pY2AxisScaleGroup->addButton(_pUi->radioYWindowScale_2, AxisMode::SCALE_WINDOW_AUTO);
    _pY2AxisScaleGroup->addButton(_pUi->radioYMinMax_2, AxisMode::SCALE_MINMAX);
    _pY2AxisScaleGroup->addButton(_pUi->radioYManual_2, AxisMode::SCALE_MANUAL);
    connect(_pY2AxisScaleGroup, &QButtonGroup::idClicked, this, &MainWindow::y2AxisScaleGroupClicked);

    /* handle focus change */
    connect(dynamic_cast<QApplication*>(QApplication::instance()), &QApplication::focusChanged, this, &MainWindow::appFocusChanged);

    /* Update notes in data file when requested by notes model */
    connect(_pNoteModel, &NoteModel::dataFileUpdateRequested, this, &MainWindow::updateDataFileNotes);

    /* Trigger update check */
    _pUi->actionUpdateAvailable->setVisible(false);
    _pUpdateNotify = new UpdateNotify(new VersionDownloader(), Util::currentVersion());
    connect(_pUpdateNotify, &UpdateNotify::updateCheckResult, this, &MainWindow::showVersionUpdate);
    _pUpdateNotify->checkForUpdate();

    // Default to full auto scaling
    _pGraphView->setAxisToAuto();

    connect(_pGraphDataHandler, &GraphDataHandler::graphDataReady, _pGraphView, &GraphView::plotResults);
    connect(_pGraphDataHandler, &GraphDataHandler::graphDataReady, _pLegend, &Legend::addLastReceivedDataToLegend);
    connect(_pGraphDataHandler, &GraphDataHandler::graphDataReady, this, &MainWindow::updateCommunicationStats);

    handleCommandLineArguments(cmdArguments);

#if 0
    //Debugging
    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(0, "${40001}");

    //_pGraphDataModel->setActive(2, false);

#endif

}

MainWindow::~MainWindow()
{
    delete _pGraphView;
    delete _pConnectionDialog;
    delete _pModbusPoll;
    delete _pGraphShowHide;
    delete _pGraphBringToFront;
    delete _pDataFileHandler;
    delete _pProjectFileHandler;

    delete _pUpdateNotify;

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

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (
            (_pGuiModel->guiState() == GuiModel::DATA_LOADED)
            && (_pNoteModel->isNotesDataUpdated())
        )
    {
        QMessageBox::StandardButton resBtn = QMessageBox::question(this, windowTitle(),
                                                                    tr("The notes are changed.\nDo you want discard the changes or update (save) the data file?\n"),
                                                                    QMessageBox::Cancel | QMessageBox::Discard | QMessageBox::Save,
                                                                    QMessageBox::Cancel);
        if (resBtn == QMessageBox::Cancel)
        {
            event->ignore();
        }
        else if (resBtn == QMessageBox::Discard)
        {
            event->accept();
        }
        else if (resBtn == QMessageBox::Save)
        {
            if (_pDataFileHandler->updateNoteLines())
            {
                event->accept();
            }
            else
            {
                event->ignore();
            }
        }
    }
    else
    {
        event->accept();
    }
}

void MainWindow::exitApplication()
{
    QApplication::quit();
}

void MainWindow::selectImageExportFile()
{
    /* Add question whether to save when legend is undocked */
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
        QFileDialog dialog(this);
        FileSelectionHelper::configureFileDialog(&dialog,
                                                 FileSelectionHelper::DIALOG_TYPE_SAVE,
                                                 FileSelectionHelper::FILE_TYPE_PNG);

        QString selectedFile = FileSelectionHelper::showDialog(&dialog);
        if (!selectedFile.isEmpty())
        {
            QPixmap pixMap = this->window()->grab();
            pixMap.save(selectedFile);
        }
    }
}

void MainWindow::showAbout()
{
    AboutDialog aboutDialog(_pUpdateNotify, this);

    aboutDialog.exec();
}

void MainWindow::openOnlineDoc()
{
    QDesktopServices::openUrl(QUrl("https://modbusscope.readthedocs.io/en/stable"));
}

void MainWindow::openUpdateUrl()
{
    if (_pUpdateNotify->link().isValid())
    {
        QDesktopServices::openUrl(_pUpdateNotify->link());
    }
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

void MainWindow::showLogSettingsDialog()
{
    _pLogDialog->exec();
}

void MainWindow::handleShowRegisterDialog(bool checked)
{
    Q_UNUSED(checked);
    showRegisterDialog(QString(""));
}

void MainWindow::showRegisterDialog(QString mbcFile)
{
    if (_pGuiModel->guiState() == GuiModel::DATA_LOADED)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Clear data?", "An imported data file is loaded. Do you want to clear the data and start adding registers for a new log?", QMessageBox::Yes|QMessageBox::No);
        if (reply != QMessageBox::Yes)
        {
            return;
        }

        _pGraphDataModel->clear();
        _pNoteModel->clear();

        _pGuiModel->setGuiState(GuiModel::INIT);
    }

    RegisterDialog registerDialog(_pGuiModel, _pGraphDataModel, _pSettingsModel, this);

    if (mbcFile.isEmpty())
    {
        registerDialog.exec();
    }
    else
    {
        _pGuiModel->setLastMbcImportedFile(mbcFile);
        registerDialog.execWithMbcImport();
    }
}

void MainWindow::addNoteToGraph()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Add note"),
                                         tr("Note Text:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok)
    {
        Note newNote(text, _lastRightClickPos);
        _pNoteModel->add(newNote);
    }
}

void MainWindow::toggleZoom(bool checked)
{
    /*
    Handler of triggered signal => only called when user clicks zoom button,
    Not when setChecked function is called
    */
    if (_pGraphDataModel->size() > 0)
    {
        if (checked)
        {
            /* Activate */
            _pGuiModel->setZoomState(GuiModel::ZOOM_TRIGGERED);
        }
        else
        {
            /* Deactivate */
            _pGuiModel->setZoomState(GuiModel::ZOOM_IDLE);
        }
    }
    else
    {
        _pGuiModel->setZoomState(GuiModel::ZOOM_IDLE);
        _pUi->actionZoom->setChecked(false);
    }
}

void MainWindow::clearData()
{
    _pGuiModel->setCommunicationStats(0, 0);
    _pGuiModel->setCommunicationStartTime(QDateTime::currentMSecsSinceEpoch());

    _pModbusPoll->resetCommunicationStats();
    _pGraphView->clearResults();
    _pGuiModel->clearMarkersState();
    _pDataFileHandler->rewriteDataFile();
    _pNoteModel->clear();
    _pLegend->clearLegendData();
}

void MainWindow::startScope()
{
    if (_pGuiModel->guiState() == GuiModel::DATA_LOADED)
    {
        _pGraphDataModel->clear();
        _pNoteModel->clear();

        _pGuiModel->setGuiState(GuiModel::INIT);
    }

    if (_pGraphDataModel->activeCount() != 0)
    {

        _pGuiModel->setGuiState(GuiModel::STARTED);

        _runtimeTimer.singleShot(250, this, &MainWindow::updateRuntime);

        QList<ModbusRegister> registerList;
        _pGraphDataHandler->processActiveRegisters(_pGraphDataModel);
        _pGraphDataHandler->modbusRegisterList(registerList);

        _pModbusPoll->startCommunication(registerList);

        clearData();

        if (_pSettingsModel->writeDuringLog())
        {
            _pDataFileHandler->enableExporterDuringLog();
        }

        if (_pGuiModel->xAxisScalingMode() == AxisMode::SCALE_MANUAL)
        {
            _pGuiModel->setxAxisScale(AxisMode::SCALE_AUTO);
        }

        if (_pGuiModel->yAxisScalingMode() == AxisMode::SCALE_MANUAL)
        {
            _pGuiModel->setyAxisScale(AxisMode::SCALE_AUTO);
        }

        if (_pGuiModel->y2AxisScalingMode() == AxisMode::SCALE_MANUAL)
        {
            _pGuiModel->sety2AxisScale(AxisMode::SCALE_AUTO);
        }

    }
    else
    {
        Util::showError("There are no register in the scope list. Please select at least one register.");
    }
}

void MainWindow::stopScope()
{
    _pModbusPoll->stopCommunication();

    _pGuiModel->setCommunicationEndTime(QDateTime::currentMSecsSinceEpoch());

    if (_pSettingsModel->writeDuringLog())
    {
        _pDataFileHandler->disableExporterDuringLog();
    }

    _pGuiModel->setGuiState(GuiModel::STOPPED);
}

void MainWindow::showDiagnostic()
{
    _pDiagnosticDialog->show();
}

void MainWindow::showNotesDialog()
{
    _pNotesDock->show();
}

void MainWindow::toggleMarkersState()
{
    if (_pGuiModel->markerState())
    {
        /* One or more markers are visible, hide them */
        _pGuiModel->clearMarkersState();
    }
    else
    {
        /* No markers are visible, show them */
        if (_pGraphDataModel->size() > 0)
        {
            _pGraphView->showMarkers();
        }
    }
}

void MainWindow::handleYMinChange()
{
    bool bOk = false;
    double val = QLocale().toDouble(_pUi->lineYMin->text(), &bOk);

    if (bOk)
    {
        _pGuiModel->setyAxisMin(val);
        _pGuiModel->setyAxisScale(AxisMode::SCALE_MINMAX);
    }
}

void MainWindow::handleY2MinChange()
{
    bool bOk = false;
    double val = QLocale().toDouble(_pUi->lineY2Min->text(), &bOk);

    if (bOk)
    {
        _pGuiModel->sety2AxisMin(val);
        _pGuiModel->sety2AxisScale(AxisMode::SCALE_MINMAX);
    }
}

void MainWindow::handleYMaxChange()
{
    bool bOk = false;
    double val = QLocale().toDouble(_pUi->lineYMax->text(), &bOk);

    if (bOk)
    {
        _pGuiModel->setyAxisMax(val);
        _pGuiModel->setyAxisScale(AxisMode::SCALE_MINMAX);
    }
}

void MainWindow::handleY2MaxChange()
{
    bool bOk = false;
    double val = QLocale().toDouble(_pUi->lineY2Max->text(), &bOk);

    if (bOk)
    {
        _pGuiModel->sety2AxisMax(val);
        _pGuiModel->sety2AxisScale(AxisMode::SCALE_MINMAX);
    }
}

void MainWindow::handleGraphVisibilityChange(quint32 graphIdx)
{
    if (_pGraphDataModel->isActive(graphIdx))
    {
        const quint32 activeIdx = static_cast<quint32>(_pGraphDataModel->convertToActiveGraphIndex(graphIdx));

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
        const quint32 activeIdx = static_cast<quint32>(_pGraphDataModel->convertToActiveGraphIndex(graphIdx));

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
        const quint32 activeIdx = static_cast<quint32>(_pGraphDataModel->convertToActiveGraphIndex(graphIdx));
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

void MainWindow::handleZoomStateChanged()
{
    if (_pGuiModel->zoomState() == GuiModel::ZOOM_IDLE)
    {
        _pUi->actionZoom->setChecked(false);
    }
    else
    {
        _pUi->actionZoom->setChecked(true);
    }
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

        QObject::connect(pShowHideAction, &QAction::toggled, this, &MainWindow::menuShowHideGraphClicked);
        QObject::connect(pBringToFront, &QAction::toggled, this, &MainWindow::menuBringToFrontGraphClicked);
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
    if (_pGuiModel->xAxisScalingMode() == AxisMode::SCALE_AUTO)
    {
        // Full auto scaling
        _pUi->radioXFullScale->setChecked(true);
    }
    else if (_pGuiModel->xAxisScalingMode() == AxisMode::SCALE_SLIDING)
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
    if (_pGuiModel->yAxisScalingMode() == AxisMode::SCALE_AUTO)
    {
        // Full auto scaling
        _pUi->radioYFullScale->setChecked(true);
    }
    else if (_pGuiModel->yAxisScalingMode() == AxisMode::SCALE_MINMAX)
    {
        // Min and max selected
        _pUi->radioYMinMax->setChecked(true);
    }
    else if (_pGuiModel->yAxisScalingMode() == AxisMode::SCALE_WINDOW_AUTO)
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

void MainWindow::updatey2AxisSlidingMode()
{
    if (_pGuiModel->y2AxisScalingMode() == AxisMode::SCALE_AUTO)
    {
        // Full auto scaling
        _pUi->radioYFullScale_2->setChecked(true);
    }
    else if (_pGuiModel->y2AxisScalingMode() == AxisMode::SCALE_MINMAX)
    {
        // Min and max selected
        _pUi->radioYMinMax_2->setChecked(true);
    }
    else if (_pGuiModel->y2AxisScalingMode() == AxisMode::SCALE_WINDOW_AUTO)
    {
        // Window auto scale selected
        _pUi->radioYWindowScale_2->setChecked(true);
    }
    else
    {
        // manual
        _pUi->radioYManual_2->setChecked(true);
    }
}

void MainWindow::updateyAxisMinMax()
{
    _pUi->lineYMin->setText(Util::formatDoubleForExport(_pGuiModel->yAxisMin()));
    _pUi->lineYMax->setText(Util::formatDoubleForExport(_pGuiModel->yAxisMax()));
}

void MainWindow::updatey2AxisMinMax()
{
    _pUi->lineY2Min->setText(Util::formatDoubleForExport(_pGuiModel->y2AxisMin()));
    _pUi->lineY2Max->setText(Util::formatDoubleForExport(_pGuiModel->y2AxisMax()));
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
        _pUi->actionOpenDataFile->setEnabled(true);
        _pUi->actionOpenProjectFile->setEnabled(true);
        _pUi->actionSaveDataFile->setEnabled(false);
        _pUi->actionExportImage->setEnabled(false);
        _pUi->actionSaveProjectFileAs->setEnabled(true);
        _pUi->actionClearData->setEnabled(true);

        _pStatusRuntime->setText(_cRuntime.arg("0:00:00"));
        _pStatusRuntime->setVisible(true);

        _pStatusStats->setText(_cStatsTemplate.arg(0).arg(0));
        _pStatusRuntime->setVisible(true);

        _pDataParserModel->resetSettings();
        _pGuiModel->setProjectFilePath(QString(""));

        _pGuiModel->setWindowTitleDetail(QString(""));
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
        _pUi->actionOpenDataFile->setEnabled(false);
        _pUi->actionOpenProjectFile->setEnabled(false);
        _pUi->actionSaveDataFile->setEnabled(false);
        _pUi->actionSaveProjectFileAs->setEnabled(false);
        _pUi->actionSaveProjectFile->setEnabled(false);
        _pUi->actionExportImage->setEnabled(false);
        _pUi->actionReloadProjectFile->setEnabled(false);
        _pUi->actionClearData->setEnabled(true);

        _pStatusRuntime->setText(_cRuntime.arg("0:00:00"));
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
        _pUi->actionOpenDataFile->setEnabled(true);
        _pUi->actionOpenProjectFile->setEnabled(true);
        _pUi->actionSaveDataFile->setEnabled(true);
        _pUi->actionSaveProjectFileAs->setEnabled(true);
        _pUi->actionExportImage->setEnabled(true);
        _pUi->actionClearData->setEnabled(true);

        _pDataParserModel->resetSettings();

        if (_pGuiModel->projectFilePath().isEmpty())
        {
            _pUi->actionReloadProjectFile->setEnabled(false);
            _pUi->actionSaveProjectFile->setEnabled(false);
        }
        else
        {
            _pUi->actionReloadProjectFile->setEnabled(true);
            _pUi->actionSaveProjectFile->setEnabled(true);
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
        _pUi->actionOpenDataFile->setEnabled(true);
        _pUi->actionOpenProjectFile->setEnabled(true);

        _pUi->actionSaveDataFile->setEnabled(false);
        _pUi->actionSaveProjectFileAs->setEnabled(false);
        _pUi->actionSaveProjectFile->setEnabled(false);
        _pUi->actionExportImage->setEnabled(true);
        _pUi->actionClearData->setEnabled(false);

        _pStatusRuntime->setText(QString(""));
        _pStatusRuntime->setVisible(false);

        _pStatusStats->setText(QString(""));
        _pStatusStats->setVisible(false);

        _pUi->actionReloadProjectFile->setEnabled(false);

        _pGuiModel->setWindowTitleDetail(QFileInfo(_pDataParserModel->dataFilePath()).fileName());
    }
}

void MainWindow::projectFileLoaded()
{
    // if a project file is previously loaded, then it can be reloaded
    if (_pGuiModel->projectFilePath().isEmpty())
    {
        _pGuiModel->setWindowTitleDetail("");
        _pUi->actionReloadProjectFile->setEnabled(false);
        _pUi->actionSaveProjectFile->setEnabled(false);
    }
    else
    {
        _pGuiModel->setWindowTitleDetail(QFileInfo(_pGuiModel->projectFilePath()).fileName());
        _pUi->actionReloadProjectFile->setEnabled(true);
        _pUi->actionSaveProjectFile->setEnabled(true);
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
        _lastRightClickPos = _pGraphView->pixelToPointF(pos);
        _menuRightClick.popup(_pUi->customPlot->mapToGlobal(pos));
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
    if (!_pModbusPoll->isActive())
    {
        const QString filename(e->mimeData()->urls().last().toLocalFile());
        QFileInfo fileInfo(filename);
        _pGuiModel->setLastDir(fileInfo.dir().absolutePath());
        if (fileInfo.completeSuffix().toLower() == QString("mbs"))
        {
            _pProjectFileHandler->openProjectFile(filename);
        }
        else if (fileInfo.completeSuffix().toLower() == QString("csv"))
        {
            _pDataFileHandler->openDataFile(filename);
        }
        else if (fileInfo.completeSuffix().toLower() == QString("mbc"))
        {
            showRegisterDialog(filename);
        }
        else
        {
            /* Assume data file import */
            _pDataFileHandler->openDataFile(filename);
        }
    }
}

void MainWindow::appFocusChanged(QWidget *old, QWidget *now)
{
    Q_UNUSED(now);
    if (old != NULL)
    {
        _pGuiModel->setCursorValues(false);
    }
}

void MainWindow::xAxisScaleGroupClicked(int id)
{
    _pGuiModel->setxAxisScale((AxisMode::AxisScaleOptions)id);
}

void MainWindow::yAxisScaleGroupClicked(int id)
{
    _pGuiModel->setyAxisScale((AxisMode::AxisScaleOptions)id) ;
}

void MainWindow::y2AxisScaleGroupClicked(int id)
{
    _pGuiModel->sety2AxisScale((AxisMode::AxisScaleOptions)id) ;
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

    QString strTimePassed = QString("%1:%2:%3").arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));

    _pStatusRuntime->setText(_cRuntime.arg(strTimePassed));

    // restart timer
    if (_pModbusPoll->isActive())
    {
        _runtimeTimer.singleShot(250, this, &MainWindow::updateRuntime);
    }
}

void MainWindow::updateCommunicationStats(QList<bool> successList)
{
    quint32 error = 0;
    quint32 success = 0;
    for(int idx = 0; idx < successList.size(); idx++)
    {
        successList[idx] ? success++ : error++;
    }

    _pGuiModel->incrementCommunicationStats(success, error);
}

void MainWindow::updateDataFileNotes()
{
    if (_pGuiModel->guiState() == GuiModel::DATA_LOADED)
    {
        if (_pNoteModel->isNotesDataUpdated())
        {
            _pDataFileHandler->updateNoteLines();
        }
    }
}

void MainWindow::showVersionUpdate(UpdateNotify::UpdateState result)
{
    if (result == UpdateNotify::VERSION_LATEST)
    {
        _pUi->actionUpdateAvailable->setVisible(false);
    }
    else
    {
        QString strUpdate = QString("v%1 available...").arg(_pUpdateNotify->version());

        _pUi->menuHelp->setIcon(_pUi->actionUpdateAvailable->icon());

        _pUi->actionUpdateAvailable->setText(strUpdate);
        _pUi->actionUpdateAvailable->setVisible(true);
    }
}

void MainWindow::handleCommandLineArguments(QStringList cmdArguments)
{
    QCommandLineParser argumentParser;
    argumentParser.setApplicationDescription("Log data through the Modbus protocol");
    argumentParser.addHelpOption();

	// Project file option
    argumentParser.addPositionalArgument("project file", QCoreApplication::translate("main", "Project file (.mbs) to open"));

    // Process arguments
    argumentParser.process(cmdArguments);

    if (!argumentParser.positionalArguments().isEmpty())
    {
        QString filename = argumentParser.positionalArguments().at(0);
        QFileInfo fileInfo(filename);
        _pGuiModel->setLastDir(fileInfo.dir().absolutePath());
        _pProjectFileHandler->openProjectFile(filename);
    }
}

