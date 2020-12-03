#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

#include <QString>
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

    explicit Diagnostic(QString category, LogSeverity severity, qint32 timeOffset, QString message);

    QString category() const;
    void setCategory(const QString &category);

    LogSeverity severity() const;
    void setSeverity(const LogSeverity &severity);

    QString message() const;
    void setMessage(const QString &message);

    qint32 timeOffset() const;
    void setTimeOffset(const qint32 &timeOffset);

    QString severityString() const;

    QString toString() const;
    QString toExportString() const;

private:

    QString _category;
    LogSeverity _severity;
    qint32 _timeOffset;
    QString _message;
};

QDebug operator<<(QDebug debug, const Diagnostic &log);

#endif // DIAGNOSTIC_H
