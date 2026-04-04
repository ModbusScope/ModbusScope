#include "ProtocolAdapter/adapterprocess.h"

#include "util/scopelogging.h"

#include <QJsonDocument>
#include <QTimer>

static constexpr int cStartTimeoutMs = 3000;
static constexpr int cStopTimeoutMs = 3000;

AdapterProcess::AdapterProcess(QObject* parent) : QObject(parent)
{
    _pProcess = new QProcess(this);
    _pFramingReader = new FramingReader(this);

    _pKillTimer = new QTimer(this);
    _pKillTimer->setSingleShot(true);
    connect(_pKillTimer, &QTimer::timeout, this, [this]() {
        if (_pProcess->state() != QProcess::NotRunning)
        {
            _pProcess->kill();
        }
    });

    connect(_pProcess, &QProcess::readyReadStandardOutput, this, &AdapterProcess::onReadyReadStdout);
    connect(_pProcess, &QProcess::readyReadStandardError, this, &AdapterProcess::onReadyReadStderr);
    connect(_pProcess, &QProcess::finished, this, &AdapterProcess::onProcessFinished);
    connect(_pFramingReader, &FramingReader::messageReceived, this, &AdapterProcess::onMessageReceived);
}

bool AdapterProcess::start(const QString& path)
{
    if (_pProcess->state() != QProcess::NotRunning)
    {
        qCWarning(scopeComm) << "AdapterProcess: process already running";
        return true;
    }

    _pKillTimer->stop();
    _pendingMethods.clear();
    _nextRequestId = 1;

    disconnect(_pProcess, &QProcess::errorOccurred, this, &AdapterProcess::onProcessError);
    _pProcess->start(path, QStringList());

    if (!_pProcess->waitForStarted(cStartTimeoutMs))
    {
        connect(_pProcess, &QProcess::errorOccurred, this, &AdapterProcess::onProcessError);
        qCWarning(scopeComm) << "AdapterProcess: failed to start process:" << path;
        emit processError(QString("Failed to start adapter process: %1").arg(path));
        return false;
    }

    connect(_pProcess, &QProcess::errorOccurred, this, &AdapterProcess::onProcessError);
    qCInfo(scopeComm) << "AdapterProcess: process started:" << path;
    return true;
}

void AdapterProcess::stop()
{
    if (_pProcess->state() != QProcess::NotRunning)
    {
        _pProcess->closeWriteChannel();
        _pKillTimer->stop();
        _pKillTimer->start(cStopTimeoutMs);
    }
}

int AdapterProcess::sendRequest(const QString& method, const QJsonObject& params)
{
    if (!isRunning())
    {
        qCWarning(scopeComm) << "AdapterProcess: sendRequest called while process is not running";
        emit processError(QString("Cannot send request '%1': adapter process is not running").arg(method));
        return -1;
    }

    int id = _nextRequestId++;
    if (_nextRequestId < 0)
    {
        _nextRequestId = 1;
    }

    QJsonObject request;
    request["jsonrpc"] = "2.0";
    request["id"] = id;
    request["method"] = method;
    request["params"] = params;

    QByteArray json = QJsonDocument(request).toJson(QJsonDocument::Compact);
    if (!writeFramed(json))
    {
        qCWarning(scopeComm) << "AdapterProcess: failed to write request:" << method;
        return -1;
    }

    _pendingMethods.insert(id, method);
    return id;
}

bool AdapterProcess::isRunning() const
{
    return _pProcess->state() != QProcess::NotRunning;
}

bool AdapterProcess::writeFramed(const QByteArray& json)
{
    QByteArray frame = "Content-Length: " + QByteArray::number(json.size()) + "\r\n\r\n" + json;
    qint64 written = _pProcess->write(frame);
    if (written != frame.size())
    {
        emit processError(QString("Failed to write to adapter process (wrote %1 of %2 bytes, error: %3)")
                            .arg(written)
                            .arg(frame.size())
                            .arg(_pProcess->errorString()));
        return false;
    }
    return true;
}

void AdapterProcess::onReadyReadStdout()
{
    QByteArray data = _pProcess->readAllStandardOutput();
    _pFramingReader->feed(data);
}

void AdapterProcess::onReadyReadStderr()
{
    QByteArray data = _pProcess->readAllStandardError();
    qCDebug(scopeComm) << "Adapter stderr:" << QString::fromUtf8(data).trimmed();
}

void AdapterProcess::onMessageReceived(QByteArray body)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        qCWarning(scopeComm) << "AdapterProcess: JSON parse error:" << parseError.errorString();
        return;
    }

    QJsonObject msg = doc.object();

    if (msg.contains("method"))
    {
        qCDebug(scopeComm) << "AdapterProcess: notification:" << msg["method"].toString();
        emit notificationReceived(msg["method"].toString(), msg.value("params"));
        return;
    }

    int id = msg["id"].toInt(-1);
    if (id < 0 || !_pendingMethods.contains(id))
    {
        qCWarning(scopeComm) << "AdapterProcess: unexpected response id:" << id;
        return;
    }

    QString method = _pendingMethods.take(id);

    if (msg.contains("error"))
    {
        emit errorReceived(id, method, msg["error"].toObject());
    }
    else
    {
        emit responseReceived(id, method, msg["result"]);
    }
}

void AdapterProcess::onProcessError(QProcess::ProcessError error)
{
    qCWarning(scopeComm) << "AdapterProcess: process error:" << error;
    emit processError(QString("Adapter process error: %1").arg(static_cast<int>(error)));
}

void AdapterProcess::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCInfo(scopeComm) << "AdapterProcess: process finished, exit code:" << exitCode << "status:" << exitStatus;
    _pendingMethods.clear();
    _pKillTimer->stop();
    emit processFinished();
}
