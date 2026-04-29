#ifndef ADAPTERPROCESS_H
#define ADAPTERPROCESS_H

#include "ProtocolAdapter/framingreader.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QMap>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QTimer>

/*!
 * \brief Transport layer for an external adapter process communicating via JSON-RPC 2.0 over stdio.
 *
 * Owns the QProcess and FramingReader. Responsible for launching the adapter binary,
 * writing Content-Length framed requests, and parsing incoming responses.
 *
 * Pending request IDs are tracked internally so that the method name is included
 * in the responseReceived() and errorReceived() signals.
 *
 * Virtual methods (start, stop, sendRequest) allow a mock subclass in unit tests.
 */
class AdapterProcess : public QObject
{
    Q_OBJECT

public:
    explicit AdapterProcess(QObject* parent = nullptr);
    virtual ~AdapterProcess() = default;

    /*!
     * \brief Launch the adapter process at the given path.
     * \param path Absolute or relative path to the adapter executable.
     * \return true if the process started within the timeout, false otherwise.
     */
    virtual bool start(const QString& path);

    /*!
     * \brief Signal the adapter process to stop and return immediately.
     *
     * Closes stdin so the adapter exits cleanly. If it has not exited within
     * the stop timeout, it is killed. The \c processFinished signal is emitted
     * asynchronously when the process actually exits.
     */
    virtual void stop();

    /*!
     * \brief Send a JSON-RPC 2.0 request to the adapter.
     * \param method The JSON-RPC method name.
     * \param params The params object (may be empty).
     * \return The request ID assigned to this request, or -1 on write failure.
     */
    virtual int sendRequest(const QString& method, const QJsonObject& params);

    /*!
     * \brief Returns true if the adapter process is currently running.
     */
    bool isRunning() const;

signals:
    /*!
     * \brief Emitted when a successful JSON-RPC response is received.
     * \param id The request ID from the original request.
     * \param method The method name of the original request.
     * \param result The result value from the response.
     */
    void responseReceived(int id, QString method, QJsonValue result);

    /*!
     * \brief Emitted when a JSON-RPC error response is received.
     * \param id The request ID from the original request.
     * \param method The method name of the original request.
     * \param error The error object from the response.
     */
    void errorReceived(int id, QString method, QJsonObject error);

    /*!
     * \brief Emitted on process-level errors (start failure, crash, write failure).
     * \param message Human-readable error description.
     */
    void processError(QString message);

    /*!
     * \brief Emitted when a server-initiated JSON-RPC notification is received.
     * \param method The notification method name.
     * \param params The params value from the notification (may be undefined).
     */
    void notificationReceived(QString method, QJsonValue params);

    /*!
     * \brief Emitted when the adapter process exits.
     */
    void processFinished();

private slots:
    void onReadyReadStdout();
    void onReadyReadStderr();
    void onMessageReceived(QByteArray body);
    void onProcessError(QProcess::ProcessError error);
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    bool writeFramed(const QByteArray& json);

    QProcess* _pProcess;
    QTimer* _pKillTimer;
    FramingReader* _pFramingReader;
    QMap<int, QString> _pendingMethods;
    int _nextRequestId{ 1 };
};

#endif // ADAPTERPROCESS_H
