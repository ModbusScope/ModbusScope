
#include "ProtocolAdapter/adaptermanager.h"

#include "ProtocolAdapter/adapterclient.h"
#include "ProtocolAdapter/adapterprocess.h"
#include "models/settingsmodel.h"
#include "util/scopelogging.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

AdapterManager::AdapterManager(SettingsModel* pSettingsModel, QObject* parent)
    : QObject(parent), _pSettingsModel(pSettingsModel)
{
    _pAdapterProcess = new AdapterProcess(this);
    _pAdapterClient = new AdapterClient(_pAdapterProcess, this);

    connect(_pAdapterClient, &AdapterClient::sessionStarted, this, &AdapterManager::sessionStarted);
    connect(_pAdapterClient, &AdapterClient::readDataResult, this, &AdapterManager::readDataResult);
    connect(_pAdapterClient, &AdapterClient::buildExpressionResult, this, &AdapterManager::buildExpressionResult);
    connect(_pAdapterClient, &AdapterClient::expressionHelpResult, this, &AdapterManager::expressionHelpResult);
    connect(_pAdapterClient, &AdapterClient::sessionStopped, this, &AdapterManager::sessionStopped);
    connect(_pAdapterClient, &AdapterClient::sessionError, this, &AdapterManager::sessionError);
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
    const QString suffix = QFileInfo(QCoreApplication::applicationFilePath()).suffix();
    const QString adapterName = "modbusadapter" + (suffix.isEmpty() ? QString() : "." + suffix);
    const QString adapterPath = QDir(QCoreApplication::applicationDirPath()).filePath(adapterName);
    _pAdapterClient->prepareAdapter(adapterPath);
}

/*! \brief Fetch the effective adapter config from the settings model and start the session.
 * \param registerExpressions Register expression strings to pass to the adapter.
 */
void AdapterManager::startSession(const QStringList& registerExpressions)
{
    const AdapterData* data = _pSettingsModel->adapterData("modbus");
    QJsonObject config = data->effectiveConfig();
    _pAdapterClient->provideConfig(config, registerExpressions);
}

/*! \brief Send an adapter.expressionHelp request to retrieve expression syntax help text. */
void AdapterManager::requestExpressionHelp()
{
    _pAdapterClient->requestExpressionHelp();
}

/*! \brief Send adapter.shutdown and signal the adapter process to stop. */
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
    _pSettingsModel->updateAdapterFromDescribe("modbus", description);
    _pAdapterClient->requestDataPointSchema();
}

/*! \brief Receive the adapter data point schema and forward it to the settings model.
 * \param schema The data point schema JSON object returned by adapter.dataPointSchema.
 */
void AdapterManager::onDataPointSchemaResult(const QJsonObject& schema)
{
    _pSettingsModel->setAdapterDataPointSchema("modbus", schema);
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
