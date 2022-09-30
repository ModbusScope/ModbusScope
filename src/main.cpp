#include <QApplication>
#include "mainapp.h"

#ifdef WIN32
#include "Synchapi.h"
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("ModbusScope");

#ifdef WIN32
    CreateMutexA(NULL, FALSE, "ModbusScope");
#endif

    MainApp app(a.arguments());

    return a.exec();
}
