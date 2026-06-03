
#include "communication/adapterpoll.h"

#include "models/settingsmodel.h"
#include "util/formatdatetime.h"
#include "util/scopelogging.h"

AdapterPoll::AdapterPoll(SettingsModel* pSettingsModel, QObject* parent) : QObject(parent)
{
    _pPollTimer = new QTimer(this);
    _pPollTimer->setSingleShot(true);
    connect(_pPollTimer, &QTimer::timeout, this, &AdapterPoll::triggerRegisterRead);

    _pSettingsModel = pSettingsModel;
    _lastPollStart = QDateTime::currentMSecsSinceEpoch();

    _pAdapterHub = new AdapterHub(_pSettingsModel, this);

    connect(_pAdapterHub, &AdapterHub::sessionStarted, this, &AdapterPoll::triggerRegisterRead);
    connect(_pAdapterHub, &AdapterHub::readDataResult, this, &AdapterPoll::onReadDataResult);
    connect(_pAdapterHub, &AdapterHub::sessionError, this, &AdapterPoll::onSessionError);
}

AdapterPoll::AdapterPoll(SettingsModel* pSettingsModel, AdapterHub* pAdapterHub, QObject* parent) : QObject(parent)
{
    _pPollTimer = new QTimer(this);
    _pPollTimer->setSingleShot(true);
    connect(_pPollTimer, &QTimer::timeout, this, &AdapterPoll::triggerRegisterRead);

    _pSettingsModel = pSettingsModel;
    _lastPollStart = QDateTime::currentMSecsSinceEpoch();

    _pAdapterHub = pAdapterHub;

    connect(_pAdapterHub, &AdapterHub::sessionStarted, this, &AdapterPoll::triggerRegisterRead);
    connect(_pAdapterHub, &AdapterHub::readDataResult, this, &AdapterPoll::onReadDataResult);
    connect(_pAdapterHub, &AdapterHub::sessionError, this, &AdapterPoll::onSessionError);
}

AdapterPoll::~AdapterPoll() = default;

/*! \brief Prepare the protocol adapter subprocess for use.
 *
 * Delegates to AdapterHub::initAdapter().
 */
void AdapterPoll::initAdapter()
{
    _pAdapterHub->initAdapter();
}

void AdapterPoll::startCommunication(QList<DataPoint>& registerList)
{
    _registerList = registerList;

    qCInfo(scopeComm) << "Active registers: " << DataPoint::dumpListToString(_registerList);
    qCInfo(scopeComm) << qUtf8Printable(QString("Start logging: %1").arg(FormatDateTime::currentDateTime()));

    resetCommunicationStats();

    QStringList expressions = buildRegisterExpressions(_registerList);

    if (_pAdapterHub->isAdapterReady())
    {
        _pollState = PollState::Active;
        _pAdapterHub->startSession(expressions);
    }
    else
    {
        _pendingExpressions = expressions;
        if (_pollState != PollState::WaitingForAdapter)
        {
            _pollState = PollState::WaitingForAdapter;
            _adapterReadyConnection = connect(_pAdapterHub, &AdapterHub::adapterReady, this,
                                              &AdapterPoll::onAdapterReady, Qt::SingleShotConnection);
        }
        if (_pAdapterHub->isAdapterIdle())
        {
            _pAdapterHub->initAdapter();
        }
        /* else: adapter is already initializing; onAdapterReady fires when it reaches AWAITING_CONFIG */
    }
}

void AdapterPoll::resetCommunicationStats()
{
    _lastPollStart = QDateTime::currentMSecsSinceEpoch();
}

void AdapterPoll::stopCommunication()
{
    if (_pollState == PollState::WaitingForAdapter)
    {
        disconnect(_adapterReadyConnection);
        _adapterReadyConnection = {};
    }
    _pollState = PollState::Inactive;
    _pendingExpressions.clear();
    _pPollTimer->stop();
    _pAdapterHub->stopSession();

    qCInfo(scopeComm) << qUtf8Printable(QString("Stop logging: %1").arg(FormatDateTime::currentDateTime()));
}

bool AdapterPoll::isActive() const
{
    return _pollState != PollState::Inactive;
}

void AdapterPoll::triggerRegisterRead()
{
    if (_pollState == PollState::Active)
    {
        _lastPollStart = QDateTime::currentMSecsSinceEpoch();
        _pAdapterHub->requestReadData();
    }
}

void AdapterPoll::onReadDataResult(ResultDoubleList results)
{
    if (_pollState != PollState::Active)
    {
        return;
    }

    emit registerDataReady(results);

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

/*! \brief Returns the AdapterHub owned by this instance. */
AdapterHub* AdapterPoll::adapterHub() const
{
    return _pAdapterHub;
}

void AdapterPoll::onAdapterReady()
{
    if (_pollState == PollState::WaitingForAdapter)
    {
        _pollState = PollState::Active;
        _pAdapterHub->startSession(_pendingExpressions);
        _pendingExpressions.clear();
    }
}

void AdapterPoll::onSessionError(const QString& message)
{
    qCWarning(scopeComm) << "AdapterHub error:" << message;
    if (_pollState == PollState::WaitingForAdapter)
    {
        disconnect(_adapterReadyConnection);
        _adapterReadyConnection = {};
    }
    _pollState = PollState::Inactive;
}

QStringList AdapterPoll::buildRegisterExpressions(const QList<DataPoint>& registerList)
{
    QStringList expressions;
    for (const DataPoint& reg : registerList)
    {
        expressions.append(reg.address());
    }
    return expressions;
}
