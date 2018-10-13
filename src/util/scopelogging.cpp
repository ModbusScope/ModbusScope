
#include <QDateTime>

#include "scopelogging.h"


Q_LOGGING_CATEGORY(scopeConnection, "scope.connection")
Q_LOGGING_CATEGORY(scopeModbus, "scope.modbus")
Q_LOGGING_CATEGORY(scopeNotes, "scope.notes")

namespace ScopeLogging
{
    static qint64 logStartTime;

    void startLogging()
    {
        logStartTime = QDateTime::currentMSecsSinceEpoch();
    }

    void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        Q_UNUSED(type);
        Q_UNUSED(context);

        QByteArray localMsg = msg.toLocal8Bit();
        int offset = static_cast<int>(QDateTime::currentMSecsSinceEpoch() - logStartTime);

        fprintf(stderr, "%08d - %s\n", offset, localMsg.constData());
    }
}
