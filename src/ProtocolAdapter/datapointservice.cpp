#include "ProtocolAdapter/datapointservice.h"

#include "ProtocolAdapter/adapterhub.h"
#include "ProtocolAdapter/adaptermanager.h"
#include "models/adapterdata.h"
#include "models/settingsmodel.h"
#include "util/scopelogging.h"

#include <QJsonArray>
#include <QTimer>

/*!
 * \brief Construct a service that routes data point requests through \a pAdapterHub.
 *
 * adaptersChanged() is emitted when the hub reports adapters ready or a session started,
 * because an adapter may have become usable.
 * \param pSettingsModel   Settings model with the devices and adapter schemas.
 * \param pAdapterHub      Hub that owns the adapter managers.
 * \param parent           Optional QObject parent.
 * \param requestTimeoutMs Time after which a request without reply fails.
 */
DataPointService::DataPointService(SettingsModel* pSettingsModel,
                                   AdapterHub* pAdapterHub,
                                   QObject* parent,
                                   int requestTimeoutMs)
    : QObject(parent), _pSettingsModel(pSettingsModel), _pAdapterHub(pAdapterHub), _requestTimeoutMs(requestTimeoutMs)
{
    connect(_pAdapterHub, &AdapterHub::adapterReady, this, &DataPointService::adaptersChanged);
    connect(_pAdapterHub, &AdapterHub::sessionStarted, this, &DataPointService::adaptersChanged);
}

/*!
 * \brief Returns whether data point requests for \a deviceId can be sent now.
 *
 * True when the device exists and its adapter is running and ready for requests.
 */
bool DataPointService::isDeviceUsable(deviceId_t deviceId) const
{
    return usableManager(deviceId) != nullptr;
}

/*!
 * \brief Returns the id of the adapter that serves \a deviceId, or an empty string for an unknown device.
 */
QString DataPointService::adapterIdForDevice(deviceId_t deviceId) const
{
    if (!_pSettingsModel->hasDevice(deviceId))
    {
        return QString();
    }

    return _pSettingsModel->adapterIdForDevice(deviceId);
}

/*!
 * \brief Returns the address schema of the adapter that serves \a deviceId.
 *
 * The device is already fixed by the caller, so the adapter's own \c deviceId property is removed:
 * an address form built from this schema must not show a second device selector.
 * \return The schema for SchemaFormWidget, or an empty object when the adapter has not published one.
 */
QJsonObject DataPointService::addressSchema(deviceId_t deviceId) const
{
    QJsonObject schema = dataPointSchema(deviceId).value("addressSchema").toObject();

    QJsonObject properties = schema.value("properties").toObject();
    if (!properties.contains(QStringLiteral("deviceId")))
    {
        return schema;
    }

    properties.remove(QStringLiteral("deviceId"));
    schema["properties"] = properties;

    QJsonArray required = schema.value("required").toArray();
    for (qsizetype idx = 0; idx < required.size(); ++idx)
    {
        if (required.at(idx).toString() == QStringLiteral("deviceId"))
        {
            required.removeAt(idx);
            schema["required"] = required;
            break;
        }
    }

    return schema;
}

/*!
 * \brief Returns the default address field values of the adapter that serves \a deviceId.
 */
QJsonObject DataPointService::defaults(deviceId_t deviceId) const
{
    return dataPointSchema(deviceId).value("defaults").toObject();
}

/*!
 * \brief Ask the adapter of \a deviceId to build a data point reference from address fields.
 *
 * \a fields can be the values of an address form or the \c fields of a describeDataPoint result:
 * \c dataType is sent as the data type, a \c deviceId key is ignored in favour of \a deviceId.
 * \param deviceId Device the data point belongs to.
 * \param fields   Address field values.
 * \return Ticket that identifies the request in expressionBuilt() or requestFailed().
 */
quint64 DataPointService::buildExpression(deviceId_t deviceId, const QJsonObject& fields)
{
    AdapterManager* pManager = usableManager(deviceId);
    if (pManager == nullptr)
    {
        const quint64 ticket = ++_lastTicket;
        failLater(ticket, unusableReason(deviceId));
        return ticket;
    }

    QJsonObject addressFields = fields;
    const QString dataType = addressFields.take(QStringLiteral("dataType")).toString();
    addressFields.remove(QStringLiteral("deviceId"));

    connectManager(pManager);
    return track(pManager, pManager->buildExpression(addressFields, dataType, deviceId));
}

/*!
 * \brief Ask the adapter of \a deviceId to describe one data point reference.
 * \param deviceId  Device the reference belongs to.
 * \param reference One data point reference, e.g. \c ${40001}.
 * \return Ticket that identifies the request in dataPointDescribed() or requestFailed().
 */
quint64 DataPointService::describeDataPoint(deviceId_t deviceId, const QString& reference)
{
    AdapterManager* pManager = usableManager(deviceId);
    if (pManager == nullptr)
    {
        const quint64 ticket = ++_lastTicket;
        failLater(ticket, unusableReason(deviceId));
        return ticket;
    }

    connectManager(pManager);
    return track(pManager, pManager->describeDataPoint(reference));
}

void DataPointService::emitQueuedFailures()
{
    const QList<QueuedFailure> failures = _queuedFailures;
    _queuedFailures.clear();

    for (const QueuedFailure& failure : failures)
    {
        emit requestFailed(failure.ticket, failure.error);
    }
}

