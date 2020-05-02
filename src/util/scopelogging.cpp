
#include <QDateTime>

#include "diagnosticmodel.h"
#include "scopelogging.h"

Q_LOGGING_CATEGORY(scopeConnection, "scope.connection")
Q_LOGGING_CATEGORY(scopeModbus, "scope.modbus")
Q_LOGGING_CATEGORY(scopeNotes, "scope.notes")


ScopeLogging::ScopeLogging()
{
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
    QLoggingCategory::setFilterRules(v("qt.modbus* = true"));
    QLoggingCategory::setFilterRules(QStringLiteral("scope.connection* = false"));
#else
    //QLoggingCategory::setFilterRules("*=false\n");
#endif

}

void ScopeLogging::handleLog(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    switch(type)
    {
        case QtDebugMsg:
            break;

        case QtInfoMsg:
            break;

    case QtWarningMsg:
    default:

        break;
    }

    _pDiagnosticModel->addCommunicationLog(Diagnostic::LOG_INFO, msg);

    QByteArray localMsg = msg.toLocal8Bit();
    int offset = static_cast<int>(QDateTime::currentMSecsSinceEpoch() - _logStartTime);

    fprintf(stderr, "%08d - %s\n", offset, localMsg.constData());
}

namespace ModbusScopeLog
{
    void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        ScopeLogging::Logger().handleLog(type, context, msg);
    }
}
