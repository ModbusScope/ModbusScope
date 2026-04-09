#ifndef ADAPTERCLIENT_H
#define ADAPTERCLIENT_H

#include "ProtocolAdapter/adapterprocess.h"
#include "models/device.h"
#include "util/result.h"

#include <QJsonObject>
#include <QObject>
#include <QStringList>
#include <QTimer>

/*!
 * \brief Lifecycle manager for an external adapter process communicating via JSON-RPC 2.0.
 *
 * Drives the full adapter session sequence using an AdapterProcess for transport:
 *   prepareAdapter() → initialize → describe → describeResult()
 *   provideConfig()  → configure  → start    → sessionStarted()
 *   requestReadData() → readData → readDataResult()
 *   requestStatus() → getStatus → statusResult()
 *   stopSession() → shutdown → process exit → sessionStopped()
 *
 * Takes ownership of the AdapterProcess passed to the constructor.
 */
class AdapterClient : public QObject
{
    Q_OBJECT

public:
    explicit AdapterClient(AdapterProcess* pProcess, QObject* parent = nullptr, int handshakeTimeoutMs = 10000);
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
     */
    void prepareAdapter(const QString& adapterPath);

    /*!
     * \brief Provide the configuration and register expressions to send to adapter.configure and adapter.start.
     *
     * Must be called after describeResult() has been emitted (i.e., in the
     * AWAITING_CONFIG state). Sends adapter.configure with the given config,
     * then continues to adapter.start with the register expressions, and emits
     * sessionStarted() on success.
     *
     * \param config JSON object passed as the \c config param to adapter.configure.
     * \param registerExpressions Register expression strings passed to adapter.start.
     */
    void provideConfig(QJsonObject config, QStringList registerExpressions);

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
     * \brief Send adapter.shutdown and signal the adapter process to stop.
     *
     * The sessionStopped() signal is emitted asynchronously once the process
     * has fully exited.
     */
    void stopSession();

    /*!
     * \brief Send an adapter.dataPointSchema request to discover the data point UI schema.
     *
     * Must only be called after describeResult() has been emitted (i.e., in the
     * AWAITING_CONFIG state). Emits dataPointSchemaResult() when the adapter responds.
     */
    void requestDataPointSchema();

    /*!
     * \brief Send an adapter.describeDataPoint request to parse a data point expression.
     *
     * Can be called in AWAITING_CONFIG or ACTIVE state.
     * Emits describeDataPointResult() when the adapter responds.
     *
     * \param expression The data point expression string to describe.
     */
    void describeDataPoint(const QString& expression);

    /*!
     * \brief Send an adapter.validateDataPoint request to validate a data point expression.
     *
     * Can be called in AWAITING_CONFIG or ACTIVE state.
     * Emits validateDataPointResult() when the adapter responds.
     *
     * \param expression The data point expression string to validate.
     */
    void validateDataPoint(const QString& expression);

    /*!
     * \brief Send an adapter.buildExpression request to construct a data point expression string.
     *
     * Can be called in AWAITING_CONFIG or ACTIVE state.
     * Emits buildExpressionResult() when the adapter responds.
     *
     * \param addressFields Address field values as returned by the data point schema form (e.g. objectType, address).
     * \param dataType      Data type identifier (e.g. "16b"). Omitted from params when empty; adapter uses its default.
     * \param deviceId      Device identifier. Omitted from params when zero; adapter uses its default.
     */
    void buildExpression(const QJsonObject& addressFields, const QString& dataType, deviceId_t deviceId);

signals:
    /*!
     * \brief Emitted when the adapter has been initialized, described, configured, and started.
     */
    void sessionStarted();

    /*!
     * \brief Emitted when an adapter.readData response has been received.
     * \param results One entry per register, in the same order as the expressions passed to provideConfig().
     */
    void readDataResult(ResultDoubleList results);

    /*!
     * \brief Emitted when an unrecoverable error occurs (process failure, RPC error).
     * \param message Human-readable error description.
     */
    void sessionError(QString message);

    /*!
     * \brief Emitted during prepareAdapter() when the adapter.describe response is received.
     * \param description The full describe result object (name, version, configVersion, schema, defaults,
     * capabilities).
     */
    void describeResult(QJsonObject description);

    /*!
     * \brief Emitted when an adapter.getStatus response has been received.
     * \param active true if the adapter is actively polling.
     */
    void statusResult(bool active);

    /*!
     * \brief Emitted when the adapter process has been intentionally stopped and the client is IDLE.
     *
     * This signal is not emitted on unexpected process exits or errors — only after
     * a successful stopSession() call. The caller may use this to re-invoke prepareAdapter().
     */
    void sessionStopped();

    /*!
     * \brief Emitted when an adapter.diagnostic notification is received from the adapter.
     * \param level Severity level string: "debug", "info", or "warning".
     * \param message The diagnostic message from the adapter.
     */
    void diagnosticReceived(QString level, QString message);

    /*!
     * \brief Emitted when an adapter.dataPointSchema response has been received.
     * \param schema The full data point schema object (addressSchema, dataTypes, defaultDataType).
     */
    void dataPointSchemaResult(QJsonObject schema);

    /*!
     * \brief Emitted when an adapter.describeDataPoint response has been received.
     * \param result The full result object (valid, fields, description or error).
     */
    void describeDataPointResult(QJsonObject result);

    /*!
     * \brief Emitted when an adapter.validateDataPoint response has been received.
     * \param valid Whether the expression is valid.
     * \param error Human-readable error message when valid is false; empty otherwise.
     */
    void validateDataPointResult(bool valid, QString error);

    /*!
     * \brief Emitted when an adapter.buildExpression response has been received.
     * \param expression The constructed data point expression string (e.g. \c ${h0:f32b}).
     */
    void buildExpressionResult(QString expression);

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
    void onNotificationReceived(QString method, QJsonValue params);

private:
    void handleLifecycleResponse(const QString& method, const QJsonObject& result);

    static constexpr int cHandshakeTimeoutMs = 10000;

    AdapterProcess* _pProcess;
    QTimer _handshakeTimer;
    int _handshakeTimeoutMs;
    QJsonObject _pendingConfig;
    QStringList _pendingExpressions;
};

#endif // ADAPTERCLIENT_H
