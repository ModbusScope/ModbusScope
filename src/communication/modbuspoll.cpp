
#include "communication/modbuspoll.h"

#include "models/settingsmodel.h"
#include "util/formatdatetime.h"
#include "util/scopelogging.h"

#include <QCoreApplication>
#include <QJsonArray>

ModbusPoll::ModbusPoll(SettingsModel* pSettingsModel, QObject* parent) : QObject(parent), _bPollActive(false)
{
    _pPollTimer = new QTimer(this);
    _pPollTimer->setSingleShot(true);
    connect(_pPollTimer, &QTimer::timeout, this, &ModbusPoll::triggerRegisterRead);

    _pSettingsModel = pSettingsModel;
    _lastPollStart = QDateTime::currentMSecsSinceEpoch();

    _pAdapterProcess = new AdapterProcess(this);
    _pAdapterClient = new AdapterClient(_pAdapterProcess, this);

    connect(_pAdapterClient, &AdapterClient::sessionStarted, this, &ModbusPoll::triggerRegisterRead);
    connect(_pAdapterClient, &AdapterClient::readDataResult, this, &ModbusPoll::onReadDataResult);
    connect(_pAdapterClient, &AdapterClient::describeResult, this, &ModbusPoll::onDescribeResult);
    connect(_pAdapterClient, &AdapterClient::registerSchemaResult, this, &ModbusPoll::onRegisterSchemaResult);
    connect(_pAdapterClient, &AdapterClient::buildExpressionResult, this, &ModbusPoll::buildExpressionResult);
    connect(_pAdapterClient, &AdapterClient::sessionError, this, [this](QString message) {
        qCWarning(scopeComm) << "AdapterClient error:" << message;
        _bPollActive = false;
        disconnect(_pAdapterClient, &AdapterClient::sessionStopped, this, &ModbusPoll::initAdapter);
    });
    connect(_pAdapterClient, &AdapterClient::sessionStopped, this, &ModbusPoll::initAdapter);
    connect(_pAdapterClient, &AdapterClient::diagnosticReceived, this, &ModbusPoll::onAdapterDiagnostic);
}

ModbusPoll::~ModbusPoll() = default;

/*! \brief Prepare the protocol adapter subprocess for use.
 *
 * Resolves the adapter binary relative to the running executable so the path
 * is correct in the build tree, AppImage, and installed layouts alike.
 * Calls prepareAdapter() on the client, which triggers the adapter.describe
 * handshake.
 */
void ModbusPoll::initAdapter()
{
    const QString adapterPath = QCoreApplication::applicationDirPath() + "/modbusadapter";
    _pAdapterClient->prepareAdapter(adapterPath);
}

void ModbusPoll::startCommunication(QList<DataPoint>& registerList)
{
    _registerList = registerList;
    _bPollActive = true;

    /* Re-establish auto-restart in case it was disconnected by a prior session error */
    disconnect(_pAdapterClient, &AdapterClient::sessionStopped, this, &ModbusPoll::initAdapter);
    connect(_pAdapterClient, &AdapterClient::sessionStopped, this, &ModbusPoll::initAdapter);

    qCInfo(scopeComm) << QString("Start logging: %1").arg(FormatDateTime::currentDateTime());

    resetCommunicationStats();

    QStringList expressions = buildRegisterExpressions(_registerList);

    const AdapterData* data = _pSettingsModel->adapterData("modbus");
    QJsonObject config = data->effectiveConfig();

    _pAdapterClient->provideConfig(config, expressions);
}

void ModbusPoll::resetCommunicationStats()
{
    _lastPollStart = QDateTime::currentMSecsSinceEpoch();
}

void ModbusPoll::stopCommunication()
{
    _bPollActive = false;
    _pPollTimer->stop();
    _pAdapterClient->stopSession();

    qCInfo(scopeComm) << QString("Stop logging: %1").arg(FormatDateTime::currentDateTime());
}

bool ModbusPoll::isActive()
{
    return _bPollActive;
}

void ModbusPoll::triggerRegisterRead()
{
    if (_bPollActive)
    {
        _lastPollStart = QDateTime::currentMSecsSinceEpoch();
        _pAdapterClient->requestReadData();
    }
}

void ModbusPoll::onReadDataResult(ResultDoubleList results)
{
    emit registerDataReady(results);

    if (_bPollActive)
    {
        const quint32 passedInterval = static_cast<quint32>(QDateTime::currentMSecsSinceEpoch() - _lastPollStart);
        uint waitInterval;

        if (passedInterval > _pSettingsModel->pollTime())
        {
            waitInterval = 1;
        }
        else
        {
            waitInterval = _pSettingsModel->pollTime() - passedInterval;
        }

        _pPollTimer->start(static_cast<int>(waitInterval));
    }
}

void ModbusPoll::onDescribeResult(const QJsonObject& description)
{
    _pSettingsModel->updateAdapterFromDescribe("modbus", description);
    _pAdapterClient->requestRegisterSchema();
}

/*! \brief Receive the adapter register schema and forward it to the settings model.
 * \param schema The register schema JSON object returned by adapter.registerSchema.
 */
void ModbusPoll::onRegisterSchemaResult(const QJsonObject& schema)
{
    _pSettingsModel->setAdapterRegisterSchema("modbus", schema);
}

/*! \brief Route an adapter.diagnostic notification to the diagnostics log.
 *
 * Maps the adapter's level string to the appropriate Qt logging severity so
 * the message flows through ScopeLogging into DiagnosticModel.
 *
 * \param level Severity string from the adapter: "debug", "info", "warning", or "error".
 * \param message The diagnostic message text.
 */
void ModbusPoll::onAdapterDiagnostic(const QString& level, const QString& message)
{
    if (level == QStringLiteral("debug"))
    {
        qCDebug(scopeAdapter) << message;
    }
    else if (level == QStringLiteral("info"))
    {
        qCInfo(scopeAdapter) << message;
    }
    else if (level == QStringLiteral("warning"))
    {
        qCWarning(scopeAdapter) << message;
    }
    else if (level == QStringLiteral("error"))
    {
        qCCritical(scopeAdapter) << message;
    }
    else
    {
        qCWarning(scopeAdapter) << "AdapterClient: unknown diagnostic level:" << level << "-" << message;
    }
}

/*!
 * \brief Request the adapter to construct a register expression from its component parts.
 * \param addressFields Address field values from the register schema form.
 * \param dataType      Data type identifier; empty string uses the adapter default.
 * \param deviceId      Device identifier; 0 uses the adapter default.
 */
void ModbusPoll::buildExpression(const QJsonObject& addressFields, const QString& dataType, deviceId_t deviceId)
{
    _pAdapterClient->buildExpression(addressFields, dataType, deviceId);
}

QStringList ModbusPoll::buildRegisterExpressions(const QList<DataPoint>& registerList)
{
    QStringList expressions;
    for (const DataPoint& reg : registerList)
    {
        expressions.append(reg.address());
    }
    return expressions;
}
