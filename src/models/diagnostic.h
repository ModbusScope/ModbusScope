#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

#include <QString>
#include <QDateTime>
#include <QDebug>

class Diagnostic
{

public:

    typedef enum
    {
        LOG_WARNING = 0,
        LOG_INFO,
        LOG_DEBUG
    } LogSeverity;

    explicit Diagnostic(QString category, LogSeverity severity, QDateTime timestamp, QString message);

    QString category() const;
    void setCategory(const QString &category);

    LogSeverity severity() const;
    void setSeverity(const LogSeverity &severity);

    QString message() const;
    void setMessage(const QString &message);

    QDateTime timestamp() const;
    void setTimestamp(const QDateTime &timestamp);

    QString severityString() const;

    QString toString() const;

private:

    QString _category;
    LogSeverity _severity;
    QDateTime _timestamp;
    QString _message;
};

QDebug operator<<(QDebug debug, const Diagnostic &log);

#endif // DIAGNOSTIC_H
