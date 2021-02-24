#include <QApplication>

#include "mainapp.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("ModbusScope");

    MainApp app(a.arguments());

    return a.exec();
}
