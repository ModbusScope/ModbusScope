
#include "tst_communicationstatsmodel.h"

#include "models/communicationstatsmodel.h"

#include <QDateTime>
#include <QSignalSpy>
#include <QTest>

void TestCommunicationStatsModel::init()
{
}

void TestCommunicationStatsModel::cleanup()
{
}

void TestCommunicationStatsModel::defaultStateIsAllZero()
{
    CommunicationStatsModel model;

    QCOMPARE(model.startTime(), 0);
    QCOMPARE(model.endTime(), 0);
    QCOMPARE(model.successCount(), 0u);
    QCOMPARE(model.errorCount(), 0u);
    QCOMPARE(model.medianPollTime(), 0u);
}

void TestCommunicationStatsModel::setStatsEmitsSignalOnChange()
{
    CommunicationStatsModel model;
    QSignalSpy spy(&model, &CommunicationStatsModel::statsChanged);

    model.setStats(10, 2);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(model.successCount(), 10u);
    QCOMPARE(model.errorCount(), 2u);
}

void TestCommunicationStatsModel::setStatsDoesNotEmitSignalWhenUnchanged()
{
    CommunicationStatsModel model;
    model.setStats(10, 2);

    QSignalSpy spy(&model, &CommunicationStatsModel::statsChanged);
    model.setStats(10, 2);

    QCOMPARE(spy.count(), 0);
}

void TestCommunicationStatsModel::setStartTimeStores()
{
    CommunicationStatsModel model;
    model.setStartTime(12345);

    QCOMPARE(model.startTime(), 12345);
}

void TestCommunicationStatsModel::setEndTimeStores()
{
    CommunicationStatsModel model;
    model.setEndTime(99999);

    QCOMPARE(model.endTime(), 99999);
}

void TestCommunicationStatsModel::setMedianPollTimeStores()
{
    CommunicationStatsModel model;
    model.setMedianPollTime(500);

    QCOMPARE(model.medianPollTime(), 500u);
}

void TestCommunicationStatsModel::runTimeReflectsStartTime()
{
    CommunicationStatsModel model;
    const qint64 before = QDateTime::currentMSecsSinceEpoch();
    model.setStartTime(before);
    const qint64 runTime = model.runTime();

    QVERIFY(runTime >= 0);
    QVERIFY(runTime < 5000);
}

QTEST_GUILESS_MAIN(TestCommunicationStatsModel)
