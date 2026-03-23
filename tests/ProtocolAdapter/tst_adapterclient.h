#ifndef TST_ADAPTERCLIENT_H
#define TST_ADAPTERCLIENT_H

#include <QObject>

class TestAdapterClient : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void lifecycleInitializeToStart();
    void describeSignalEmitted();
    void readDataValidResults();
    void readDataEmptyRegisters();
    void requestStatusEmitsSignal();
    void errorResponseEmitsSessionError();
    void unexpectedResponseEmitsNoSignals();
    void notificationIgnored();
    void processErrorEmitsSessionError();
};

#endif // TST_ADAPTERCLIENT_H