/*!
 * \brief Fail the pending requests of a manager that is being destroyed.
 * \param pObject The destroyed manager.
 */
void DataPointService::onManagerDestroyed(QObject* pObject)
{
    _connectedManagers.remove(pObject);

    for (auto it = _pendingRequests.begin(); it != _pendingRequests.end();)
    {
        if (static_cast<QObject*>(it->pManager) == pObject)
        {
            failLater(it.key(), QStringLiteral("The adapter stopped"));
            it = _pendingRequests.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

AdapterManager* DataPointService::usableManager(deviceId_t deviceId) const
{
    const QString adapterId = adapterIdForDevice(deviceId);
    if (adapterId.isEmpty())
    {
        return nullptr;
    }

    AdapterManager* pManager = _pAdapterHub->adapterManager(adapterId);
    if (pManager == nullptr || !(pManager->isAdapterReady() || pManager->isAdapterActive()))
    {
        return nullptr;
    }

    return pManager;
}

/*!
 * \brief Returns the data point schema of the adapter that serves \a deviceId, without adding unknown adapters.
 */
QJsonObject DataPointService::dataPointSchema(deviceId_t deviceId) const
{
    const QString adapterId = adapterIdForDevice(deviceId);
    if (adapterId.isEmpty() || !_pSettingsModel->adapterIds().contains(adapterId))
    {
        return QJsonObject();
    }

    return _pSettingsModel->adapterData(adapterId)->dataPointSchema();
}

/*!
 * \brief Issue a ticket for a sent request and start its timeout.
 * \param pManager  Manager the request was sent to.
 * \param requestId Request id returned by the manager; -1 when the request was not sent.
 * \return The new ticket.
 */
quint64 DataPointService::track(AdapterManager* pManager, int requestId)
{
    const quint64 ticket = ++_lastTicket;
    if (requestId < 0)
    {
        failLater(ticket, QStringLiteral("The adapter did not accept the request"));
        return ticket;
    }

    _pendingRequests.insert(ticket, { pManager, requestId });
    QTimer::singleShot(_requestTimeoutMs, this, [this, ticket]() { onRequestTimeout(ticket); });
    return ticket;
}

/*!
 * \brief Find and forget the pending ticket for a reply.
 * \return The ticket, or 0 when the reply belongs to no pending ticket (e.g. it timed out).
 */
quint64 DataPointService::takeTicket(AdapterManager* pManager, int requestId)
{
    for (auto it = _pendingRequests.begin(); it != _pendingRequests.end(); ++it)
    {
        if (it->pManager == pManager && it->requestId == requestId)
        {
            const quint64 ticket = it.key();
            _pendingRequests.erase(it);
            return ticket;
        }
    }

    return 0;
}

void DataPointService::failLater(quint64 ticket, const QString& error)
{
    _queuedFailures.append({ ticket, error });
    if (_queuedFailures.size() == 1)
    {
        QMetaObject::invokeMethod(this, &DataPointService::emitQueuedFailures, Qt::QueuedConnection);
    }
}

void DataPointService::connectManager(AdapterManager* pManager)
{
    if (_connectedManagers.contains(pManager))
    {
        return;
    }
    _connectedManagers.insert(pManager);

    connect(pManager, &QObject::destroyed, this, &DataPointService::onManagerDestroyed);

    connect(pManager, &AdapterManager::buildExpressionResult, this,
            [this, pManager](int requestId, const QString& expression) {
                onExpressionBuilt(pManager, requestId, expression);
            });
    connect(pManager, &AdapterManager::describeDataPointResult, this,
            [this, pManager](int requestId, const QJsonObject& result) {
                onDataPointDescribed(pManager, requestId, result);
            });
    connect(pManager, &AdapterManager::dataPointRequestFailed, this,
            [this, pManager](int requestId, const QString& error) { onRequestFailed(pManager, requestId, error); });
}

void DataPointService::onExpressionBuilt(AdapterManager* pManager, int requestId, const QString& expression)
{
    const quint64 ticket = takeTicket(pManager, requestId);
    if (ticket == 0)
    {
        return;
    }

    emit expressionBuilt(ticket, expression);
}

void DataPointService::onDataPointDescribed(AdapterManager* pManager, int requestId, const QJsonObject& result)
{
    const quint64 ticket = takeTicket(pManager, requestId);
    if (ticket == 0)
    {
        return;
    }

    emit dataPointDescribed(ticket, result);
}

void DataPointService::onRequestFailed(AdapterManager* pManager, int requestId, const QString& error)
{
    const quint64 ticket = takeTicket(pManager, requestId);
    if (ticket == 0)
    {
        return;
    }

    failLater(ticket, error);
}

void DataPointService::onRequestTimeout(quint64 ticket)
{
    if (!_pendingRequests.remove(ticket))
    {
        return;
    }

    qCWarning(scopeComm) << "DataPointService: request" << ticket << "timed out";
    failLater(ticket, QStringLiteral("The adapter did not respond"));
}

QString DataPointService::unusableReason(deviceId_t deviceId) const
{
    if (!_pSettingsModel->hasDevice(deviceId))
    {
        return QStringLiteral("Device %1 does not exist").arg(deviceId);
    }

    return QStringLiteral("The adapter is not running");
}
