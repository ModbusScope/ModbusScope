#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("ModbusScope");

    /* Make sure debug messages are printed in console (disabled by default in Ubuntu) */
    QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true);

    MainWindow w(a.arguments());

    w.show();

    return a.exec();
}
