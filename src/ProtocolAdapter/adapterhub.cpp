
#include "ProtocolAdapter/adapterhub.h"

#include "ProtocolAdapter/adaptermanager.h"
#include "models/settingsmodel.h"

AdapterHub::AdapterHub(SettingsModel* pSettingsModel, QObject* parent)
    : QObject(parent), _pAdapterManager(new AdapterManager(pSettingsModel, this))
{
    connect(_pAdapterManager, &AdapterManager::sessionStarted, this, &AdapterHub::sessionStarted);
    connect(_pAdapterManager, &AdapterManager::sessionStopped, this, &AdapterHub::sessionStopped);
    connect(_pAdapterManager, &AdapterManager::adapterReady, this, &AdapterHub::adapterReady);
    connect(_pAdapterManager, &AdapterManager::sessionError, this, &AdapterHub::sessionError);
    connect(_pAdapterManager, &AdapterManager::readDataResult, this, &AdapterHub::readDataResult);
    connect(_pAdapterManager, &AdapterManager::buildExpressionResult, this, &AdapterHub::buildExpressionResult);
    connect(_pAdapterManager, &AdapterManager::expressionHelpResult, this, &AdapterHub::expressionHelpResult);
    connect(_pAdapterManager, &AdapterManager::describeDataPointResult, this, &AdapterHub::describeDataPointResult);
}

/*! \brief Protected constructor for mock subclasses used in unit tests.
 *
 * Does not create an AdapterManager. Subclasses that override all virtual methods
 * can use this to avoid instantiating real adapter infrastructure.
 */
AdapterHub::AdapterHub(QObject* parent) : QObject(parent), _pAdapterManager(nullptr)
{
}

/*! \brief Launch the adapter subprocess and begin the initialization handshake. */
void AdapterHub::initAdapter()
{
    _pAdapterManager->initAdapter();
}

/*! \brief Provide register expressions to the adapter and start the session.
 * \param expressions Register expression strings to pass to the adapter.
 */
void AdapterHub::startSession(const QStringList& expressions)
{
    _pAdapterManager->startSession(expressions);
}

/*! \brief Send adapter.stop to pause polling and keep the adapter process alive. */
void AdapterHub::stopSession()
{
    _pAdapterManager->stopSession();
}

/*! \brief Send an adapter.readData request to the active adapter. */
void AdapterHub::requestReadData()
{
    _pAdapterManager->requestReadData();
}

/*! \brief Return the AdapterManager for the given adapter ID.
 *
 * \param id Adapter identifier string (e.g. "modbus").
 * \return Pointer to the matching AdapterManager, or nullptr if not found.
 */
AdapterManager* AdapterHub::adapterManager(const QString& id) const
{
    if (_pAdapterManager != nullptr && _pAdapterManager->adapterId() == id)
    {
        return _pAdapterManager;
    }
    return nullptr;
}

/*! \brief Returns true when the adapter is in AWAITING_CONFIG and ready for provideConfig(). */
bool AdapterHub::isAdapterReady() const
{
    return _pAdapterManager->isAdapterReady();
}

/*! \brief Returns true when the adapter process is not running (IDLE state). */
bool AdapterHub::isAdapterIdle() const
{
    return _pAdapterManager->isAdapterIdle();
}
