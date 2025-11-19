#include "mainapp.h"
#include <QApplication>
#include <QStyleHints>

#ifdef WIN32
#include "Synchapi.h"
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);
    QCoreApplication::setOrganizationName("ModbusScope");
    QCoreApplication::setApplicationName("ModbusScope");

#ifdef WIN32
    CreateMutexA(NULL, FALSE, "ModbusScope");
#endif

    MainApp app(a.arguments());

    return a.exec();
}
