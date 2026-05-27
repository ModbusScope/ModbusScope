
#ifndef TST_RECENTFILEMODULE_H
#define TST_RECENTFILEMODULE_H

#include <QObject>

class TestRecentFileModule : public QObject
{
    Q_OBJECT

private slots:

    void init();
    void cleanup();

    void initiallyEmpty();
    void updateAddsFile();
    void updateMovesExistingToFront();
    void updateRespectsCap();
    void clearRecentProjectFiles();
    void persistsAcrossInstances();
    void signalEmittedOnUpdate();
    void signalEmittedOnClear();

private:
};

#endif /* TST_RECENTFILEMODULE_H */
