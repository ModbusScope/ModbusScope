
#include "communication/adapterpoll.h"

#include "models/settingsmodel.h"
#include "util/formatdatetime.h"
#include "util/scopelogging.h"

AdapterPoll::AdapterPoll(SettingsModel* pSettingsModel, QObject* parent) : QObject(parent), _bPollActive(false)
{
    _pPollTimer = new QTimer(this);
    _pPollTimer->setSingleShot(true);
    connect(_pPollTimer, &QTimer::timeout, this, &AdapterPoll::triggerRegisterRead);

    _pSettingsModel = pSettingsModel;
    _lastPollStart = QDateTime::currentMSecsSinceEpoch();

    _pAdapterManager = new AdapterManager(_pSettingsModel, this);

    connect(_pAdapterManager, &AdapterManager::sessionStarted, this, &AdapterPoll::triggerRegisterRead);
    connect(_pAdapterManager, &AdapterManager::readDataResult, this, &AdapterPoll::onReadDataResult);
    connect(_pAdapterManager, &AdapterManager::sessionStopped, this, &AdapterPoll::initAdapter);
    connect(_pAdapterManager, &AdapterManager::sessionError, this, [this](QString message) {
        qCWarning(scopeComm) << "AdapterManager error:" << message;
        _bPollActive = false;
        disconnect(_pAdapterManager, &AdapterManager::sessionStopped, this, &AdapterPoll::initAdapter);
    });
}

AdapterPoll::~AdapterPoll() = default;

/*! \brief Prepare the protocol adapter subprocess for use.
 *
 * Resolves the adapter binary relative to the running executable so the path
 * is correct in the build tree, AppImage, and installed layouts alike.
 * Delegates to AdapterManager::initAdapter().
 */
void AdapterPoll::initAdapter()
{
    _pAdapterManager->initAdapter();
}

void AdapterPoll::startCommunication(QList<DataPoint>& registerList)
{
    _registerList = registerList;
    _bPollActive = true;

    /* Re-establish auto-restart in case it was disconnected by a prior session error */
    disconnect(_pAdapterManager, &AdapterManager::sessionStopped, this, &AdapterPoll::initAdapter);
    connect(_pAdapterManager, &AdapterManager::sessionStopped, this, &AdapterPoll::initAdapter);

    qCInfo(scopeComm) << "Active registers: " << DataPoint::dumpListToString(_registerList);
    qCInfo(scopeComm) << qUtf8Printable(QString("Start logging: %1").arg(FormatDateTime::currentDateTime()));

    resetCommunicationStats();

    QStringList expressions = buildRegisterExpressions(_registerList);
    _pAdapterManager->startSession(expressions);
}

void AdapterPoll::resetCommunicationStats()
{
    _lastPollStart = QDateTime::currentMSecsSinceEpoch();
}

void AdapterPoll::stopCommunication()
{
    _bPollActive = false;
    _pPollTimer->stop();
    _pAdapterManager->stopSession();

    qCInfo(scopeComm) << qUtf8Printable(QString("Stop logging: %1").arg(FormatDateTime::currentDateTime()));
}

bool AdapterPoll::isActive()
{
    return _bPollActive;
}

void AdapterPoll::triggerRegisterRead()
{
    if (_bPollActive)
    {
        _lastPollStart = QDateTime::currentMSecsSinceEpoch();
        _pAdapterManager->requestReadData();
    }
}

void AdapterPoll::onReadDataResult(ResultDoubleList results)
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

/*! \brief Returns the AdapterManager owned by this instance. */
AdapterManager* AdapterPoll::adapterManager() const
{
    return _pAdapterManager;
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
