
#include "ProtocolAdapter/adapterhub.h"

#include "ProtocolAdapter/adapterdiscovery.h"
#include "ProtocolAdapter/adaptermanager.h"
#include "models/settingsmodel.h"
#include "util/scopelogging.h"

#include <QCoreApplication>

#include <utility>

AdapterHub::AdapterHub(SettingsModel* pSettingsModel, QObject* parent)
    : QObject(parent), _pSettingsModel(pSettingsModel)
{
}

/*! \brief Protected constructor for mock subclasses used in unit tests.
 *
 * Does not create any AdapterManager. Subclasses that override all virtual methods
 * can use this to avoid instantiating real adapter infrastructure.
 */
AdapterHub::AdapterHub(QObject* parent) : QObject(parent), _pSettingsModel(nullptr)
{
}

/*! \brief Ensure every discovered adapter has an initialization handshake in flight or complete.
 *
 * On the first call, uses AdapterDiscovery to locate adapter binaries in the application
 * directory and creates one AdapterManager per binary. On every call, including the first,
 * (re)starts the handshake only for managers currently in IDLE state; a manager that is already
 * ready, mid-handshake, active, or degraded is left untouched.
 *
 * This makes the call idempotent and safe to repeat at any time - in particular, when one
 * adapter has crashed back to IDLE while its siblings are unaffected, only the crashed one is
 * restarted. The hub's adapterReady() signal still fires once every manager - old and newly
 * restarted alike - is back in AWAITING_CONFIG.
 */
void AdapterHub::initAdapter()
{
    if (_adapterManagers.isEmpty())
    {
        _pendingReadyAdapters.clear();
        _pendingStartAdapters.clear();

        const QList<AdapterInfo> discovered = AdapterDiscovery::discover(QCoreApplication::applicationDirPath());

        if (discovered.isEmpty())
        {
            qCWarning(scopeComm) << "AdapterHub: no adapter binaries found in"
                                 << QCoreApplication::applicationDirPath();
            emit sessionError(QStringLiteral("No adapter binaries found"));
            return;
        }

        for (const AdapterInfo& info : discovered)
        {
            if (_adapterManagers.contains(info.id))
            {
                qCWarning(scopeComm) << "AdapterHub: duplicate adapter id" << info.id << "for" << info.binaryPath
                                     << "- skipping";
                continue;
            }
            auto* mgr = new AdapterManager(info.id, info.binaryPath, _pSettingsModel, this);
            _adapterManagers.insert(info.id, mgr);
            connectManager(mgr, info.id);
        }
    }

    for (auto it = _adapterManagers.constBegin(); it != _adapterManagers.constEnd(); ++it)
    {
        if (it.value()->isAdapterIdle())
        {
            _pendingReadyAdapters.insert(it.key());
            it.value()->initAdapter();
        }
    }
}

/*! \brief Provide register expressions to the named adapter and start its session.
 * \param adapterId   Adapter identifier (e.g. "modbus").
 * \param expressions Register expression strings to pass to the adapter.
 */
void AdapterHub::startSession(const QString& adapterId, const QStringList& expressions)
{
    AdapterManager* mgr = _adapterManagers.value(adapterId, nullptr);
    if (mgr != nullptr)
    {
        _pendingStartAdapters.insert(adapterId);
        mgr->startSession(expressions);
    }
    else
    {
        qCWarning(scopeComm) << "AdapterHub::startSession: unknown adapter" << adapterId;
    }
}

/*! \brief Send adapter.stop to every adapter manager with a session in progress or established.
 *
 * A manager that never started a session (AWAITING_CONFIG) or has no process running (IDLE) is
 * left untouched. A manager mid-handshake is force-stopped but not waited on, and is purged from
 * _pendingStartAdapters directly since its force-stop never emits sessionStarted()/sessionError().
 *
 * Bookkeeping for every manager being stopped is updated before any manager's stopSession() is
 * actually called, since a degraded manager's stopSession() can emit adapterReady() synchronously
 * and must not be allowed to fire AdapterHub::adapterReady() mid-sweep.
 */
void AdapterHub::stopSession()
{
    QList<AdapterManager*> managersToStop;
    for (auto it = _adapterManagers.constBegin(); it != _adapterManagers.constEnd(); ++it)
    {
        AdapterManager* mgr = it.value();
        if (mgr->isAdapterIdle() || mgr->isAdapterReady())
        {
            continue;
        }

        if (mgr->isAdapterActive())
        {
            _pendingReadyAdapters.insert(it.key());
        }
        else
        {
            _pendingStartAdapters.remove(it.key());
        }
        managersToStop.append(mgr);
    }

    for (AdapterManager* mgr : std::as_const(managersToStop))
    {
        mgr->stopSession();
    }
}

/*! \brief Send adapter.readData to all adapter managers with an established session. */
void AdapterHub::requestReadData()
{
    for (auto it = _adapterManagers.constBegin(); it != _adapterManagers.constEnd(); ++it)
    {
        if (it.value()->isAdapterActive())
        {
            it.value()->requestReadData();
        }
    }
}

/*! \brief Return the AdapterManager for the given adapter ID.
 * \param id Adapter identifier string (e.g. "modbus").
 * \return Pointer to the matching AdapterManager, or nullptr if not found.
 */
AdapterManager* AdapterHub::adapterManager(const QString& id) const
{
    return _adapterManagers.value(id, nullptr);
}

/*! \brief Return the list of discovered adapter IDs.
 * \return QStringList of adapter ID strings, sorted alphabetically.
 */
QStringList AdapterHub::adapterIds() const
{
    return _adapterManagers.keys();
}

/*! \brief Returns true when all adapter managers are in AWAITING_CONFIG state. */
bool AdapterHub::isAdapterReady() const
{
    if (_adapterManagers.isEmpty())
    {
        return false;
    }
    for (const AdapterManager* mgr : _adapterManagers)
    {
        if (!mgr->isAdapterReady())
        {
            return false;
        }
    }
    return true;
}

void AdapterHub::onManagerAdapterReady(const QString& id)
{
    _pendingReadyAdapters.remove(id);
    if (_pendingReadyAdapters.isEmpty())
    {
        emit adapterReady();
    }
}

void AdapterHub::onManagerSessionStarted(const QString& id)
{
    _pendingStartAdapters.remove(id);
    if (_pendingStartAdapters.isEmpty())
    {
        emit sessionStarted();
    }
}

void AdapterHub::onManagerSessionError(const QString& id, const QString& message)
{
    _pendingStartAdapters.remove(id);
    emit sessionError(message);
}

void AdapterHub::connectManager(AdapterManager* mgr, const QString& id)
{
    connect(mgr, &AdapterManager::adapterReady, this, [this, id]() { onManagerAdapterReady(id); });
    connect(mgr, &AdapterManager::sessionStarted, this, [this, id]() { onManagerSessionStarted(id); });
    connect(mgr, &AdapterManager::sessionStopped, this, &AdapterHub::sessionStopped);
    connect(mgr, &AdapterManager::sessionError, this,
            [this, id](QString message) { onManagerSessionError(id, message); });
    connect(mgr, &AdapterManager::readDataResult, this, [this, id](ResultDoubleList r) { emit readDataResult(id, r); });
    connect(mgr, &AdapterManager::buildExpressionResult, this, &AdapterHub::buildExpressionResult);
    connect(mgr, &AdapterManager::expressionHelpResult, this, &AdapterHub::expressionHelpResult);
    connect(mgr, &AdapterManager::describeDataPointResult, this, &AdapterHub::describeDataPointResult);
}
