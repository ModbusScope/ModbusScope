#ifndef TST_ADAPTERPOLL_H
#define TST_ADAPTERPOLL_H

#include <QObject>

class TestAdapterPoll : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void startCommunicationWhenAdapterReady();
    void startCommunicationWhenAdapterNotReady();
    void doubleStartCommunicationWhileInitializing();
    void stopCommunicationClearsPendingState();
    void stopCommunicationAllowsNewWaitAfterRestart();
    void phantomAdapterDoesNotHangPoll();
    void mismatchedGroupResultsAreInvalidNotNoValue();
    void sessionErrorClearsForRestart();
    void sessionErrorEmitsCommunicationError();
    void sessionErrorWhileInactiveDoesNotEmitCommunicationError();
    void sessionErrorWhileWaitingForAdapterEmitsCommunicationError();
    void qualityChangeIsLoggedOnce();
    void qualityRecoveryIsLogged();
    void qualityFlagsAreLogged();
};

#endif // TST_ADAPTERPOLL_H
