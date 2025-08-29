#include <QApplication>
#include "mainapp.h"

#ifdef WIN32
#include "Synchapi.h"
#endif

int main(int argc, char *argv[])
{
    // TODO: When porting to Qt 6.8, replace with QStyleHints::setColorScheme(Qt::ColorScheme scheme)
    qputenv("QT_QPA_PLATFORM", "windows:darkmode=0");

    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("ModbusScope");
    QCoreApplication::setApplicationName("ModbusScope");

#ifdef WIN32
    CreateMutexA(NULL, FALSE, "ModbusScope");
#endif

    MainApp app(a.arguments());

    return a.exec();
}
