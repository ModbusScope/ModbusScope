#ifndef ADAPTERPOLL_H
#define ADAPTERPOLL_H

#include "ProtocolAdapter/adaptermanager.h"
#include "communication/datapoint.h"
#include "util/result.h"

#include <QStringList>
#include <QTimer>

// Forward declaration
class SettingsModel;

class AdapterPoll : public QObject
{
    Q_OBJECT
public:
    explicit AdapterPoll(SettingsModel* pSettingsModel, QObject* parent = nullptr);

    /*!
     * \brief Constructor for testing: accepts a pre-built AdapterManager instead of creating one.
     *
     * The caller retains ownership of \a pAdapterManager; it is not reparented.
     */
    explicit AdapterPoll(SettingsModel* pSettingsModel, AdapterManager* pAdapterManager, QObject* parent = nullptr);

    ~AdapterPoll();

    void initAdapter();
    void startCommunication(QList<DataPoint>& registerList);
    void stopCommunication();

    bool isActive() const;
    void resetCommunicationStats();

    /*!
     * \brief Returns the AdapterManager owned by this poll instance.
     *
     * Callers that need to interact with the adapter directly (e.g. to call
     * buildExpression()) should use this accessor rather than going through AdapterPoll.
     */
    AdapterManager* adapterManager() const;

signals:
    void registerDataReady(ResultDoubleList registers);

private slots:
    void triggerRegisterRead();
    void onReadDataResult(ResultDoubleList results);
    void onAdapterReady();
    void onSessionError(const QString& message);

private:
    QStringList buildRegisterExpressions(const QList<DataPoint>& registerList);

    enum class PollState
    {
        Inactive,
        WaitingForAdapter,
        Active
    };

    QList<DataPoint> _registerList;
    QStringList _pendingExpressions;

    PollState _pollState = PollState::Inactive;
    QMetaObject::Connection _adapterReadyConnection;
    QTimer* _pPollTimer;
    qint64 _lastPollStart;

    SettingsModel* _pSettingsModel;
    AdapterManager* _pAdapterManager;
};

#endif // ADAPTERPOLL_H
