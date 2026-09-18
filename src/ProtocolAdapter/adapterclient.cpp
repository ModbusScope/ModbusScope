#include "ProtocolAdapter/adapterclient.h"

#include "util/scopelogging.h"

#include <QJsonArray>

#include <memory>

namespace {

/*!
 * \brief Extracts the string entries of a readData data point's "flags" value.
 *
 * A missing or non-array "flags" value, or a non-string entry within it, yields no candidate
 * for that entry rather than a crash — malformed input degrades gracefully into "no flags"
 * instead of being rejected outright.
 */
QStringList flagIdCandidates(const QJsonValue& flagsValue)
{
    QStringList ids;
    if (!flagsValue.isArray())
    {
        return ids;
    }

    const QJsonArray flagsArray = flagsValue.toArray();
    for (const auto& entry : flagsArray)
    {
        if (entry.isString())
        {
            ids.append(entry.toString());
        }
    }
    return ids;
}

} // namespace

/*!
 * \brief Construct an AdapterClient driving \a pProcess.
 * \param pProcess          Transport for the adapter subprocess; ownership is taken.
 * \param adapterId         Identifier of the adapter this client talks to (e.g. "modbus", "iec104"),
 *                          used only to tag diagnostic log lines. May be empty.
 * \param parent            Optional QObject parent.
 * \param handshakeTimeoutMs Timeout in milliseconds for adapter handshake responses.
 */
AdapterClient::AdapterClient(std::unique_ptr<AdapterProcess> pProcess,
                             QString adapterId,
                             QObject* parent,
                             int handshakeTimeoutMs)
    : QObject(parent),
      _adapterId(std::move(adapterId)),
      _pProcess(std::move(pProcess)),
      _handshakeTimeoutMs(handshakeTimeoutMs)
{
    Q_ASSERT(_pProcess);

    _handshakeTimer.setSingleShot(true);
    connect(&_handshakeTimer, &QTimer::timeout, this, &AdapterClient::onHandshakeTimeout);

    connect(_pProcess.get(), &AdapterProcess::responseReceived, this, &AdapterClient::onResponseReceived);
    connect(_pProcess.get(), &AdapterProcess::errorReceived, this, &AdapterClient::onErrorReceived);
    connect(_pProcess.get(), &AdapterProcess::processError, this, &AdapterClient::onProcessError);
    connect(_pProcess.get(), &AdapterProcess::processFinished, this, &AdapterClient::onProcessFinished);
    connect(_pProcess.get(), &AdapterProcess::notificationReceived, this, &AdapterClient::onNotificationReceived);
}

AdapterClient::~AdapterClient() = default;

bool AdapterClient::isReady() const
{
    return _state == State::AWAITING_CONFIG;
}

bool AdapterClient::isIdle() const
{
    return _state == State::IDLE;
}

bool AdapterClient::isActive() const
{
    return _state == State::ACTIVE || _state == State::ACTIVE_DEGRADED;
}

void AdapterClient::prepareAdapter(const QString& adapterPath)
{
    if (_state != State::IDLE)
    {
        qCWarning(scopeComm) << "AdapterClient:" << _adapterId << "prepareAdapter called in non-idle state";
        return;
    }

    if (!_pProcess->start(adapterPath))
    {
        return;
    }

    qCInfo(scopeComm) << "AdapterClient:" << _adapterId << "process started, sending initialize";
    _state = State::INITIALIZING;
    _incompatibilityReason.clear();
    _handshakeTimer.start(_handshakeTimeoutMs);
    QJsonObject params;
    params["protocolVersion"] = cProtocolVersion;
    _pProcess->sendRequest("adapter.initialize", params);
}

