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
    void startCommunicationWhenAdapterIdle();
    void startCommunicationWhenAdapterInitializing();
    void doubleStartCommunicationWhileInitializing();
    void stopCommunicationClearsPendingState();
    void stopCommunicationAllowsNewWaitAfterRestart();
};

#endif // TST_ADAPTERPOLL_H
