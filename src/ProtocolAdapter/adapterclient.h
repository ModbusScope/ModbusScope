#ifndef ADAPTERCLIENT_H
#define ADAPTERCLIENT_H

#include "ProtocolAdapter/adapterprocess.h"
#include "util/result.h"

#include <QJsonObject>
#include <QObject>
#include <QStringList>
#include <QTimer>

/*!
 * \brief Lifecycle manager for an external adapter process communicating via JSON-RPC 2.0.
 *
 * Drives the full adapter session sequence using an AdapterProcess for transport:
 *   startSession() → initialize → describe → configure → start → sessionStarted()
 *   requestReadData() → readData → readDataResult()
 *   requestStatus() → getStatus → statusResult()
 *   stopSession() → shutdown → process exit
 *
 * Takes ownership of the AdapterProcess passed to the constructor.
 */
class AdapterClient : public QObject
{
    Q_OBJECT

public:
    explicit AdapterClient(AdapterProcess* pProcess, QObject* parent = nullptr);
    ~AdapterClient();

    /*!
     * \brief Launch the adapter and run the initialization lifecycle up to describe.
     *
     * Starts the adapter at \a adapterPath, then sequentially sends
     * adapter.initialize and adapter.describe. After the describe response is received
     * and describeResult() is emitted, the client enters the AWAITING_CONFIG state.
     * The caller must then call provideConfig() to continue the lifecycle.
     *
     * \param adapterPath Path to the adapter executable.
     * \param registerExpressions Register expression strings passed to adapter.start.
     */
    void startSession(const QString& adapterPath, QStringList registerExpressions);

    /*!
     * \brief Provide the configuration to send to adapter.configure.
     *
     * Must be called after describeResult() has been emitted (i.e., in the
     * AWAITING_CONFIG state). Sends adapter.configure with the given config,
     * then continues to adapter.start and emits sessionStarted() on success.
     *
     * \param config JSON object passed as the \c config param to adapter.configure.
     */
    void provideConfig(QJsonObject config);

    /*!
     * \brief Send an adapter.readData request to the active adapter.
     *
     * Must only be called after sessionStarted() has been emitted.
     * Emits readDataResult() when the adapter responds.
     */
    void requestReadData();

    /*!
     * \brief Send an adapter.getStatus request to the active adapter.
     *
     * Must only be called after sessionStarted() has been emitted.
     * Emits statusResult() when the adapter responds.
     */
    void requestStatus();

    /*!
     * \brief Send adapter.shutdown and terminate the adapter process.
     */
    void stopSession();

signals:
    /*!
     * \brief Emitted when the adapter has been initialized, described, configured, and started.
     */
    void sessionStarted();

    /*!
     * \brief Emitted when an adapter.readData response has been received.
     * \param results One entry per register, in the same order as the expressions passed to startSession().
     */
    void readDataResult(ResultDoubleList results);

    /*!
     * \brief Emitted when an unrecoverable error occurs (process failure, RPC error).
     * \param message Human-readable error description.
     */
    void sessionError(QString message);

    /*!
     * \brief Emitted during startSession() when the adapter.describe response is received.
     * \param description The full describe result object (name, version, configVersion, schema, defaults,
     * capabilities).
     */
    void describeResult(QJsonObject description);

    /*!
     * \brief Emitted when an adapter.getStatus response has been received.
     * \param active true if the adapter is actively polling.
     */
    void statusResult(bool active);

protected:
    enum class State
    {
        IDLE,
        INITIALIZING,
        DESCRIBING,
        AWAITING_CONFIG,
        CONFIGURING,
        STARTING,
        ACTIVE,
        STOPPING
    };

    State _state{ State::IDLE };

private slots:
    void onResponseReceived(int id, const QString& method, const QJsonValue& result);
    void onErrorReceived(int id, const QString& method, const QJsonObject& error);
    void onProcessError(const QString& message);
    void onProcessFinished();
    void onHandshakeTimeout();

private:
    void handleLifecycleResponse(const QString& method, const QJsonObject& result);

    static constexpr int cHandshakeTimeoutMs = 10000;

    AdapterProcess* _pProcess;
    QTimer _handshakeTimer;
    QJsonObject _pendingConfig;
    QStringList _pendingExpressions;
};

#endif // ADAPTERCLIENT_H