void AdapterClient::provideConfig(QJsonObject config, QStringList registerExpressions)
{
    if (_state != State::AWAITING_CONFIG)
    {
        qCWarning(scopeComm) << "AdapterClient:" << _adapterId << "provideConfig called in unexpected state"
                             << static_cast<int>(_state);
        return;
    }

    _pendingExpressions = registerExpressions;
    _reportedUnrecognisedReadDataState = false;
    _reportedMalformedReadDataPoint = false;
    _reportedUnknownFlagIds.clear();

    if (!_incompatibilityReason.isEmpty())
    {
        _state = State::ACTIVE_DEGRADED;
        _incompatibleAnnouncePending = true;
        QMetaObject::invokeMethod(this, &AdapterClient::announceIncompatibleSession, Qt::QueuedConnection);
        return;
    }

    _pendingConfig = config;
    _pendingAuxRequests.clear();
    _state = State::CONFIGURING;
    _handshakeTimer.start(_handshakeTimeoutMs);
    QJsonObject params;
    params.insert("config", _pendingConfig);
    _pProcess->sendRequest("adapter.configure", params);
}

void AdapterClient::requestReadData()
{
    if (_state != State::ACTIVE && _state != State::ACTIVE_DEGRADED)
    {
        qCWarning(scopeComm) << "AdapterClient:" << _adapterId << "requestReadData called in non-active state";
        return;
    }

    if (_state == State::ACTIVE_DEGRADED)
    {
        emit readDataResult(invalidResults());
        return;
    }

    _pProcess->sendRequest("adapter.readData", QJsonObject());
}

void AdapterClient::requestStatus()
{
    if (_state != State::ACTIVE && _state != State::ACTIVE_DEGRADED)
    {
        qCWarning(scopeComm) << "AdapterClient:" << _adapterId << "requestStatus called in non-active state";
        return;
    }

    if (_state == State::ACTIVE_DEGRADED)
    {
        emit statusResult(false);
        return;
    }

    _pProcess->sendRequest("adapter.getStatus", QJsonObject());
}

/*!
 * \brief Request the adapter's data point schema while awaiting configuration.
 */
void AdapterClient::requestDataPointSchema()
{
    if (_state != State::AWAITING_CONFIG)
    {
        qCWarning(scopeComm) << "AdapterClient:" << _adapterId << "requestDataPointSchema called in unexpected state"
                             << static_cast<int>(_state);
        return;
    }

    if (isAuxRequestRefused("requestDataPointSchema"))
    {
        return;
    }

    _pendingAuxRequests["adapter.dataPointSchema"] = _pProcess->sendRequest("adapter.dataPointSchema", QJsonObject());
}

/*!
 * \brief Request a human-readable description of a data point expression.
 * \param expression The data point expression string to describe.
 */
void AdapterClient::describeDataPoint(const QString& expression)
{
    if (_state != State::AWAITING_CONFIG && _state != State::ACTIVE && _state != State::ACTIVE_DEGRADED)
    {
        qCWarning(scopeComm) << "AdapterClient:" << _adapterId << "describeDataPoint called in unexpected state"
                             << static_cast<int>(_state);
        return;
    }

    if (isAuxRequestRefused("describeDataPoint"))
    {
        return;
    }

    QJsonObject params;
    params["expression"] = expression;
    _pendingAuxRequests["adapter.describeDataPoint"] = _pProcess->sendRequest("adapter.describeDataPoint", params);
}

/*!
 * \brief Validate a data point expression string via the adapter.
 * \param expression The data point expression string to validate.
 */
void AdapterClient::validateDataPoint(const QString& expression)
{
    if (_state != State::AWAITING_CONFIG && _state != State::ACTIVE && _state != State::ACTIVE_DEGRADED)
    {
        qCWarning(scopeComm) << "AdapterClient:" << _adapterId << "validateDataPoint called in unexpected state"
                             << static_cast<int>(_state);
        return;
    }

    if (isAuxRequestRefused("validateDataPoint"))
    {
        return;
    }

    QJsonObject params;
    params["expression"] = expression;
    _pendingAuxRequests["adapter.validateDataPoint"] = _pProcess->sendRequest("adapter.validateDataPoint", params);
}

