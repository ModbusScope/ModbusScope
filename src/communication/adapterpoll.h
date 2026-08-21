#ifndef ADAPTERPOLL_H
#define ADAPTERPOLL_H

#include "ProtocolAdapter/adapterhub.h"
#include "communication/datapoint.h"
#include "util/result.h"

#include <QMap>
#include <QSet>
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

    /*!
     * \brief Emitted when the adapter session fails fatally while communication is running or
     * still pending (including the WaitingForAdapter state, before any session has started).
     *
     * By the time this fires, polling has already been halted internally (pollState reset to
     * Inactive); callers must resync any "communication running" state of their own and surface
     * \a message to the user, since none of this is otherwise externally visible.
     */
    void communicationError(QString message);

private slots:
    void triggerRegisterRead();
    void onReadDataResult(const QString& adapterId, ResultDoubleList results);
    void onAdapterReady();
    void onSessionError(const QString& message);

private:
    /*!
     * \brief Tracks the register expressions and their original positions for one adapter.
     */
    struct AdapterGroup
    {
        QStringList expressions;
        QList<int> originalIndices;
    };

    void buildAdapterGroups(const QList<DataPoint>& registerList);
    void startSessions();

    enum class PollState
    {
        Inactive,
        WaitingForAdapter,
        Active
    };

    QList<DataPoint> _registerList;
    QMap<QString, AdapterGroup> _adapterGroups;
    QMap<QString, ResultDoubleList> _pendingResults;
    QSet<QString> _pendingResultAdapters;

    PollState _pollState = PollState::Inactive;
    QMetaObject::Connection _adapterReadyConnection;
    QTimer* _pPollTimer;
    qint64 _lastPollStart;

    SettingsModel* _pSettingsModel;
    AdapterHub* _pAdapterHub;
};

#endif // ADAPTERPOLL_H
