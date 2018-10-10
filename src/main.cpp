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

    QLoggingCategory::setFilterRules("*.debug=false\n"
                                 #if 0
                                     "scope.notes*=true"
                                 #endif
                                     );

#ifndef DEBUG
    QLoggingCategory::setFilterRules("*=false\n");
#endif

    MainWindow w(a.arguments());

    w.show();

    return a.exec();
}