/*!
 * \brief Send an adapter.buildExpression request to construct a data point expression string.
 * \param addressFields Address field values as returned by the data point schema form.
 * \param dataType      Data type identifier; omitted from params when empty.
 * \param deviceId      Device identifier; omitted from params when zero.
 */
void AdapterClient::buildExpression(const QJsonObject& addressFields, const QString& dataType, deviceId_t deviceId)
{
    if (_state != State::AWAITING_CONFIG && _state != State::ACTIVE && _state != State::ACTIVE_DEGRADED)
    {
        qCWarning(scopeComm) << "AdapterClient:" << _adapterId << "buildExpression called in unexpected state"
                             << static_cast<int>(_state);
        return;
    }

    if (isAuxRequestRefused("buildExpression"))
    {
        return;
    }

    QJsonObject params;
    params["fields"] = addressFields;
    const QString trimmedDataType = dataType.trimmed();
    if (!trimmedDataType.isEmpty())
    {
        params["dataType"] = trimmedDataType;
    }
    if (deviceId != 0)
    {
        params["deviceId"] = static_cast<qint64>(deviceId);
    }
    _pendingAuxRequests["adapter.buildExpression"] = _pProcess->sendRequest("adapter.buildExpression", params);
}

/*!
 * \brief Request expression syntax help text from the adapter.
 */
void AdapterClient::requestExpressionHelp()
{
    if (_state != State::AWAITING_CONFIG && _state != State::ACTIVE && _state != State::ACTIVE_DEGRADED)
    {
        qCWarning(scopeComm) << "AdapterClient:" << _adapterId << "requestExpressionHelp called in unexpected state"
                             << static_cast<int>(_state);
        return;
    }

    if (isAuxRequestRefused("requestExpressionHelp"))
    {
        return;
    }

    _pendingAuxRequests["adapter.expressionHelp"] = _pProcess->sendRequest("adapter.expressionHelp", QJsonObject());
}

void AdapterClient::stopSession()
{
    if (_state == State::IDLE || _state == State::STOPPING || _state == State::STOPPING_SESSION)
    {
        return;
    }

    _handshakeTimer.stop();
    _pendingAuxRequests.clear();
    _incompatibleAnnouncePending = false;

    if (_state == State::ACTIVE_DEGRADED)
    {
        /* No real session was ever established with the adapter (adapter.configure or
           adapter.start was rejected), so there is nothing to tell it to stop: transition
           locally exactly as a successful adapter.stop response would. */
        qCInfo(scopeComm) << "AdapterClient:" << _adapterId << "degraded session stopped locally, awaiting config";
        _state = State::AWAITING_CONFIG;
        emit sessionStopped();
        emit adapterReady();
    }
    else if (_state == State::ACTIVE)
    {
        _state = State::STOPPING_SESSION;
        _pProcess->sendRequest("adapter.stop", QJsonObject());
        _handshakeTimer.start(_handshakeTimeoutMs);
    }
    else
    {
        /* STARTING and all other mid-handshake states: force-kill.
           adapter.stop requires an established session (adapter.start acknowledged). */
        _state = State::STOPPING;
        _pProcess->stop();
        /* sessionStopped is emitted from onProcessFinished or onProcessError */
    }
}

void AdapterClient::onResponseReceived(int id, const QString& method, const QJsonValue& result)
{
    if (result.isObject())
    {
        handleLifecycleResponse(id, method, result.toObject());
    }
    else
    {
        qCWarning(scopeComm) << "AdapterClient:" << _adapterId << "unexpected non-object result for" << method;
        _handshakeTimer.stop();
        /* Set IDLE before stop() so onProcessFinished's IDLE guard suppresses any
           duplicate sessionError emission when the process exits asynchronously. */
        _pendingAuxRequests.clear();
        _state = State::IDLE;
        _pProcess->stop();
        emit sessionError(QString("Unexpected non-object result for %1").arg(method));
    }
}

