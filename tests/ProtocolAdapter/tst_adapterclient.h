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

    void requestDataPointSchemaEmitsSignal();
    void requestDataPointSchemaInWrongStateIgnored();
    void describeDataPointInAwaitingConfig();
    void describeDataPointInActiveState();
    void describeDataPointInWrongStateIgnored();
    void validateDataPointValid();
    void validateDataPointInvalid();
    void validateDataPointInActiveState();
    void validateDataPointInWrongStateIgnored();

    void buildExpressionRequestAndResponse();
    void buildExpressionInActiveState();
    void buildExpressionInWrongStateIgnored();
    void buildExpressionOmitsDefaultDataType();
    void buildExpressionOmitsDefaultDeviceId();

    void expressionHelpRequestAndResponse();
    void expressionHelpInActiveState();
    void expressionHelpInWrongStateIgnored();
    void expressionHelpErrorIsNonFatal();

    void readDataErrorIsNonFatal();
    void readDataErrorInNonActiveStateIsIgnored();
};

#endif // TST_ADAPTERCLIENT_H
