#include "ProtocolAdapter/adapterclient.h"

#include "util/scopelogging.h"

#include <QJsonArray>

AdapterClient::AdapterClient(AdapterProcess* pProcess, QObject* parent, int handshakeTimeoutMs)
    : QObject(parent), _pProcess(pProcess), _handshakeTimeoutMs(handshakeTimeoutMs)
{
    Q_ASSERT(pProcess);
    _pProcess->setParent(this);

    _handshakeTimer.setSingleShot(true);
    connect(&_handshakeTimer, &QTimer::timeout, this, &AdapterClient::onHandshakeTimeout);

    connect(_pProcess, &AdapterProcess::responseReceived, this, &AdapterClient::onResponseReceived);
    connect(_pProcess, &AdapterProcess::errorReceived, this, &AdapterClient::onErrorReceived);
    connect(_pProcess, &AdapterProcess::processError, this, &AdapterClient::onProcessError);
    connect(_pProcess, &AdapterProcess::processFinished, this, &AdapterClient::onProcessFinished);
    connect(_pProcess, &AdapterProcess::notificationReceived, this, &AdapterClient::onNotificationReceived);
}

AdapterClient::~AdapterClient() = default;

void AdapterClient::prepareAdapter(const QString& adapterPath)
{
    if (_state != State::IDLE)
    {
        qCWarning(scopeComm) << "AdapterClient: prepareAdapter called in non-idle state";
        return;
    }

    if (!_pProcess->start(adapterPath))
    {
        return;
    }

    qCInfo(scopeComm) << "AdapterClient: process started, sending initialize";
    _state = State::INITIALIZING;
    _handshakeTimer.start(_handshakeTimeoutMs);
    _pProcess->sendRequest("adapter.initialize", QJsonObject());
}

void AdapterClient::provideConfig(QJsonObject config, QStringList registerExpressions)
{
    if (_state != State::AWAITING_CONFIG)
    {
        qCWarning(scopeComm) << "AdapterClient: provideConfig called in unexpected state" << static_cast<int>(_state);
        return;
    }

    _pendingExpressions = registerExpressions;
    _pendingConfig = config;
    _state = State::CONFIGURING;
    _handshakeTimer.start(_handshakeTimeoutMs);
    QJsonObject params;
    params["config"] = _pendingConfig;
    _pProcess->sendRequest("adapter.configure", params);
}

void AdapterClient::requestReadData()
{
    if (_state != State::ACTIVE)
    {
        qCWarning(scopeComm) << "AdapterClient: requestReadData called in non-active state";
        return;
    }

    _pProcess->sendRequest("adapter.readData", QJsonObject());
}

void AdapterClient::requestStatus()
{
    if (_state != State::ACTIVE)
    {
        qCWarning(scopeComm) << "AdapterClient: requestStatus called in non-active state";
        return;
    }

    _pProcess->sendRequest("adapter.getStatus", QJsonObject());
}

void AdapterClient::requestDataPointSchema()
{
    if (_state != State::AWAITING_CONFIG)
    {
        qCWarning(scopeComm) << "AdapterClient: requestDataPointSchema called in unexpected state"
                             << static_cast<int>(_state);
        return;
    }

    _pProcess->sendRequest("adapter.dataPointSchema", QJsonObject());
}

void AdapterClient::describeDataPoint(const QString& expression)
{
    if (_state != State::AWAITING_CONFIG && _state != State::ACTIVE)
    {
        qCWarning(scopeComm) << "AdapterClient: describeDataPoint called in unexpected state"
                             << static_cast<int>(_state);
        return;
    }

    QJsonObject params;
    params["expression"] = expression;
    _pProcess->sendRequest("adapter.describeDataPoint", params);
}

