#include "controllers/scopecontroller.h"

#include "ProtocolAdapter/adapterhub.h"
#include "communication/adapterpoll.h"
#include "communication/communicationstats.h"
#include "importexport/datafilehandler.h"
#include "importexport/projectfilehandler.h"
#include "models/graphdatamodel.h"
#include "models/guimodel.h"
#include "models/notemodel.h"
#include "models/settingsmodel.h"
#include "util/expressionstatus.h"

#include <QFileInfo>
#include <utility>

using GuiState = GuiModel::GuiState;

/*!
 * \brief Construct the controller and its owned session/data-handling collaborators
 * \param pGuiModel GUI state / project-path model
 * \param pSettingsModel Settings model (adapters, devices, poll time)
 * \param pGraphDataModel Graph data model (registers, expressions, series data)
 * \param pCommunicationStatsModel Communication statistics model
 * \param pNoteModel Notes model
 * \param pDataParserModel Data-file parser settings model
 * \param parent Parent QObject
 */
ScopeController::ScopeController(GuiModel* pGuiModel,
                                 SettingsModel* pSettingsModel,
                                 GraphDataModel* pGraphDataModel,
                                 CommunicationStatsModel* pCommunicationStatsModel,
                                 NoteModel* pNoteModel,
                                 DataParserModel* pDataParserModel,
                                 QObject* parent)
    : QObject(parent),
      _pGuiModel(pGuiModel),
      _pSettingsModel(pSettingsModel),
      _pGraphDataModel(pGraphDataModel),
      _pCommunicationStatsModel(pCommunicationStatsModel),
      _pNoteModel(pNoteModel),
      _pDataParserModel(pDataParserModel)
{
    _pAdapterPoll = new AdapterPoll(_pSettingsModel, this);
    connect(_pAdapterPoll, &AdapterPoll::registerDataReady, this, &ScopeController::onRegisterDataReady);

    _pDataFileHandler = new DataFileHandler(_pGuiModel, _pGraphDataModel, _pCommunicationStatsModel, _pNoteModel,
                                            _pSettingsModel, _pDataParserModel, this);
    _pProjectFileHandler = new ProjectFileHandler(_pGuiModel, _pSettingsModel, _pGraphDataModel, this);
    _pExpressionStatus = new ExpressionStatus(_pGraphDataModel, _pSettingsModel, this);
    _pCommunicationStats = new CommunicationStats(_pGraphDataModel, _pCommunicationStatsModel, 50, this);

    connect(_pGraphDataModel, &GraphDataModel::colorChanged, _pDataFileHandler, &DataFileHandler::rewriteDataFile);
    connect(_pNoteModel, &NoteModel::dataFileUpdateRequested, this, &ScopeController::updateDataFileNotes);
}

ScopeController::~ScopeController()
{
    _pAdapterPoll->stopCommunication();
    disconnect(_pAdapterPoll, &AdapterPoll::registerDataReady, this, &ScopeController::onRegisterDataReady);
}

/*!
 * \brief Initialize the adapter subprocess
 */
void ScopeController::initAdapter()
{
    _pAdapterPoll->initAdapter();
}

/*!
 * \brief Whether a logging session is currently active
 */
bool ScopeController::isPolling() const
{
    return _pAdapterPoll->isActive();
}

/*!
 * \brief Return the adapter hub, for callers that need to interact with an adapter directly
 */
AdapterHub* ScopeController::adapterHub() const
{
    return _pAdapterPoll->adapterHub();
}

/*!
 * \brief Return the data-file handler, for wiring UI-side export/import actions
 */
DataFileHandler* ScopeController::dataFileHandler() const
{
    return _pDataFileHandler;
}

/*!
 * \brief Return the project-file handler, for wiring UI-side project actions
 */
ProjectFileHandler* ScopeController::projectFileHandler() const
{
    return _pProjectFileHandler;
}

/*!
 * \brief Return the communication statistics collaborator, for wiring UI-side timing updates
 */
CommunicationStats* ScopeController::communicationStats() const
{
    return _pCommunicationStats;
}

