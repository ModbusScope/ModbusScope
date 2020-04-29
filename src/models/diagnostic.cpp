#include "diagnostic.h"
#include "util.h"

/*!
 * \brief Creates new error log data class
 * \param category Severity of log
 * \param timestamp Timestamp of log
 * \param message Extra information
 */
Diagnostic::Diagnostic(LogCategory category, LogSeverity severity, QDateTime timestamp, QString message)
{
    _category = category;
    _severity = severity;
    _timestamp = timestamp;
    _message = message;
}

Diagnostic::LogCategory Diagnostic::category() const
{
    return _category;
}

void Diagnostic::setCategory(const Diagnostic::LogCategory &category)
{
    _category = category;
}

/*!
 * \brief Diagnostic::severity
 * \return Severity of log
 */
Diagnostic::LogSeverity Diagnostic::severity() const
{
    return _severity;
}

/*!
 * \brief Diagnostic::setSeverity
 * \param severity New severity to set
 */
void Diagnostic::setSeverity(const LogSeverity &severity)
{
    _severity = severity;
}

/*!
 * \brief Diagnostic::message
 * \return Extra information of log
 */
QString Diagnostic::message() const
{
    return _message;
}

/*!
 * \brief Diagnostic::setMessage
 * \param message Extra information to set
 */
void Diagnostic::setMessage(const QString &message)
{
    _message = message;
}

/*!
 * \brief Diagnostic::timestamp
 * \return Timestamp of log
 */
QDateTime Diagnostic::timestamp() const
{
    return _timestamp;
}

/*!
 * \brief Diagnostic::setTimestamp
 * \param timestamp new timestamp for the log
 */
void Diagnostic::setTimestamp(const QDateTime &timestamp)
{
    _timestamp = timestamp;
}

/*!
 * \brief Diagnostic::severityString
 * \return Strings of severity
 */
QString Diagnostic::severityString() const
{
    switch (_severity)
    {
        case LOG_INFO:
            return QString("Info");
            break;

        case LOG_ERROR:
            return QString("Error");
            break;

        case LOG_DEBUG:
            return QString("Debug");
            break;

        default:
            return QString("Unknown");
            break;
    }
}

/*!
 * \return Strings of category
 */
QString Diagnostic::categoryString() const
{
    switch (_category)
    {
        case LOG_COMMUNICATION:
            return QString("Communication");
            break;
        default:
            return QString("Unknown");
            break;
    }
}

/*!
 * \brief Diagnostic::toString
 * \return Printable summary of log
 */
QString Diagnostic::toString() const
{
    return QString("%1 - %2 [%3]: %4").arg(timestamp().toString(Util::timeStringFormat()))
                          .arg(categoryString())
                          .arg(severityString())
                          .arg(message());
}

/*!
 * \brief Operator overload to print log in debug window
 */
QDebug operator<<(QDebug debug, const Diagnostic &log)
{
    QDebugStateSaver saver(debug);

    debug.nospace() << log.toString();

    return debug;
}
