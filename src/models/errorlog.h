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

    typedef enum
    {
        LOG_COMMUNICATION = 0,
    } LogCategory;

    explicit ErrorLog(LogCategory category, LogSeverity severity, QDateTime timestamp, QString message);

    LogCategory category() const;
    void setCategory(const LogCategory &category);

    LogSeverity severity() const;
    void setSeverity(const LogSeverity &severity);

    QString message() const;
    void setMessage(const QString &message);

    QDateTime timestamp() const;
    void setTimestamp(const QDateTime &timestamp);

    QString severityString() const;

    QString categoryString() const;

    QString toString() const;

private:

    LogCategory _category;
    LogSeverity _severity;
    QDateTime _timestamp;
    QString _message;
};

QDebug operator<<(QDebug debug, const ErrorLog &log);

#endif // ERRORLOG_H
