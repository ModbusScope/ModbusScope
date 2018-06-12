#include "errorlog.h"
#include "util.h"

/*!
 * \brief Creates new error log data class
 * \param category Category of log
 * \param timestamp Timestamp of log
 * \param message Extra information
 */
ErrorLog::ErrorLog(LogCategory category, QDateTime timestamp, QString message)
{
    _category = category;
    _timestamp = timestamp;
    _message = message;
}

/*!
 * \brief ErrorLog::category
 * \return Category of log
 */
ErrorLog::LogCategory ErrorLog::category() const
{
    return _category;
}

/*!
 * \brief ErrorLog::setCategory
 * \param category New category to set
 */
void ErrorLog::setCategory(const LogCategory &category)
{
    _category = category;
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
 * \brief ErrorLog::categoryString
 * \return Strings of category
 */
QString ErrorLog::categoryString() const
{
    switch (_category)
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
                          .arg(categoryString())
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
