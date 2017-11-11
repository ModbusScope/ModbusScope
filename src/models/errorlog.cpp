#include "errorlog.h"
#include "util.h"

ErrorLog::ErrorLog(LogCategory category, QDateTime timestamp, QString message)
{
    _category = category;
    _timestamp = timestamp;
    _message = message;
}

ErrorLog::LogCategory ErrorLog::category() const
{
    return _category;
}

void ErrorLog::setCategory(const LogCategory &category)
{
    _category = category;
}

QString ErrorLog::message() const
{
    return _message;
}

void ErrorLog::setMessage(const QString &message)
{
    _message = message;
}

QDateTime ErrorLog::timestamp() const
{
    return _timestamp;
}

void ErrorLog::setTimestamp(const QDateTime &timestamp)
{
    _timestamp = timestamp;
}

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

QString ErrorLog::toString() const
{
    return QString("%1 [%2]: %3").arg(timestamp().toString(Util::timeStringFormat()))
                          .arg(categoryString())
                          .arg(message());
}

QDebug operator<<(QDebug debug, const ErrorLog &log)
{
    QDebugStateSaver saver(debug);

    //debug.nospace() << log.timestamp().toString(Util::timeStringFormat()) << " [" << log.categoryString() << "]: " << log.message();
    debug.nospace() << log.toString();

    return debug;
}
