#ifndef SCOPELOGGING_H
#define SCOPELOGGING_H

#include <QLoggingCategory>
#include "diagnosticmodel.h"

Q_DECLARE_LOGGING_CATEGORY(scopeConnection)
Q_DECLARE_LOGGING_CATEGORY(scopeModbus)
Q_DECLARE_LOGGING_CATEGORY(scopeNotes)

class ScopeLogging
{
public:
    explicit ScopeLogging();

    static ScopeLogging& Logger();

    void initLogging(DiagnosticModel* pDiagnosticModel);

    void handleLog(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
    qint64 _logStartTime;

    DiagnosticModel* _pDiagnosticModel;
};

inline ScopeLogging& ScopeLogging::Logger()
{
    static ScopeLogging scopeLogger;
    return scopeLogger;
}

namespace ModbusScopeLog
{
    void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
}

#endif // SCOPELOGGING_H
