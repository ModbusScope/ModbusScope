#ifndef MAINAPP_H
#define MAINAPP_H

#include <QObject>

class MainWindow;

class MainApp : public QObject
{
    Q_OBJECT
public:
    explicit MainApp(QStringList cmdArguments, QObject *parent = nullptr);
    ~MainApp();

signals:

private:
    MainWindow* _pMainWin;

};

#endif // MAINAPP_H
