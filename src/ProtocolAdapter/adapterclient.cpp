#include "ProtocolAdapter/adapterclient.h"

#include "util/scopelogging.h"

#include <QJsonArray>

AdapterClient::AdapterClient(AdapterProcess* pProcess, QObject* parent) : QObject(parent), _pProcess(pProcess)
{
    Q_ASSERT(pProcess);
    _pProcess->setParent(this);

    _handshakeTimer.setSingleShot(true);
    connect(&_handshakeTimer, &QTimer::timeout, this, &AdapterClient::onHandshakeTimeout);

    connect(_pProcess, &AdapterProcess::responseReceived, this, &AdapterClient::onResponseReceived);
    connect(_pProcess, &AdapterProcess::errorReceived, this, &AdapterClient::onErrorReceived);
    connect(_pProcess, &AdapterProcess::processError, this, &AdapterClient::onProcessError);
    connect(_pProcess, &AdapterProcess::processFinished, this, &AdapterClient::onProcessFinished);
}

AdapterClient::~AdapterClient() = default;

void AdapterClient::startSession(const QString& adapterPath, QJsonObject config, QStringList registerExpressions)
{
    if (_state != State::IDLE)
    {
        qCWarning(scopeComm) << "AdapterClient: startSession called in non-idle state";
        return;
    }

    _pendingConfig = config;
    _pendingExpressions = registerExpressions;

    if (!_pProcess->start(adapterPath))
    {
        return;
    }

    qCInfo(scopeComm) << "AdapterClient: process started, sending initialize";
    _state = State::INITIALIZING;
    _handshakeTimer.start(cHandshakeTimeoutMs);
    _pProcess->sendRequest("adapter.initialize", QJsonObject());
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
    }
    else
    {
        _pProcess->stop();
        _state = State::IDLE;
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
        _state = State::IDLE;
        QTimer::singleShot(0, this, [this]() { _pProcess->stop(); });
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
    _state = State::IDLE;
    QTimer::singleShot(0, this, [this]() { _pProcess->stop(); });

    if (previousState != State::STOPPING)
    {
        emit sessionError(QString("Adapter error on %1: %2").arg(method, errorMsg));
    }
}

void AdapterClient::onProcessError(const QString& message)
{
    _handshakeTimer.stop();
    _state = State::IDLE;
    emit sessionError(message);
}

void AdapterClient::onProcessFinished()
{
    _handshakeTimer.stop();
    if (_state != State::IDLE)
    {
        _state = State::IDLE;
        emit sessionError("Adapter process exited unexpectedly");
    }
}

void AdapterClient::onHandshakeTimeout()
{
    qCWarning(scopeComm) << "AdapterClient: handshake timed out in state" << static_cast<int>(_state);
    _state = State::IDLE;
    QTimer::singleShot(0, this, [this]() { _pProcess->stop(); });
    emit sessionError("Adapter handshake timed out");
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
        qCInfo(scopeComm) << "AdapterClient: described, sending configure";
        emit describeResult(result);
        _state = State::CONFIGURING;
        QJsonObject params;
        params["config"] = _pendingConfig;
        _pProcess->sendRequest("adapter.configure", params);
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
        _state = State::IDLE;
    }
    else
    {
        qCWarning(scopeComm) << "AdapterClient: unexpected response for" << method << "in state"
                             << static_cast<int>(_state);
    }
}
