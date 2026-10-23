#ifndef DATAPOINTDESCRIPTIONCACHE_H
#define DATAPOINTDESCRIPTIONCACHE_H

#include "util/expressionreferences.h"

#include <QDeadlineTimer>
#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QString>

class DataPointService;

struct DataPointDescription
{
    enum class State
    {
        Pending,
        Valid,
        Invalid,
        Failed,
        Unavailable
    };

    State state{ State::Pending };
    QString description;
    QJsonObject fields;
    QString error;
};

class DataPointDescriptionCache : public QObject
{
    Q_OBJECT

public:
    static constexpr int cFailedRetryMs = 10000;

    explicit DataPointDescriptionCache(DataPointService* pService,
                                       QObject* parent = nullptr,
                                       int failedRetryMs = cFailedRetryMs);

    DataPointDescription lookup(const ExpressionReferences::Reference& reference);

signals:
    void descriptionChanged(const QString& referenceText);

private slots:
    void onDataPointDescribed(quint64 ticket, const QJsonObject& result);
    void onRequestFailed(quint64 ticket, const QString& error);
    void onAdaptersChanged();

private:
    struct Entry
    {
        DataPointDescription description;
        QDeadlineTimer retryDeadline;
        QString adapterId;
    };

    struct PendingRequest
    {
        QString key;
        QString adapterId;
    };

    DataPointService* _pService;
    int _failedRetryMs;

    QHash<QString, Entry> _entries;
    QHash<quint64, PendingRequest> _pendingRequests;
};

#endif // DATAPOINTDESCRIPTIONCACHE_H
