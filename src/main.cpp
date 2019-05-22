#include "mainwindow.h"
#include "scopelogging.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("ModbusScope");

    ScopeLogging::startLogging();
    qInstallMessageHandler(ScopeLogging::messageHandler);

    /* Make sure debug messages are printed in console (disabled by default in Ubuntu) */
    QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true);

#ifdef VERBOSE
    // Enable to have internal QModbus debug messages
    QLoggingCategory::setFilterRules(QStringLiteral("qt.modbus* = true"));
    QLoggingCategory::setFilterRules(QStringLiteral("scope.connection* = false"));
#else
    QLoggingCategory::setFilterRules("*=false\n");
#endif

    MainWindow w(a.arguments());

    w.show();

    return a.exec();
}
