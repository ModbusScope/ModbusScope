
#include "tst_communicationstats.h"

#include <QtTest/QtTest>
#include <qforeach.h>

#include "communicationstats.h"

#include "graphdatamodel.h"
#include "settingsmodel.h"

void TestCommunicationStats::init()
{
    _pSettingsModel = new SettingsModel;
    _pGraphDataModel = new GraphDataModel(_pSettingsModel);
    _pGraphDataModel->add();

    _pCommunicationStats = new CommunicationStats(_pGraphDataModel);
}

void TestCommunicationStats::cleanup()
{
    delete _pCommunicationStats;
    delete _pGraphDataModel;
    delete _pSettingsModel;
}

void TestCommunicationStats::noGraph()
{
    _pGraphDataModel->clear();

    _pCommunicationStats->updateTimingInfo();

    QCOMPARE(_pGraphDataModel->medianPollTime(), 0);
}

void TestCommunicationStats::notEnoughDataCount()
{
    QVector<double> times = QVector<double>() << 1;
    setPollData(times);
    _pCommunicationStats->updateTimingInfo();

    QCOMPARE(_pGraphDataModel->medianPollTime(), 0);
}

void TestCommunicationStats::justEnoughDataCount()
{
    QVector<double> times = QVector<double>() << 1 << 2;
    setPollData(times);

    _pCommunicationStats->updateTimingInfo();

    QCOMPARE(_pGraphDataModel->medianPollTime(), 1);
}

void TestCommunicationStats::dataCountIsEven()
{
    QVector<double> times = QVector<double>() << 1 << 1 << 2 << 3 << 5 << 8;
    setPollData(times);

    _pCommunicationStats->updateTimingInfo();

    QCOMPARE(_pGraphDataModel->medianPollTime(), 1);
}

void TestCommunicationStats::dataCountIsNotEven()
{
    QVector<double> times = QVector<double>() << 1 << 1 << 2 << 3 << 5 << 7 << 12;
    setPollData(times);

    _pCommunicationStats->updateTimingInfo();

    QCOMPARE(_pGraphDataModel->medianPollTime(), 2);
}

void TestCommunicationStats::pollTimeDiffIsGettingSmaller()
{
    /* 100 ms sample time with variation of reverse fibonacci (12 to 1) */
    QVector<double> times = QVector<double>() << 0 << 112 << 219 << 324 << 427 << 529 << 630;
    setPollData(times);

    _pCommunicationStats->updateTimingInfo();

    QCOMPARE(_pGraphDataModel->medianPollTime(), 105);
}

void TestCommunicationStats::onlyLastXSamples()
{
    auto _pCommunicationStatsLimited = new CommunicationStats(_pGraphDataModel, 3);

    QVector<double> times = QVector<double>() << 1 << 1 << 2 << 3 << 5 << 7 << 12;
    setPollData(times);

    _pCommunicationStats->updateTimingInfo();
    QCOMPARE(_pGraphDataModel->medianPollTime(), 2);

    _pCommunicationStatsLimited->updateTimingInfo();
    QCOMPARE(_pGraphDataModel->medianPollTime(), 5);

    delete _pCommunicationStatsLimited;
}

void TestCommunicationStats::setPollData(QVector<double> times)
{
    QSharedPointer<QCPGraphDataContainer> dataMap = _pGraphDataModel->dataMap(0);

    QVector<QCPGraphData> data;

    foreach (double time, times)
    {
        data.append(QCPGraphData(time, 0));
    }

    dataMap->add(data);
}


QTEST_GUILESS_MAIN(TestCommunicationStats)
