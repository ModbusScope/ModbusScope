#include "mainapp.h"

#include "mainwindow.h"

MainApp::MainApp(QStringList cmdArguments, QObject *parent) : QObject(parent)
{
    _pMainWin = new MainWindow(cmdArguments);

    _pMainWin->show();
}

MainApp::~MainApp()
{
    delete _pMainWin;
}
