#ifndef DATAPOINTSERVICE_H
#define DATAPOINTSERVICE_H

#include "models/device.h"

#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QSet>
#include <QString>

class AdapterHub;
class AdapterManager;
class SettingsModel;

/*!
 * \brief Routes data point requests for a device to the adapter that serves it.
 *
 * Callers identify a request by the ticket returned when it is sent. Every ticket ends in exactly one
 * of expressionBuilt(), dataPointDescribed() or requestFailed(). requestFailed() is always delivered
 * on a later event loop turn, so a caller never receives it from inside the call that sent the request.
 */
class DataPointService : public QObject
{
    Q_OBJECT

public:
    static constexpr int cRequestTimeoutMs = 3000;

    explicit DataPointService(SettingsModel* pSettingsModel,
                              AdapterHub* pAdapterHub,
                              QObject* parent = nullptr,
                              int requestTimeoutMs = cRequestTimeoutMs);

    bool isDeviceUsable(deviceId_t deviceId) const;
    QString adapterIdForDevice(deviceId_t deviceId) const;
    QJsonObject addressSchema(deviceId_t deviceId) const;
    QJsonObject defaults(deviceId_t deviceId) const;

    quint64 buildExpression(deviceId_t deviceId, const QJsonObject& fields);
    quint64 describeDataPoint(deviceId_t deviceId, const QString& reference);

signals:
    void expressionBuilt(quint64 ticket, QString expression);
    void dataPointDescribed(quint64 ticket, QJsonObject result);
    void requestFailed(quint64 ticket, QString error);
    void adaptersChanged();

private slots:
    void emitQueuedFailures();
    void onManagerDestroyed(QObject* pObject);

private:
    struct PendingRequest
    {
        AdapterManager* pManager;
        int requestId;
    };

    struct QueuedFailure
    {
        quint64 ticket;
        QString error;
    };

    AdapterManager* usableManager(deviceId_t deviceId) const;
    QJsonObject dataPointSchema(deviceId_t deviceId) const;
    quint64 track(AdapterManager* pManager, int requestId);
    quint64 takeTicket(AdapterManager* pManager, int requestId);
    void failLater(quint64 ticket, const QString& error);
    void connectManager(AdapterManager* pManager);
    void onExpressionBuilt(AdapterManager* pManager, int requestId, const QString& expression);
    void onDataPointDescribed(AdapterManager* pManager, int requestId, const QJsonObject& result);
    void onRequestFailed(AdapterManager* pManager, int requestId, const QString& error);
    void onRequestTimeout(quint64 ticket);
    QString unusableReason(deviceId_t deviceId) const;

    SettingsModel* _pSettingsModel;
    AdapterHub* _pAdapterHub;
    int _requestTimeoutMs;

    quint64 _lastTicket{ 0 };
    QHash<quint64, PendingRequest> _pendingRequests;
    QList<QueuedFailure> _queuedFailures;
    QSet<QObject*> _connectedManagers;
};

#endif // DATAPOINTSERVICE_H
