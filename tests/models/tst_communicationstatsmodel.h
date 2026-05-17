
#ifndef TST_COMMUNICATIONSTATSMODEL_H
#define TST_COMMUNICATIONSTATSMODEL_H

#include <QObject>

class TestCommunicationStatsModel : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void defaultStateIsAllZero();
    void setStatsEmitsSignalOnChange();
    void setStatsDoesNotEmitSignalWhenUnchanged();
    void setStartTimeStores();
    void setEndTimeStores();
    void setMedianPollTimeStores();
    void setStartTimeEmitsSignal();
    void setEndTimeEmitsSignal();
    void setMedianPollTimeEmitsSignal();
    void runTimeReflectsStartTime();
    void runTimeIsZeroWhenNotStarted();
};

#endif /* TST_COMMUNICATIONSTATSMODEL_H */