void AdapterClient::validateDataPoint(const QString& expression)
{
    if (_state != State::AWAITING_CONFIG && _state != State::ACTIVE)
    {
        qCWarning(scopeComm) << "AdapterClient: validateDataPoint called in unexpected state"
                             << static_cast<int>(_state);
        return;
    }

    QJsonObject params;
    params["expression"] = expression;
    _pProcess->sendRequest("adapter.validateDataPoint", params);
}

/*!
 * \brief Send an adapter.buildExpression request to construct a data point expression string.
 * \param addressFields Address field values as returned by the data point schema form.
 * \param dataType      Data type identifier; omitted from params when empty.
 * \param deviceId      Device identifier; omitted from params when zero.
 */
void AdapterClient::buildExpression(const QJsonObject& addressFields, const QString& dataType, deviceId_t deviceId)
{
    if (_state != State::AWAITING_CONFIG && _state != State::ACTIVE)
    {
        qCWarning(scopeComm) << "AdapterClient: buildExpression called in unexpected state" << static_cast<int>(_state);
        return;
    }

    QJsonObject params;
    params["fields"] = addressFields;
    if (!dataType.isEmpty())
    {
        params["dataType"] = dataType;
    }
    if (deviceId != 0)
    {
        params["deviceId"] = static_cast<qint64>(deviceId);
    }
    _pProcess->sendRequest("adapter.buildExpression", params);
}

void AdapterClient::stopSession()
{
    if (_state == State::IDLE || _state == State::STOPPING)
    {
        return;
    }

    _handshakeTimer.stop();

    if (_state == State::ACTIVE || _state == State::STARTING)
    {
        _state = State::STOPPING;
        _pProcess->sendRequest("adapter.shutdown", QJsonObject());
        _handshakeTimer.start(_handshakeTimeoutMs);
    }
    else
    {
        _state = State::STOPPING;
        _pProcess->stop();
        /* sessionStopped is emitted from onProcessFinished once the process exits */
    }
}

void AdapterClient::onResponseReceived(int id, const QString& method, const QJsonValue& result)
{
    Q_UNUSED(id)
    if (result.isObject())
    {
        handleLifecycleResponse(method, result.toObject());
    }
    else
    {
        qCWarning(scopeComm) << "AdapterClient: unexpected non-object result for" << method;
        _handshakeTimer.stop();
        /* Set IDLE before stop() so onProcessFinished's IDLE guard suppresses any
           duplicate sessionError emission when the process exits asynchronously. */
        _state = State::IDLE;
        _pProcess->stop();
        emit sessionError(QString("Unexpected non-object result for %1").arg(method));
    }
}

void AdapterClient::onErrorReceived(int id, const QString& method, const QJsonObject& error)
{
    Q_UNUSED(id)
    _handshakeTimer.stop();
    QString errorMsg = error.value("message").toString();
    qCWarning(scopeComm) << "AdapterClient: error for" << method << ":" << errorMsg;

    State previousState = _state;
    /* Set IDLE before stop() so onProcessFinished's IDLE guard suppresses any
       duplicate sessionError emission when the process exits asynchronously. */
    _state = State::IDLE;
    _pProcess->stop();

    if (previousState != State::STOPPING)
    {
        emit sessionError(QString("Adapter error on %1: %2").arg(method, errorMsg));
    }
}

void AdapterClient::onProcessError(const QString& message)
{
    _handshakeTimer.stop();
    if (_state != State::STOPPING)
    {
        _state = State::IDLE;
        emit sessionError(message);
    }
}

void AdapterClient::onProcessFinished()
{
    _handshakeTimer.stop();
    if (_state == State::STOPPING)
    {
        _state = State::IDLE;
        emit sessionStopped();
    }
    else if (_state != State::IDLE)
    {
        _state = State::IDLE;
        emit sessionError("Adapter process exited unexpectedly");
    }
}

