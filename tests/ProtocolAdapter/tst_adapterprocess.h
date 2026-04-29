#ifndef TST_ADAPTERPROCESS_H
#define TST_ADAPTERPROCESS_H

#include <QObject>

class TestAdapterProcess : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void notRunningByDefault();
    void startFailsWithBadPath();
    void sendRequestEmitsResponseReceived();
    void processFinishedEmittedOnStop();
};

#endif // TST_ADAPTERPROCESS_H
