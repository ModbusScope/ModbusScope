
#ifndef TEST_RECENTFILEMODULE_H__
#define TEST_RECENTFILEMODULE_H__

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

#endif /* TEST_RECENTFILEMODULE_H__ */
