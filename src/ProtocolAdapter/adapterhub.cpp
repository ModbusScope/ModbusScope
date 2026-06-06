
#include "ProtocolAdapter/adapterhub.h"

#include "ProtocolAdapter/adapterdiscovery.h"
#include "ProtocolAdapter/adaptermanager.h"
#include "models/settingsmodel.h"
#include "util/scopelogging.h"

#include <QCoreApplication>

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

/*! \brief Discover adapter binaries and start the initialization handshake for each.
 *
 * Uses AdapterDiscovery to locate binaries in the application directory, creates one
 * AdapterManager per binary, and calls initAdapter() on each. The hub's adapterReady()
 * signal is emitted only after all managers have reached AWAITING_CONFIG state.
 */
void AdapterHub::initAdapter()
{
    qDeleteAll(_adapterManagers);
    _adapterManagers.clear();
    _pendingReadyAdapters.clear();
    _pendingStartAdapters.clear();

    const QList<AdapterInfo> discovered = AdapterDiscovery::discover(QCoreApplication::applicationDirPath());

    if (discovered.isEmpty())
    {
        qCWarning(scopeComm) << "AdapterHub: no adapter binaries found in" << QCoreApplication::applicationDirPath();
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

    for (auto it = _adapterManagers.constBegin(); it != _adapterManagers.constEnd(); ++it)
    {
        _pendingReadyAdapters.insert(it.key());
        it.value()->initAdapter();
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

/*! \brief Send adapter.stop to all active adapter managers. */
void AdapterHub::stopSession()
{
    for (auto it = _adapterManagers.constBegin(); it != _adapterManagers.constEnd(); ++it)
    {
        _pendingReadyAdapters.insert(it.key());
        it.value()->stopSession();
    }
}

/*! \brief Send adapter.readData to all adapter managers. */
void AdapterHub::requestReadData()
{
    for (auto it = _adapterManagers.constBegin(); it != _adapterManagers.constEnd(); ++it)
    {
        it.value()->requestReadData();
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

/*! \brief Returns true when all adapter managers are in IDLE state (no subprocess running). */
bool AdapterHub::isAdapterIdle() const
{
    if (_adapterManagers.isEmpty())
    {
        return true;
    }
    for (const AdapterManager* mgr : _adapterManagers)
    {
        if (!mgr->isAdapterIdle())
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
    if (_pendingStartAdapters.isEmpty())
    {
        emit sessionStarted();
    }
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
