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
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>
#include <utility>

using GuiState = GuiModel::GuiState;

/*!
 * \brief Construct the controller and its owned session/data-handling collaborators
 * \param pGuiModel GUI state / project-path model
 * \param pSettingsModel Settings model (adapters, devices, poll time)
 * \param pGraphDataModel Graph data model (data points, expressions, series data)
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
    connect(_pAdapterPoll, &AdapterPoll::communicationError, this, &ScopeController::onCommunicationError);

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
    disconnect(_pAdapterPoll, &AdapterPoll::communicationError, this, &ScopeController::onCommunicationError);
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
 * \brief Start a logging session, or report an error when no data points are configured
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
        QList<DataPoint> registerList;
        _graphDataHandler.setupExpressions(_pGraphDataModel, registerList);

        const QString validationError = sessionValidationError(registerList);
        if (!validationError.isEmpty())
        {
            emit errorOccurred(validationError);
            return;
        }

        _pGuiModel->setGuiState(GuiState::STARTED);

        clear();

        _pAdapterPoll->startCommunication(registerList);
        if (!_pAdapterPoll->isActive())
        {
            /* A synchronous session error was already handled by onCommunicationError(), which
               called stop(); starting communication stats or the exporter here would resurrect
               state that stop() just tore down. */
            return;
        }
        _pCommunicationStats->start();

        if (_pSettingsModel->writeDuringLog())
        {
            _pDataFileHandler->enableExporterDuringLog();
        }
    }
    else
    {
        emit errorOccurred(tr("There are no data points in the scope list. Please select at least one data point."));
    }
}

/*!
 * \brief Check that every device referenced by a register can actually be reached
 *
 * Guards against three ways a register's device can silently fail to route correctly: it may
 * not exist at all (in which case SettingsModel::adapterIdForDevice() falls back to "modbus"
 * rather than reporting an error), it may have been dropped from the owning adapter's wire
 * config by AdapterData::configForWire()'s device-limit truncation, or its owning adapter may
 * not be a real, discovered adapter at all. The truncation check only runs for an adapter
 * SettingsModel already knows about, so it never has to insert a placeholder AdapterData entry
 * as a side effect of validation alone.
 * \param registerList  The registers about to be sent to AdapterPoll::startCommunication().
 * \return An empty string when every device is reachable; otherwise a message naming each
 * problem device, suitable for errorOccurred().
 */
QString ScopeController::sessionValidationError(const QList<DataPoint>& registerList) const
{
    QStringList problems;
    QSet<deviceId_t> seenDevices;
    QHash<QString, QJsonValue> wireDevicesByAdapter;

    for (const DataPoint& dataPoint : registerList)
    {
        const deviceId_t devId = dataPoint.deviceId();
        if (seenDevices.contains(devId))
        {
            continue;
        }
        seenDevices.insert(devId);

        if (!_pSettingsModel->hasDevice(devId))
        {
            problems.append(tr("Device %1: no such device").arg(devId));
            continue;
        }

        const QString deviceName = _pSettingsModel->deviceSettings(devId)->name();
        const QString adapterId = _pSettingsModel->adapterIdForDevice(devId);

        /* adapterData() inserts a default entry for an unknown adapterId, which would leave a
           phantom entry behind for validation alone - only look up wire config for an adapter
           SettingsModel already knows about. An adapter it has never heard of can't have
           truncated anything, so fall through to the liveness check below. */
        if (_pSettingsModel->adapterIds().contains(adapterId))
        {
            if (!wireDevicesByAdapter.contains(adapterId))
            {
                wireDevicesByAdapter[adapterId] =
                  _pSettingsModel->adapterData(adapterId)->configForWire().value("devices");
            }

            const QJsonValue& wireDevices = wireDevicesByAdapter[adapterId];
            if (wireDevices.isArray())
            {
                const QJsonArray wireDeviceArray = wireDevices.toArray();
                bool survivesTruncation = false;
                for (const auto& wireDevice : wireDeviceArray)
                {
                    if (wireDevice.toObject().value("id").toInt(-1) == static_cast<int>(devId))
                    {
                        survivesTruncation = true;
                        break;
                    }
                }

                if (!survivesTruncation)
                {
                    problems.append(tr("Device %1 (\"%2\"): dropped from adapter '%3' (device limit exceeded)")
                                      .arg(devId)
                                      .arg(deviceName, adapterId));
                    continue;
                }
            }
        }

        if (adapterHub()->adapterManager(adapterId) == nullptr)
        {
            problems.append(
              tr("Device %1 (\"%2\"): adapter '%3' is not available").arg(devId).arg(deviceName, adapterId));
        }
    }

    if (problems.isEmpty())
    {
        return QString();
    }

    return tr("Logging cannot start because the following devices are misconfigured:\n%1").arg(problems.join('\n'));
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
 * \brief Handle a fatal adapter session failure by stopping the session and surfacing the error
 * \param message Human-readable error description from AdapterPoll::communicationError
 */
void ScopeController::onCommunicationError(const QString& message)
{
    stop();
    emit errorOccurred(message);
}

/*!
 * \brief Reset communication stats, graph data and notes for a new logging session
 */
void ScopeController::clear()
{
    _pCommunicationStats->resetTiming();
    _pAdapterPoll->resetCommunicationStats();

    // Authoritative model-side clear, also needed by a future headless caller with no GraphView.
    // GraphView::clearResults(), triggered below via dataCleared(), redundantly clears the same
    // data for currently-plotted graphs — harmless overlap.
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
