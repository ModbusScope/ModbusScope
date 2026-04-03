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
    void diagnosticNotificationForwarded();
    void diagnosticNotificationDebugLevel();
    void processErrorEmitsSessionError();
    void stopSessionDuringLifecycle();
    void doubleStopSession();
    void requestReadDataWhenNotActive();
    void nonObjectResultEmitsSessionError();
    void errorDuringShutdownSuppressed();
    void awaitingConfigPausesBeforeConfigure();
    void stopSessionDuringAwaitingConfig();
};

#endif // TST_ADAPTERCLIENT_H
