
#include "util/scopelogging.h"
#include "models/diagnosticmodel.h"

#include <QDateTime>
#include <QMetaObject>

Q_LOGGING_CATEGORY(scopeComm, "scope.comm")
Q_LOGGING_CATEGORY(scopeAdapter, "scope.comm.adapter")
Q_LOGGING_CATEGORY(scopeGeneralInfo, "scope.general.info")
Q_LOGGING_CATEGORY(scopePreset, "scope.preset")
Q_LOGGING_CATEGORY(scopeUi, "scope.ui")

namespace {

/*!
 * \brief Functor that delivers a single log entry to a DiagnosticModel
 *
 * Used with QMetaObject::invokeMethod() to move the addLog() call onto the model's own
 * thread, since ScopeLogging::handleLog() may be invoked by Qt from any thread.
 */
struct LogDispatcher
{
    DiagnosticModel* pModel;
    QString category;
    Diagnostic::LogSeverity severity;
    qint32 offset;
    QString message;

    void operator()() const
    {
        pModel->addLog(category, severity, offset, message);
    }
};

} // namespace

ScopeLogging::ScopeLogging()
{
    _pDiagnosticModel = nullptr;
    _logStartTime = 0;
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
    // QLoggingCategory::setFilterRules("*=false\n");
#endif
}

/*!
 * \brief Route a Qt log message to the diagnostic model and, when enabled, the debug log file
 *
 * Installed via qInstallMessageHandler(), which Qt may invoke from any thread that logs a
 * message. DiagnosticModel is not thread-safe, so DiagnosticModel::addLog() is dispatched onto
 * the model's own (GUI) thread instead of being called directly. DebugLogFileWriter guards its
 * own state internally, so it can be accessed directly from whichever thread is logging.
 */
void ScopeLogging::handleLog(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    Diagnostic::LogSeverity logSeverity;
    qint32 offset = static_cast<qint32>(QDateTime::currentMSecsSinceEpoch() - _logStartTime);

    switch (type)
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

    if (_pDiagnosticModel != nullptr)
    {
        const QString category = QString::fromUtf8(context.category);

        QMetaObject::invokeMethod(_pDiagnosticModel,
                                  LogDispatcher{ _pDiagnosticModel, category, logSeverity, offset, msg });
    }

    if (_debugLogFileWriter.isEnabled())
    {
        Diagnostic log(context.category, logSeverity, offset, msg);
        _debugLogFileWriter.writeLine(log.toExportString());
    }

#if 0
    QByteArray localMsg = msg.toLocal8Bit();

    fprintf(stderr, "%08d - %s\n", offset, localMsg.constData());
#endif
}

/*!
 * \brief Enable or disable automatically appending all log messages to a file in the temp folder
 * \param bEnabled  When true, log messages are appended (never truncating existing content)
 * \return True on success, false when enabling failed (e.g. the temp folder is not writable)
 */
bool ScopeLogging::setDebugFileLoggingEnabled(bool bEnabled)
{
    return _debugLogFileWriter.setEnabled(bEnabled);
}

namespace ModbusScopeLog {
void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    ScopeLogging::Logger().handleLog(type, context, msg);
}
} // namespace ModbusScopeLog
