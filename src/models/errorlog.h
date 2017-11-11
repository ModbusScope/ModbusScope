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
    } LogCategory;

    explicit ErrorLog(ErrorLog::LogCategory category, QDateTime timestamp, QString message);

    LogCategory category() const;
    void setCategory(const LogCategory &category);

    QString message() const;
    void setMessage(const QString &message);

    QDateTime timestamp() const;
    void setTimestamp(const QDateTime &timestamp);

    QString categoryString() const;

    QString toString() const;

private:

    LogCategory _category;
    QDateTime _timestamp;
    QString _message;
};

QDebug operator<<(QDebug debug, const ErrorLog &log);

#endif // ERRORLOG_H
