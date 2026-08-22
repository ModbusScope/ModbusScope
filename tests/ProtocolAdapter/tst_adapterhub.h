#ifndef TST_ADAPTERHUB_H
#define TST_ADAPTERHUB_H

#include <QObject>

class TestAdapterHub : public QObject
{
    Q_OBJECT
private slots:
    void errorOnSingleAdapterEmitsErrorOnly();
    void errorOnLastPendingAdapterDoesNotEmitSessionStarted();
    void allAdaptersSucceedEmitsSessionStartedOnce();
    void requestReadDataSkipsInactiveAdapters();
    void stopSessionOnlyWaitsOnActiveAdapters();
    void stopSessionForceStopsMidHandshakeAdaptersWithoutWaiting();
    void stopSessionPurgesPendingStartForForceStoppedAdapters();
    void stopSessionWaitsForAllAdaptersWhenOneStopsSynchronously();
};

#endif // TST_ADAPTERHUB_H
