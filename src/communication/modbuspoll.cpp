
#include "communication/modbuspoll.h"

#include "models/settingsmodel.h"
#include "util/formatdatetime.h"
#include "util/scopelogging.h"


ModbusPoll::ModbusPoll(SettingsModel* pSettingsModel, QObject* parent) : QObject(parent), _bPollActive(false)
{
    _pPollTimer = new QTimer(this);
    _pPollTimer->setSingleShot(true);
    connect(_pPollTimer, &QTimer::timeout, this, &ModbusPoll::triggerRegisterRead);

    _pSettingsModel = pSettingsModel;
    _lastPollStart = QDateTime::currentMSecsSinceEpoch();

    _pAdapterManager = new AdapterManager(_pSettingsModel, this);

    connect(_pAdapterManager, &AdapterManager::sessionStarted, this, &ModbusPoll::triggerRegisterRead);
    connect(_pAdapterManager, &AdapterManager::readDataResult, this, &ModbusPoll::onReadDataResult);
    connect(_pAdapterManager, &AdapterManager::sessionStopped, this, &ModbusPoll::initAdapter);
    connect(_pAdapterManager, &AdapterManager::sessionError, this, [this](QString message) {
        qCWarning(scopeComm) << "AdapterManager error:" << message;
        _bPollActive = false;
        disconnect(_pAdapterManager, &AdapterManager::sessionStopped, this, &ModbusPoll::initAdapter);
    });
}

ModbusPoll::~ModbusPoll() = default;

/*! \brief Prepare the protocol adapter subprocess for use.
 *
 * Resolves the adapter binary relative to the running executable so the path
 * is correct in the build tree, AppImage, and installed layouts alike.
 * Delegates to AdapterManager::initAdapter().
 */
void ModbusPoll::initAdapter()
{
    _pAdapterManager->initAdapter();
}

void ModbusPoll::startCommunication(QList<DataPoint>& registerList)
{
    _registerList = registerList;
    _bPollActive = true;

    /* Re-establish auto-restart in case it was disconnected by a prior session error */
    disconnect(_pAdapterManager, &AdapterManager::sessionStopped, this, &ModbusPoll::initAdapter);
    connect(_pAdapterManager, &AdapterManager::sessionStopped, this, &ModbusPoll::initAdapter);

    qCInfo(scopeComm) << QString("Start logging: %1").arg(FormatDateTime::currentDateTime());

    resetCommunicationStats();

    QStringList expressions = buildRegisterExpressions(_registerList);
    _pAdapterManager->startSession(expressions);
}

void ModbusPoll::resetCommunicationStats()
{
    _lastPollStart = QDateTime::currentMSecsSinceEpoch();
}

void ModbusPoll::stopCommunication()
{
    _bPollActive = false;
    _pPollTimer->stop();
    _pAdapterManager->stopSession();

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
        _pAdapterManager->requestReadData();
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


/*! \brief Returns the AdapterManager owned by this instance. */
AdapterManager* ModbusPoll::adapterManager() const
{
    return _pAdapterManager;
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
