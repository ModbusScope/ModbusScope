#ifndef ERRORLOG_H
#define ERRORLOG_H

#include <QString>
#include <QDateTime>
#include <QDebug>

class ErrorLog
{

public:

    typedef enum
    {
        LOG_INFO = 0,
        LOG_ERROR,
    } LogSeverity;

    explicit ErrorLog(ErrorLog::LogSeverity Severity, QDateTime timestamp, QString message);

    LogSeverity severity() const;
    void setSeverity(const LogSeverity &severity);

    QString message() const;
    void setMessage(const QString &message);

    QDateTime timestamp() const;
    void setTimestamp(const QDateTime &timestamp);

    QString severityString() const;

    QString toString() const;

private:

    LogSeverity _severity;
    QDateTime _timestamp;
    QString _message;
};

QDebug operator<<(QDebug debug, const ErrorLog &log);

#endif // ERRORLOG_H