void AdapterClient::onErrorReceived(int id, const QString& method, const QJsonObject& error)
{
    _handshakeTimer.stop();
    QString errorMsg = error.value("message").toString();
    qCWarning(scopeComm) << "AdapterClient:" << _adapterId << "error for" << method << ":" << errorMsg;

    /* For auxiliary requests, a JSON-RPC error is a non-fatal result rather
       than a session-level failure. Translate to the corresponding result signal or swallow. */
    if (method == QStringLiteral("adapter.validateDataPoint") &&
        (_state == State::AWAITING_CONFIG || _state == State::ACTIVE || _state == State::ACTIVE_DEGRADED))
    {
        if (_pendingAuxRequests.value(method, -1) == id)
        {
            _pendingAuxRequests.remove(method);
            emit validateDataPointResult(false, errorMsg);
        }
        return;
    }

    if (method == QStringLiteral("adapter.expressionHelp") &&
        (_state == State::AWAITING_CONFIG || _state == State::ACTIVE || _state == State::ACTIVE_DEGRADED))
    {
        if (_pendingAuxRequests.value(method, -1) == id)
        {
            _pendingAuxRequests.remove(method);
        }
        return;
    }

    if (method == QStringLiteral("adapter.readData"))
    {
        if (_state == State::ACTIVE)
        {
            emit readDataResult(invalidResults());
        }
        return;
    }

    /* A rejected adapter.configure (e.g. too many devices for an unlicensed session) is a
       configuration problem, not an adapter/process failure: the adapter stays alive.
       Treat the session as started but degraded, exactly like a rejected adapter.start below,
       so polling continues and every requestReadData() call reports invalid results instead of
       the subprocess being killed and the caller waiting indefinitely for a session that will
       never start. */
    if (method == QStringLiteral("adapter.configure") && _state == State::CONFIGURING)
    {
        degradeSession(QStringLiteral("Adapter rejected configuration: %1").arg(errorMsg));
        return;
    }

    /* A rejected adapter.start (e.g. an invalid data point expression) is a configuration
       problem, not an adapter/process failure: the adapter stays alive and configured.
       Treat the session as started but degraded, so polling continues and every
       requestReadData() call reports invalid results instead of tearing the session down. */
    if (method == QStringLiteral("adapter.start") && _state == State::STARTING)
    {
        degradeSession(QStringLiteral("Adapter rejected start: %1").arg(errorMsg));
        return;
    }

    State previousState = _state;
    /* Set IDLE before stop() so onProcessFinished's IDLE guard suppresses any
       duplicate sessionError emission when the process exits asynchronously. */
    _pendingAuxRequests.clear();
    _state = State::IDLE;
    _pProcess->stop();

    if (previousState == State::STOPPING || previousState == State::STOPPING_SESSION)
    {
        /* User-initiated stop: suppress the error and emit a clean stop signal. */
        emit sessionStopped();
    }
    else
    {
        emit sessionError(QString("Adapter error on %1: %2").arg(method, errorMsg));
    }
}

void AdapterClient::onProcessError(const QString& message)
{
    _handshakeTimer.stop();
    if (_state == State::STOPPING || _state == State::STOPPING_SESSION)
    {
        _pendingAuxRequests.clear();
        _state = State::IDLE;
        emit sessionStopped();
    }
    else if (_state != State::IDLE)
    {
        _pendingAuxRequests.clear();
        _state = State::IDLE;
        emit sessionError(message);
    }
}

void AdapterClient::onProcessFinished()
{
    _handshakeTimer.stop();
    _pendingAuxRequests.clear();
    if (_state == State::STOPPING || _state == State::STOPPING_SESSION)
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
    qCWarning(scopeComm) << "AdapterClient:" << _adapterId << "handshake timed out in state"
                         << static_cast<int>(_state);
    bool wasUserStop = (_state == State::STOPPING || _state == State::STOPPING_SESSION);
    _pendingAuxRequests.clear();
    _state = State::IDLE;
    _pProcess->stop();
    if (wasUserStop)
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
        qCWarning(scopeComm) << "AdapterClient:" << _adapterId << "adapter.diagnostic params is not an object";
        return;
    }

    QJsonObject obj = params.toObject();
    emit diagnosticReceived(obj.value(QStringLiteral("level")).toString(),
                            obj.value(QStringLiteral("message")).toString());
}

