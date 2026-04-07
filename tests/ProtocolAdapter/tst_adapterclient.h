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
    void diagnosticMalformedParams();
    void processErrorEmitsSessionError();
    void stopSessionDuringLifecycle();
    void doubleStopSession();
    void requestReadDataWhenNotActive();
    void nonObjectResultEmitsSessionError();
    void errorDuringShutdownSuppressed();
    void shutdownNoAckTimesOutToSessionStopped();
    void awaitingConfigPausesBeforeConfigure();
    void stopSessionDuringAwaitingConfig();
    void shutdownAckEmitsSessionStoppedAfterProcessExit();
    void processErrorDuringStoppingNoSessionError();
    void processErrorDuringStoppingThenProcessFinished();

    void requestRegisterSchemaEmitsSignal();
    void requestRegisterSchemaInWrongStateIgnored();
    void describeRegisterInAwaitingConfig();
    void describeRegisterInActiveState();
    void describeRegisterInWrongStateIgnored();
    void validateRegisterValid();
    void validateRegisterInvalid();
    void validateRegisterInActiveState();
    void validateRegisterInWrongStateIgnored();

    void buildExpressionRequestAndResponse();
    void buildExpressionInActiveState();
    void buildExpressionInWrongStateIgnored();
    void buildExpressionOmitsDefaultDataType();
    void buildExpressionOmitsDefaultDeviceId();
};

#endif // TST_ADAPTERCLIENT_H
