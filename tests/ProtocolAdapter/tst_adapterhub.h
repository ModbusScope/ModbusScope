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
};

#endif // TST_ADAPTERHUB_H
