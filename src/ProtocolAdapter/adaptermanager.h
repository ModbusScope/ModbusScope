#ifndef ADAPTERMANAGER_H
#define ADAPTERMANAGER_H

#include "models/device.h"
#include "util/result.h"

#include <QJsonObject>
#include <QObject>
#include <QStringList>

class AdapterClient;
class AdapterProcess;
class SettingsModel;

/*!
 * \brief Owns the adapter subprocess and drives the full adapter session lifecycle.
 *
 * Creates and holds AdapterProcess and AdapterClient. Handles initialization,
 * describe, schema retrieval, configuration, and session management. Exposes
 * only the signals needed to coordinate data polling.
 */
class AdapterManager : public QObject
{
    Q_OBJECT
public:
    explicit AdapterManager(SettingsModel* pSettingsModel, QObject* parent = nullptr);

    /*!
     * \brief Launch the adapter subprocess and begin the initialization handshake.
     *
     * Resolves the adapter binary path relative to the running executable and
     * calls AdapterClient::prepareAdapter().
     */
    void initAdapter();

    /*!
     * \brief Provide register expressions to the adapter and start the session.
     *
     * Fetches the effective adapter configuration from the settings model, then
     * calls AdapterClient::provideConfig() to configure and start the adapter.
     *
     * \param registerExpressions Register expression strings to pass to the adapter.
     */
    void startSession(const QStringList& registerExpressions);

    /*!
     * \brief Send adapter.shutdown and signal the adapter process to stop.
     */
    void stopSession();

    /*!
     * \brief Send an adapter.readData request to the active adapter.
     */
    void requestReadData();

    /*!
     * \brief Request the adapter to construct a register expression from its component parts.
     *
     * \param addressFields Address field values as returned by the register schema form.
     * \param dataType      Data type identifier (e.g. "16b"). Pass empty string for the adapter default.
     * \param deviceId      Device identifier. Pass 0 for the adapter default.
     */
    virtual void buildExpression(const QJsonObject& addressFields, const QString& dataType, deviceId_t deviceId);

    /*!
     * \brief Send an adapter.expressionHelp request to retrieve expression syntax help text.
     */
    virtual void requestExpressionHelp();

    /*!
     * \brief Route an adapter.diagnostic notification to the diagnostics log.
     *
     * Public for testability. Maps the adapter's level string to the appropriate
     * Qt logging severity.
     *
     * \param level   Severity string from the adapter: "debug", "info", "warning", or "error".
     * \param message The diagnostic message text.
     */
    void onAdapterDiagnostic(const QString& level, const QString& message);

signals:
    //! Emitted when the adapter has been initialized, described, configured, and started.
    void sessionStarted();

    //! Emitted when the adapter process has been intentionally stopped and the client is IDLE.
    void sessionStopped();

    /*!
     * \brief Emitted when an unrecoverable error occurs (process failure, RPC error).
     * \param message Human-readable error description.
     */
    void sessionError(QString message);

    /*!
     * \brief Emitted when an adapter.readData response has been received.
     * \param results One entry per register, in the same order as the expressions passed to startSession().
     */
    void readDataResult(ResultDoubleList results);

    /*!
     * \brief Emitted when an adapter.buildExpression response has been received.
     * \param expression The constructed register expression string (e.g. \c ${h0:f32b}).
     */
    void buildExpressionResult(QString expression);

    /*!
     * \brief Emitted when an adapter.expressionHelp response has been received.
     * \param helpText HTML string describing expression syntax.
     */
    void expressionHelpResult(QString helpText);

private slots:
    void onDescribeResult(const QJsonObject& description);
    void onDataPointSchemaResult(const QJsonObject& schema);

private:
    SettingsModel* _pSettingsModel;
    AdapterProcess* _pAdapterProcess;
    AdapterClient* _pAdapterClient;
};

#endif // ADAPTERMANAGER_H
