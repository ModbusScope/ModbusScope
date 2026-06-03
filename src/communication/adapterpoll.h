#ifndef ADAPTERPOLL_H
#define ADAPTERPOLL_H

#include "ProtocolAdapter/adapterhub.h"
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
     * \brief Constructor for testing: accepts a pre-built AdapterHub instead of creating one.
     *
     * The caller retains ownership of \a pAdapterHub; it is not reparented.
     */
    explicit AdapterPoll(SettingsModel* pSettingsModel, AdapterHub* pAdapterHub, QObject* parent = nullptr);

    ~AdapterPoll();

    void initAdapter();
    void startCommunication(QList<DataPoint>& registerList);
    void stopCommunication();

    bool isActive() const;
    void resetCommunicationStats();

    /*!
     * \brief Returns the AdapterHub owned by this poll instance.
     *
     * Callers that need to interact with an adapter directly (e.g. to call
     * buildExpression()) should obtain the relevant AdapterManager from the hub.
     */
    AdapterHub* adapterHub() const;

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
    AdapterHub* _pAdapterHub;
};

#endif // ADAPTERPOLL_H
