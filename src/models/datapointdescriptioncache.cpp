#include "datapointdescriptioncache.h"

#include "ProtocolAdapter/datapointservice.h"

using State = DataPointDescription::State;

/*!
 * \brief Construct a cache that asks \a pService for descriptions it does not have yet.
 * \param pService      Service that sends the describe requests.
 * \param parent        Optional QObject parent.
 * \param failedRetryMs Time after which a failed request may be sent again.
 */
DataPointDescriptionCache::DataPointDescriptionCache(DataPointService* pService, QObject* parent, int failedRetryMs)
    : QObject(parent), _pService(pService), _failedRetryMs(failedRetryMs)
{
    connect(_pService, &DataPointService::dataPointDescribed, this, &DataPointDescriptionCache::onDataPointDescribed);
    connect(_pService, &DataPointService::requestFailed, this, &DataPointDescriptionCache::onRequestFailed);
    connect(_pService, &DataPointService::adaptersChanged, this, &DataPointDescriptionCache::onAdaptersChanged);
}

/*!
 * \brief Returns what is known about a reference, and asks its adapter on a miss.
 *
 * Entries are keyed by reference text and are only used while the device is served by the adapter
 * that described it. While a request is pending, lookups do not send it again.
 * An \c Invalid entry is kept; an \c Unavailable entry is kept until an adapter may have become
 * usable; a \c Failed entry is kept until the retry interval has passed.
 * descriptionChanged() is emitted when a reply arrives.
 * \param reference The reference to describe.
 * \return The cached description, \c Pending when a request was just sent.
 */
DataPointDescription DataPointDescriptionCache::lookup(const ExpressionReferences::Reference& reference)
{
    const QString adapterId = _pService->adapterIdForDevice(reference.deviceId);
    const auto it = _entries.constFind(reference.text);
    if (it != _entries.constEnd() && it->adapterId == adapterId)
    {
        const bool bRetryDue = it->description.state == State::Failed && it->retryDeadline.hasExpired();
        if (!bRetryDue)
        {
            return it->description;
        }
    }

    Entry entry;
    entry.adapterId = adapterId;
    if (!_pService->isDeviceUsable(reference.deviceId))
    {
        entry.description.state = State::Unavailable;
        _entries.insert(reference.text, entry);
        return entry.description;
    }

    const quint64 ticket = _pService->describeDataPoint(reference.deviceId, reference.text);
    _pendingRequests.insert(ticket, PendingRequest{ reference.text, adapterId });
    _entries.insert(reference.text, entry);
    return entry.description;
}

void DataPointDescriptionCache::onDataPointDescribed(quint64 ticket, const QJsonObject& result)
{
    if (!_pendingRequests.contains(ticket))
    {
        return;
    }
    const PendingRequest request = _pendingRequests.take(ticket);
    if (_entries.value(request.key).adapterId != request.adapterId)
    {
        return;
    }

    Entry entry;
    entry.adapterId = request.adapterId;
    if (result.value("valid").toBool())
    {
        entry.description.state = State::Valid;
        entry.description.description = result.value("description").toString();
        entry.description.fields = result.value("fields").toObject();
    }
    else
    {
        entry.description.state = State::Invalid;
        entry.description.error = result.value("error").toString();
    }

    _entries.insert(request.key, entry);
    emit descriptionChanged(request.key);
}

void DataPointDescriptionCache::onRequestFailed(quint64 ticket, const QString& error)
{
    if (!_pendingRequests.contains(ticket))
    {
        return;
    }
    const PendingRequest request = _pendingRequests.take(ticket);
    if (_entries.value(request.key).adapterId != request.adapterId)
    {
        return;
    }

    Entry entry;
    entry.adapterId = request.adapterId;
    entry.description.state = State::Failed;
    entry.description.error = error;
    entry.retryDeadline = QDeadlineTimer(_failedRetryMs);

    _entries.insert(request.key, entry);
    emit descriptionChanged(request.key);
}

/*!
 * \brief Forget the references that could not be described, so the next lookup asks again.
 */
void DataPointDescriptionCache::onAdaptersChanged()
{
    QStringList droppedKeys;
    for (auto it = _entries.begin(); it != _entries.end();)
    {
        const State state = it->description.state;
        if (state == State::Unavailable || state == State::Failed)
        {
            droppedKeys.append(it.key());
            it = _entries.erase(it);
        }
        else
        {
            ++it;
        }
    }

    for (const QString& key : std::as_const(droppedKeys))
    {
        emit descriptionChanged(key);
    }
}