void AdapterClient::onHandshakeTimeout()
{
    qCWarning(scopeComm) << "AdapterClient: handshake timed out in state" << static_cast<int>(_state);
    bool wasStopping = (_state == State::STOPPING);
    _state = State::IDLE;
    _pProcess->stop();
    if (wasStopping)
    {
        emit sessionStopped();
    }
    else
    {
        emit sessionError("Adapter handshake timed out");
    }
}

void AdapterClient::onNotificationReceived(QString method, QJsonValue params)
{
    if (method != QStringLiteral("adapter.diagnostic"))
    {
        return;
    }

    if (!params.isObject())
    {
        qCWarning(scopeComm) << "AdapterClient: adapter.diagnostic params is not an object";
        return;
    }

    QJsonObject obj = params.toObject();
    emit diagnosticReceived(obj.value(QStringLiteral("level")).toString(),
                            obj.value(QStringLiteral("message")).toString());
}

void AdapterClient::handleLifecycleResponse(const QString& method, const QJsonObject& result)
{
    if (method == "adapter.initialize" && _state == State::INITIALIZING)
    {
        qCInfo(scopeComm) << "AdapterClient: initialized, sending describe";
        _state = State::DESCRIBING;
        _pProcess->sendRequest("adapter.describe", QJsonObject());
    }
    else if (method == "adapter.describe" && _state == State::DESCRIBING)
    {
        qCInfo(scopeComm) << "AdapterClient: described, awaiting config";
        _handshakeTimer.stop();
        _state = State::AWAITING_CONFIG;
        emit describeResult(result);
    }
    else if (method == "adapter.configure" && _state == State::CONFIGURING)
    {
        qCInfo(scopeComm) << "AdapterClient: configured, sending start";
        _state = State::STARTING;
        QJsonObject params;
        params["registers"] = QJsonArray::fromStringList(_pendingExpressions);
        _pProcess->sendRequest("adapter.start", params);
    }
    else if (method == "adapter.start" && _state == State::STARTING)
    {
        qCInfo(scopeComm) << "AdapterClient: started";
        _handshakeTimer.stop();
        _state = State::ACTIVE;
        emit sessionStarted();
    }
    else if (method == "adapter.readData" && _state == State::ACTIVE)
    {
        ResultDoubleList results;
        const QJsonArray registers = result["registers"].toArray();
        for (const QJsonValue& entry : registers)
        {
            QJsonObject reg = entry.toObject();
            if (reg["valid"].toBool())
            {
                results.append(ResultDouble(reg["value"].toDouble(), ResultState::State::SUCCESS));
            }
            else
            {
                results.append(ResultDouble(0.0, ResultState::State::INVALID));
            }
        }
        emit readDataResult(results);
    }
    else if (method == "adapter.getStatus" && _state == State::ACTIVE)
    {
        emit statusResult(result["active"].toBool());
    }
    else if (method == "adapter.shutdown" && _state == State::STOPPING)
    {
        qCInfo(scopeComm) << "AdapterClient: shutdown acknowledged";
        _pProcess->stop();
        /* sessionStopped is emitted from onProcessFinished once the process exits */
    }
    else if (method == "adapter.dataPointSchema" && _state == State::AWAITING_CONFIG)
    {
        emit dataPointSchemaResult(result);
    }
    else if (method == "adapter.describeDataPoint" && (_state == State::AWAITING_CONFIG || _state == State::ACTIVE))
    {
        emit describeDataPointResult(result);
    }
    else if (method == "adapter.validateDataPoint" && (_state == State::AWAITING_CONFIG || _state == State::ACTIVE))
    {
        emit validateDataPointResult(result["valid"].toBool(), result["error"].toString());
    }
    else if (method == "adapter.buildExpression" && (_state == State::AWAITING_CONFIG || _state == State::ACTIVE))
    {
        emit buildExpressionResult(result["expression"].toString());
    }
    else
    {
        qCWarning(scopeComm) << "AdapterClient: unexpected response for" << method << "in state"
                             << static_cast<int>(_state);
    }
}
