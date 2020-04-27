#include "errorlog.h"
#include "util.h"

/*!
 * \brief Creates new error log data class
 * \param category Severity of log
 * \param timestamp Timestamp of log
 * \param message Extra information
 */
ErrorLog::ErrorLog(LogSeverity severity, QDateTime timestamp, QString message)
{
    _severity = severity;
    _timestamp = timestamp;
    _message = message;
}

/*!
 * \brief ErrorLog::severity
 * \return Severity of log
 */
ErrorLog::LogSeverity ErrorLog::severity() const
{
    return _severity;
}

/*!
 * \brief ErrorLog::setSeverity
 * \param severity New severity to set
 */
void ErrorLog::setSeverity(const LogSeverity &severity)
{
    _severity = severity;
}

/*!
 * \brief ErrorLog::message
 * \return Extra information of log
 */
QString ErrorLog::message() const
{
    return _message;
}

/*!
 * \brief ErrorLog::setMessage
 * \param message Extra information to set
 */
void ErrorLog::setMessage(const QString &message)
{
    _message = message;
}

/*!
 * \brief ErrorLog::timestamp
 * \return Timestamp of log
 */
QDateTime ErrorLog::timestamp() const
{
    return _timestamp;
}

/*!
 * \brief ErrorLog::setTimestamp
 * \param timestamp new timestamp for the log
 */
void ErrorLog::setTimestamp(const QDateTime &timestamp)
{
    _timestamp = timestamp;
}

/*!
 * \brief ErrorLog::severityString
 * \return Strings of severity
 */
QString ErrorLog::severityString() const
{
    switch (_severity)
    {
        case LOG_INFO:
            return QString("Info");
            break;

        case LOG_ERROR:
            return QString("Error");
            break;
        default:
            return QString("Unknown");
            break;
    }
}

/*!
 * \brief ErrorLog::toString
 * \return Printable summary of log
 */
QString ErrorLog::toString() const
{
    return QString("%1 [%2]: %3").arg(timestamp().toString(Util::timeStringFormat()))
                          .arg(severityString())
                          .arg(message());
}

/*!
 * \brief Operator overload to print log in debug window
 */
QDebug operator<<(QDebug debug, const ErrorLog &log)
{
    QDebugStateSaver saver(debug);

    debug.nospace() << log.toString();

    return debug;
}