/*!
 * \brief Check that the pending aux response matches \a id and remove it from the map.
 * \return true if the response is current and was removed; false if stale (caller should discard).
 */
bool AdapterClient::consumeAuxResponse(const QString& method, int id)
{
    if (_pendingAuxRequests.value(method, -1) != id)
    {
        qCWarning(scopeComm) << "AdapterClient:" << _adapterId << "ignoring stale response for" << method;
        return false;
    }
    _pendingAuxRequests.remove(method);
    return true;
}

/*!
 * \brief Build one INVALID result per pending data point expression.
 * \return A ResultDoubleList the same size as the expressions passed to the current session.
 */
ResultDoubleList AdapterClient::invalidResults() const
{
    return ResultDoubleList(_pendingExpressions.size(), ResultDouble(0.0, DataQuality::State::Invalid));
}

/*!
 * \brief Decodes an adapter.readData result into one Result per data point.
 *
 * Strict per the protocol-2 contract (docs/technical/adapter-protocol-spec.md): an unrecognised
 * "state" string never falls back to good, a usable point without a numeric value is not
 * accepted as a value, and an unrecognised flag id is dropped rather than rejecting the point.
 * Each kind of violation raises one diagnostic per session (per distinct id for flags), so a
 * persistently misbehaving adapter does not flood the diagnostic log every poll.
 *
 * \param result The full adapter.readData result object.
 * \return One Result<double> per entry in "dataPoints", in order.
 */
ResultDoubleList AdapterClient::decodeReadDataResult(const QJsonObject& result)
{
    ResultDoubleList results;
    ReadDataIssues issues;

    const QJsonArray dataPoints = result["dataPoints"].toArray();
    for (const auto& entry : dataPoints)
    {
        results.append(decodeDataPoint(entry, issues));
    }

    if (issues.unrecognisedState && !_reportedUnrecognisedReadDataState)
    {
        _reportedUnrecognisedReadDataState = true;
        emit diagnosticReceived(QStringLiteral("error"),
                                QStringLiteral("Adapter '%1' sent an unrecognised adapter.readData state; "
                                               "treating the affected data point(s) as invalid")
                                  .arg(_adapterId));
    }

    if (issues.malformedPoint && !_reportedMalformedReadDataPoint)
    {
        _reportedMalformedReadDataPoint = true;
        emit diagnosticReceived(QStringLiteral("error"),
                                QStringLiteral("Adapter '%1' sent an adapter.readData data point that is not an "
                                               "object or has no numeric value; treating it as invalid")
                                  .arg(_adapterId));
    }

    if (!issues.newUnknownFlagIds.isEmpty())
    {
        emit diagnosticReceived(QStringLiteral("warning"),
                                QStringLiteral("Adapter '%1' sent unrecognised adapter.readData flag id(s): %2")
                                  .arg(_adapterId, issues.newUnknownFlagIds.join(QStringLiteral(", "))));
    }

    return results;
}

/*!
 * \brief Decodes a single adapter.readData "dataPoints" entry.
 * \param entry The entry as received; anything other than an object is malformed.
 * \param issues Updated with any contract violation found: an unrecognised state, a usable point
 * without a numeric value (or a non-object entry), and flag ids not recognised by
 * DataQuality::flagsFromIds() that this client has not reported yet.
 * \return The decoded Result. State defaults to Good when "state" is absent; a point that
 * violates the contract is Invalid. A Good point carrying flags is promoted to Degraded, so flags
 * never sit on an unflagged-looking Good.
 */
