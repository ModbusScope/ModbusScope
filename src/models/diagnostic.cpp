#include "diagnostic.h"
#include "util.h"

/*!
 * \brief Creates new error log data class
 * \param category Severity of log
 * \param timestamp Timestamp of log
 * \param message Extra information
 */
Diagnostic::Diagnostic(QString category, LogSeverity severity, qint32 timeOffset, QString message)
{
    _category = category;
    _severity = severity;
    _timeOffset = timeOffset;
    _message = message;
}

QString Diagnostic::category() const
{
    return _category;
}

void Diagnostic::setCategory(const QString &category)
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
qint32 Diagnostic::timeOffset() const
{
    return _timeOffset;
}

/*!
 * \brief Diagnostic::setTimestamp
 * \param timestamp new timestamp for the log
 */
void Diagnostic::setTimeOffset(const qint32 &timeOffset)
{
    _timeOffset = timeOffset;
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
            return QString("INFO");
            break;

        case LOG_WARNING:
            return QString("WARNING");
            break;

        case LOG_DEBUG:
            return QString("DEBUG");
            break;

        default:
            return QString("UNKNOWN");
            break;
    }
}

/*!
 * \brief Diagnostic::toString
 * \return Printable summary of log
 */
QString Diagnostic::toString() const
{

    return QString("%1 - %2 [%3]: %4")
                          .arg(timeOffset(), 8, 10, QChar('0'))
                          .arg(severityString())
                          .arg(category())
                          .arg(message());
}

/*!
 * \brief Diagnostic::toString
 * \return Printable summary of log
 */
QString Diagnostic::toExportString() const
{

    return QString("%1:%2;%3;%4")
                          .arg(timeOffset(), 8, 10, QChar('0'))
                          .arg(severityString())
                          .arg(category())
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
