#ifndef SCOPELOGGING_H
#define SCOPELOGGING_H

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(scopeModbus)
Q_DECLARE_LOGGING_CATEGORY(scopeNotes)

namespace ScopeLogging
{
    void startLogging();
    void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
};


#endif // SCOPELOGGING_H