ResultDouble AdapterClient::decodeDataPoint(const QJsonValue& entry, ReadDataIssues& issues)
{
    if (!entry.isObject())
    {
        issues.malformedPoint = true;
        return ResultDouble(0.0, DataQuality::State::Invalid);
    }

    const QJsonObject dataPoint = entry.toObject();

    DataQuality::State state = DataQuality::State::Good;
    if (dataPoint.contains(QStringLiteral("state")))
    {
        const std::optional<DataQuality::State> parsed =
          DataQuality::stateFromId(dataPoint[QStringLiteral("state")].toString());
        if (parsed.has_value())
        {
            state = parsed.value();
        }
        else
        {
            state = DataQuality::State::Invalid;
            issues.unrecognisedState = true;
        }
    }

    const bool usable = (state == DataQuality::State::Good) || (state == DataQuality::State::Degraded);
    const QJsonValue valueEntry = dataPoint[QStringLiteral("value")];
    if (usable && !valueEntry.isDouble())
    {
        issues.malformedPoint = true;
        return ResultDouble(0.0, DataQuality::State::Invalid);
    }

    QStringList unknownIds;
    const DataQuality::Flags flags =
      DataQuality::flagsFromIds(flagIdCandidates(dataPoint[QStringLiteral("flags")]), unknownIds);
    for (const QString& id : std::as_const(unknownIds))
    {
        if (!_reportedUnknownFlagIds.contains(id))
        {
            _reportedUnknownFlagIds.insert(id);
            issues.newUnknownFlagIds.append(id);
        }
    }

    ResultDouble decoded(usable ? valueEntry.toDouble() : 0.0, state);
    decoded.addFlags(flags);
    return decoded;
}

/*!
 * \brief Reports a session that was degraded because the adapter's protocol version is incompatible.
 *
 * Runs queued rather than from provideConfig(): unlike the other degraded paths there is no RPC
 * round trip to make it asynchronous, and a synchronous sessionStarted() would reach the caller
 * while it is still starting the remaining adapters. Does nothing if the session was stopped in
 * the meantime, and announces each degraded session once even if it was stopped and started again
 * before this ran.
 */
void AdapterClient::announceIncompatibleSession()
{
    if (!_incompatibleAnnouncePending || _state != State::ACTIVE_DEGRADED)
    {
        return;
    }

    _incompatibleAnnouncePending = false;
    emit diagnosticReceived(QStringLiteral("error"), _incompatibilityReason);
    emit sessionStarted();
}

/*!
 * \brief Returns whether an auxiliary request (schema, describe, validate, build, help) must not be
 * sent because the adapter speaks an incompatible protocol version.
 *
 * Such an adapter is left unusable: its reply shapes are not guaranteed, and an unexpected reply or
 * error to one of these requests would be treated as fatal for the whole polling session.
 *
 * \param requestName Name of the request, for the log message.
 */
bool AdapterClient::isAuxRequestRefused(const char* requestName) const
{
    if (_incompatibilityReason.isEmpty())
    {
        return false;
    }

    qCWarning(scopeComm) << "AdapterClient:" << _adapterId << requestName
                         << "not sent, adapter has an incompatible protocol version";
    return true;
}

/*!
 * \brief Transition into ACTIVE_DEGRADED and notify callers that the session is started-but-broken.
 * \param diagnosticMessage Human-readable description of the rejected setup RPC, forwarded via diagnosticReceived().
 */
void AdapterClient::degradeSession(const QString& diagnosticMessage)
{
    _state = State::ACTIVE_DEGRADED;
    emit diagnosticReceived(QStringLiteral("error"), diagnosticMessage);
    emit sessionStarted();
}