/*!
 * \brief Start a logging session, or report an error when no registers are configured
 */
void ScopeController::start()
{
    if (_pGuiModel->guiState() == GuiState::DATA_LOADED)
    {
        _pGraphDataModel->clear();
        _pNoteModel->clear();

        _pGuiModel->setGuiState(GuiState::INIT);
    }

    if (_pGraphDataModel->activeCount() != 0)
    {
        _pGuiModel->setGuiState(GuiState::STARTED);

        clear();

        QList<DataPoint> registerList;
        _graphDataHandler.setupExpressions(_pGraphDataModel, registerList);

        _pAdapterPoll->startCommunication(registerList);
        _pCommunicationStats->start();

        if (_pSettingsModel->writeDuringLog())
        {
            _pDataFileHandler->enableExporterDuringLog();
        }
    }
    else
    {
        emit errorOccurred(tr("There are no registers in the scope list. Please select at least one register."));
    }
}

/*!
 * \brief Stop the active logging session
 */
void ScopeController::stop()
{
    _pAdapterPoll->stopCommunication();
    _pCommunicationStats->stop();

    if (_pSettingsModel->writeDuringLog())
    {
        _pDataFileHandler->disableExporterDuringLog();
    }

    _pGuiModel->setGuiState(GuiState::STOPPED);
}

/*!
 * \brief Reset communication stats, graph data and notes for a new logging session
 */
void ScopeController::clear()
{
    _pCommunicationStats->resetTiming();
    _pAdapterPoll->resetCommunicationStats();

    // Authoritative model-side clear (also needed by a future headless caller with no GraphView);
    // GraphView::clearResults(), triggered below via dataCleared(), redundantly clears the same
    // series for currently-plotted graphs plus the QCPGraph curve data itself — harmless overlap.
    QList<GraphIdx> activeGraphList;
    _pGraphDataModel->activeGraphIndexList(activeGraphList);
    for (const GraphIdx& graphIdx : std::as_const(activeGraphList))
    {
        _pGraphDataModel->mutableDataSeries(graphIdx)->clear();
    }

    _pGuiModel->clearMarkersState();
    _pDataFileHandler->rewriteDataFile();
    _pNoteModel->clear();

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

    emit dataCleared();
}

/*!
 * \brief Open a project (.mbs), MBC import (.mbc) or data file, based on its extension
 * \param filename Path to the file to open
 */
void ScopeController::openFile(QString filename)
{
    QFileInfo fileInfo(filename);
    _pGuiModel->setLastDir(fileInfo.dir().absolutePath());
    const QString suffix = fileInfo.suffix().toLower();
    if (suffix == QStringLiteral("mbs"))
    {
        _pProjectFileHandler->openProjectFile(filename);
    }
    else if (suffix == QStringLiteral("mbc"))
    {
        if (_pSettingsModel->isMbcCompatible())
        {
            _pGuiModel->setLastMbcImportedFile(filename);
            emit mbcImportRequested();
        }
        else
        {
            emit errorOccurred(tr("MBC import is not supported by the current adapter configuration."));
        }
    }
    else
    {
        /* Assume data file import */
        _pDataFileHandler->openDataFile(filename);
    }
}

/*!
 * \brief Process incoming register data and update communication statistics
 */
void ScopeController::onRegisterDataReady(const ResultDoubleList& results)
{
    ResultDoubleList processed = _graphDataHandler.handleRegisterData(results);

    // dataProcessed is relied on to run its (UI-side) handler synchronously before the stats
    // update below, matching the original plot-then-legend-then-stats order; holds only as long
    // as every connection in this chain stays same-thread/direct.
    emit dataProcessed(processed);
    _pCommunicationStats->updateCommunicationStats(processed);
}

/*!
 * \brief Rewrite the data file's note lines when the note model reports pending changes
 */
void ScopeController::updateDataFileNotes()
{
    if (_pGuiModel->guiState() == GuiState::DATA_LOADED)
    {
        if (_pNoteModel->isNotesDataUpdated())
        {
            _pDataFileHandler->updateNoteLines();
        }
    }
}
