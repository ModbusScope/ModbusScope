
#include <QDateTime>

#include "diagnosticmodel.h"
#include "scopelogging.h"

Q_LOGGING_CATEGORY(scopeComm, "scope.comm")
Q_LOGGING_CATEGORY(scopeCommConnection, "scope.comm.connection")
Q_LOGGING_CATEGORY(scopeGeneralInfo, "scope.general.info")
Q_LOGGING_CATEGORY(scopePreset, "scope.preset")

ScopeLogging::ScopeLogging()
{
    _minSeverityLevel = Diagnostic::LOG_INFO;
    _pDiagnosticModel = nullptr;
}

void ScopeLogging::initLogging(DiagnosticModel* pDiagnosticModel)
{
    _pDiagnosticModel = pDiagnosticModel;
    _logStartTime = QDateTime::currentMSecsSinceEpoch();

    qInstallMessageHandler(ModbusScopeLog::messageHandler);

    /* Make sure debug messages are printed in console (disabled by default in Ubuntu) */
    QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true);

#ifdef VERBOSE
    // Enable to have internal QModbus debug messages
    QLoggingCategory::setFilterRules(QStringLiteral("qt.modbus* = true"));
    QLoggingCategory::setFilterRules(QStringLiteral("scope.connection* = false"));
#else
    //QLoggingCategory::setFilterRules("*=false\n");
#endif

}

/*!
 * \brief Set the minimum severity level of the allowed logs.
 *  All logs with higher severity level aren't saved.
 */
void ScopeLogging::setMinimumSeverityLevel(Diagnostic::LogSeverity minSeverity)
{
    _minSeverityLevel = minSeverity;
}

void ScopeLogging::handleLog(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Diagnostic::LogSeverity logSeverity;
    qint32 offset = static_cast<qint32>(QDateTime::currentMSecsSinceEpoch() - _logStartTime);

    switch(type)
    {
        case QtDebugMsg:
            logSeverity = Diagnostic::LOG_DEBUG;
            break;

        case QtInfoMsg:
            logSeverity = Diagnostic::LOG_INFO;
            break;

    case QtWarningMsg:
    default:
        logSeverity = Diagnostic::LOG_WARNING;
        break;
    }

    if (logSeverity <= _minSeverityLevel)
    {
        if (_pDiagnosticModel != nullptr)
        {
            _pDiagnosticModel->addLog(context.category, logSeverity, offset, msg);
        }
    }

#if 0
    QByteArray localMsg = msg.toLocal8Bit();

    fprintf(stderr, "%08d - %s\n", offset, localMsg.constData());
#endif
}

namespace ModbusScopeLog
{
    void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        ScopeLogging::Logger().handleLog(type, context, msg);
    }
}
