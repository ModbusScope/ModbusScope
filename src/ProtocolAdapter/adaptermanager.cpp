
#include "ProtocolAdapter/adaptermanager.h"

#include "ProtocolAdapter/adapterclient.h"
#include "ProtocolAdapter/adapterprocess.h"
#include "models/settingsmodel.h"
#include "util/scopelogging.h"

#include <memory>

AdapterManager::AdapterManager(const QString& adapterId,
                               const QString& adapterBinaryPath,
                               SettingsModel* pSettingsModel,
                               QObject* parent)
    : QObject(parent), _adapterId(adapterId), _adapterBinaryPath(adapterBinaryPath), _pSettingsModel(pSettingsModel)
{
    _pAdapterClient = new AdapterClient(std::make_unique<AdapterProcess>(), _adapterId, this);

    connect(_pAdapterClient, &AdapterClient::sessionStarted, this, &AdapterManager::sessionStarted);
    connect(_pAdapterClient, &AdapterClient::readDataResult, this, &AdapterManager::readDataResult);
    connect(_pAdapterClient, &AdapterClient::buildExpressionResult, this, &AdapterManager::buildExpressionResult);
    connect(_pAdapterClient, &AdapterClient::expressionHelpResult, this, &AdapterManager::expressionHelpResult);
    connect(_pAdapterClient, &AdapterClient::describeDataPointResult, this, &AdapterManager::describeDataPointResult);
    connect(_pAdapterClient, &AdapterClient::sessionStopped, this, &AdapterManager::sessionStopped);
    connect(_pAdapterClient, &AdapterClient::sessionError, this, &AdapterManager::sessionError);
    connect(_pAdapterClient, &AdapterClient::adapterReady, this, &AdapterManager::adapterReady);
    connect(_pAdapterClient, &AdapterClient::describeResult, this, &AdapterManager::onDescribeResult);
    connect(_pAdapterClient, &AdapterClient::dataPointSchemaResult, this, &AdapterManager::onDataPointSchemaResult);
    connect(_pAdapterClient, &AdapterClient::diagnosticReceived, this, &AdapterManager::onAdapterDiagnostic);
}

/*! \brief Launch the adapter subprocess and start the initialization handshake.
 *
 * Resolves the adapter binary relative to the running executable so the path
 * is correct in the build tree, AppImage, and installed layouts alike.
 */
void AdapterManager::initAdapter()
{
    _pAdapterClient->prepareAdapter(_adapterBinaryPath);
}

/*! \brief Fetch the effective adapter config from the settings model and start the session.
 * \param registerExpressions Register expression strings to pass to the adapter.
 */
void AdapterManager::startSession(const QStringList& registerExpressions)
{
    const AdapterData* data = _pSettingsModel->adapterData(_adapterId);
    if (data == nullptr)
    {
        qCWarning(scopeComm) << "AdapterManager: no adapter data found for" << _adapterId;
        emit sessionError(QStringLiteral("No adapter configuration found for '%1'").arg(_adapterId));
        return;
    }
    QJsonObject config = data->effectiveConfig();
    _pAdapterClient->provideConfig(config, registerExpressions);
}

/*! \brief Returns the identifier string of the active adapter. */
QString AdapterManager::adapterId() const
{
    return _adapterId;
}

/*! \brief Returns true when the adapter is ready to accept provideConfig() (AWAITING_CONFIG state). */
bool AdapterManager::isAdapterReady() const
{
    return _pAdapterClient->isReady();
}

/*! \brief Returns true when the adapter process is not running (IDLE state). */
bool AdapterManager::isAdapterIdle() const
{
    return _pAdapterClient->isIdle();
}

/*! \brief Returns true when the adapter has an established session (ACTIVE state). */
bool AdapterManager::isAdapterActive() const
{
    return _pAdapterClient->isActive();
}

/*! \brief Send an adapter.expressionHelp request to retrieve expression syntax help text. */
void AdapterManager::requestExpressionHelp()
{
    _pAdapterClient->requestExpressionHelp();
}

/*! \brief Send an adapter.describeDataPoint request to retrieve a human-readable description of a data point
 * expression.
 * \param expression The data point expression string to describe.
 */
void AdapterManager::describeDataPoint(const QString& expression)
{
    _pAdapterClient->describeDataPoint(expression);
}

/*! \brief Send adapter.stop to pause polling and keep the adapter process alive. */
void AdapterManager::stopSession()
{
    _pAdapterClient->stopSession();
}

/*! \brief Send an adapter.readData request to the active adapter. */
void AdapterManager::requestReadData()
{
    _pAdapterClient->requestReadData();
}

/*!
 * \brief Request the adapter to construct a register expression from its component parts.
 * \param addressFields Address field values from the register schema form.
 * \param dataType      Data type identifier; empty string uses the adapter default.
 * \param deviceId      Device identifier; 0 uses the adapter default.
 */
void AdapterManager::buildExpression(const QJsonObject& addressFields, const QString& dataType, deviceId_t deviceId)
{
    _pAdapterClient->buildExpression(addressFields, dataType, deviceId);
}

/*! \brief Receive the adapter.describe response, update the settings model, and request the data point schema.
 * \param description The full describe result object from the adapter.
 */
void AdapterManager::onDescribeResult(const QJsonObject& description)
{
    _pSettingsModel->updateAdapterFromDescribe(_adapterId, description);
    _pAdapterClient->requestDataPointSchema();
}

/*! \brief Receive the adapter data point schema and forward it to the settings model.
 * \param schema The data point schema JSON object returned by adapter.dataPointSchema.
 */
void AdapterManager::onDataPointSchemaResult(const QJsonObject& schema)
{
    _pSettingsModel->setAdapterDataPointSchema(_adapterId, schema);
}

/*!
 * \brief Route an adapter.diagnostic notification to the diagnostics log.
 *
 * Maps the adapter's level string to the appropriate Qt logging severity so
 * the message flows through ScopeLogging into DiagnosticModel.
 *
 * \param level   Severity string from the adapter: "debug", "info", "warning", or "error".
 * \param message The diagnostic message text.
 */
void AdapterManager::onAdapterDiagnostic(const QString& level, const QString& message)
{
    if (level == QStringLiteral("debug"))
    {
        qCDebug(scopeAdapter) << qUtf8Printable(message);
    }
    else if (level == QStringLiteral("info"))
    {
        qCInfo(scopeAdapter) << qUtf8Printable(message);
    }
    else if (level == QStringLiteral("warning"))
    {
        qCWarning(scopeAdapter) << qUtf8Printable(message);
    }
    else if (level == QStringLiteral("error"))
    {
        qCCritical(scopeAdapter) << qUtf8Printable(message);
    }
    else
    {
        qCWarning(scopeAdapter) << qUtf8Printable(
          QString("AdapterClient: unknown diagnostic level: %1 - %2").arg(level, message));
    }
}
