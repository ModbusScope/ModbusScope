#ifndef SCOPELOGGING_H
#define SCOPELOGGING_H

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(scopeCommConnection)
Q_DECLARE_LOGGING_CATEGORY(scopeComm)
Q_DECLARE_LOGGING_CATEGORY(scopeGeneralInfo)
Q_DECLARE_LOGGING_CATEGORY(scopePreset)
Q_DECLARE_LOGGING_CATEGORY(scopeUi)

// forward declaration
class DiagnosticModel;

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