void AdapterClient::handleLifecycleResponse(int id, const QString& method, const QJsonObject& result)
{
    if (method == "adapter.initialize" && _state == State::INITIALIZING)
    {
        qCInfo(scopeComm) << "AdapterClient:" << _adapterId << "initialized, sending describe";
        _state = State::DESCRIBING;
        _pProcess->sendRequest("adapter.describe", QJsonObject());
    }
    else if (method == "adapter.describe" && _state == State::DESCRIBING)
    {
        const int adapterProtocolVersion = result["protocolVersion"].toInt(-1);
        if (adapterProtocolVersion != cProtocolVersion)
        {
            qCWarning(scopeComm) << "AdapterClient:" << _adapterId << "protocol version mismatch: found"
                                 << adapterProtocolVersion << "required" << cProtocolVersion;
            _incompatibilityReason = QStringLiteral("Adapter '%1' speaks protocol version %2; this application "
                                                    "requires version %3")
                                       .arg(_adapterId)
                                       .arg(adapterProtocolVersion)
                                       .arg(cProtocolVersion);
            emit diagnosticReceived(QStringLiteral("error"), _incompatibilityReason);
        }

        qCInfo(scopeComm) << "AdapterClient:" << _adapterId << "described, awaiting config";
        _handshakeTimer.stop();
        _state = State::AWAITING_CONFIG;
        emit describeResult(result);
        emit adapterReady();
    }
    else if (method == "adapter.configure" && _state == State::CONFIGURING)
    {
        qCInfo(scopeComm) << "AdapterClient:" << _adapterId << "configured, sending start";
        _state = State::STARTING;
        QJsonObject params;
        params["dataPoints"] = QJsonArray::fromStringList(_pendingExpressions);
        _pProcess->sendRequest("adapter.start", params);
    }
    else if (method == "adapter.start" && _state == State::STARTING)
    {
        qCInfo(scopeComm) << "AdapterClient:" << _adapterId << "started";
        _handshakeTimer.stop();
        _state = State::ACTIVE;
        emit sessionStarted();
    }
    else if (method == "adapter.readData" && _state == State::ACTIVE)
    {
        emit readDataResult(decodeReadDataResult(result));
    }
    else if (method == "adapter.getStatus" && _state == State::ACTIVE)
    {
        emit statusResult(result["active"].toBool());
    }
    else if (method == "adapter.stop" && _state == State::STOPPING_SESSION)
    {
        qCInfo(scopeComm) << "AdapterClient:" << _adapterId << "session stopped, adapter kept alive, awaiting config";
        _handshakeTimer.stop();
        _state = State::AWAITING_CONFIG;
        emit sessionStopped();
        emit adapterReady();
    }
    else if (method == "adapter.dataPointSchema" && _state == State::AWAITING_CONFIG)
    {
        if (!consumeAuxResponse(method, id))
        {
            return;
        }
        emit dataPointSchemaResult(result);
    }
    else if (method == "adapter.describeDataPoint" &&
             (_state == State::AWAITING_CONFIG || _state == State::ACTIVE || _state == State::ACTIVE_DEGRADED))
    {
        if (!consumeAuxResponse(method, id))
        {
            return;
        }
        emit describeDataPointResult(result);
    }
    else if (method == "adapter.validateDataPoint" &&
             (_state == State::AWAITING_CONFIG || _state == State::ACTIVE || _state == State::ACTIVE_DEGRADED))
    {
        if (!consumeAuxResponse(method, id))
        {
            return;
        }
        emit validateDataPointResult(result["valid"].toBool(), result["error"].toString());
    }
    else if (method == "adapter.buildExpression" &&
             (_state == State::AWAITING_CONFIG || _state == State::ACTIVE || _state == State::ACTIVE_DEGRADED))
    {
        if (!consumeAuxResponse(method, id))
        {
            return;
        }
        emit buildExpressionResult(result["expression"].toString());
    }
    else if (method == "adapter.expressionHelp" &&
             (_state == State::AWAITING_CONFIG || _state == State::ACTIVE || _state == State::ACTIVE_DEGRADED))
    {
        if (!consumeAuxResponse(method, id))
        {
            return;
        }
        emit expressionHelpResult(result["helpText"].toString());
    }
    else
    {
        qCWarning(scopeComm) << "AdapterClient:" << _adapterId << "unexpected response for" << method << "in state"
                             << static_cast<int>(_state);
    }
}
