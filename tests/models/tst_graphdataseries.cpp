
#include "tst_graphdataseries.h"

#include "models/graphdataseries.h"

#include <QTest>

void TestGraphDataSeries::init()
{

}

void TestGraphDataSeries::cleanup()
{

}

void TestGraphDataSeries::addAppendsInOrder()
{
    GraphDataSeries series;

    series.add(1, 10);
    series.add(GraphSample{2, 20});
    series.add(3, 30);

    QCOMPARE(series.size(), 3);
    QCOMPARE(series.constBegin()->timestamp, 1.0);
    QCOMPARE(series.constBegin()->value, 10.0);
    QCOMPARE((series.constEnd() - 1)->timestamp, 3.0);
    QCOMPARE((series.constEnd() - 1)->value, 30.0);
}

void TestGraphDataSeries::addInsertsSortedWhenOutOfOrder()
{
    GraphDataSeries series;

    series.add(1, 10);
    series.add(3, 30);
    series.add(2, 20);

    QCOMPARE(series.size(), 3);
    QCOMPARE((series.constBegin() + 0)->timestamp, 1.0);
    QCOMPARE((series.constBegin() + 1)->timestamp, 2.0);
    QCOMPARE((series.constBegin() + 1)->value, 20.0);
    QCOMPARE((series.constBegin() + 2)->timestamp, 3.0);
}

void TestGraphDataSeries::addAllowsDuplicateTimestamps()
{
    GraphDataSeries series;

    series.add(1, 10);
    series.add(1, 11);

    QCOMPARE(series.size(), 2);
    QCOMPARE((series.constBegin() + 0)->timestamp, 1.0);
    QCOMPARE((series.constBegin() + 1)->timestamp, 1.0);
}

void TestGraphDataSeries::sizeIsEmptyClear()
{
    GraphDataSeries series;

    QVERIFY(series.isEmpty());
    QCOMPARE(series.size(), 0);

    series.add(1, 10);

    QVERIFY(!series.isEmpty());
    QCOMPARE(series.size(), 1);

    series.clear();

    QVERIFY(series.isEmpty());
    QCOMPARE(series.size(), 0);
}

void TestGraphDataSeries::setSamplesReplacesExistingData()
{
    GraphDataSeries series;

    series.add(100, 1);

    series.setSamples(QList<double>() << 1 << 2 << 3, QList<double>() << 10 << 20 << 30);

    QCOMPARE(series.size(), 3);
    QCOMPARE(series.constBegin()->timestamp, 1.0);
    QCOMPARE(series.constBegin()->value, 10.0);
    QCOMPARE((series.constEnd() - 1)->timestamp, 3.0);
    QCOMPARE((series.constEnd() - 1)->value, 30.0);
}

void TestGraphDataSeries::findBeginOnEmptyReturnsConstEnd()
{
    GraphDataSeries series;

    QVERIFY(series.findBegin(1, false) == series.constEnd());
    QVERIFY(series.findBegin(1, true) == series.constEnd());
}

void TestGraphDataSeries::findEndOnEmptyReturnsConstEnd()
{
    GraphDataSeries series;

    QVERIFY(series.findEnd(1, false) == series.constEnd());
    QVERIFY(series.findEnd(1, true) == series.constEnd());
}

void TestGraphDataSeries::findBeginNotExpandedReturnsFirstAtOrAboveTimestamp()
{
    GraphDataSeries series;
    series.add(1, 10);
    series.add(3, 30);
    series.add(5, 50);

    /* Exact hit */
    QVERIFY(series.findBegin(3, false) == series.constBegin() + 1);

    /* Between timestamps */
    QVERIFY(series.findBegin(4, false) == series.constBegin() + 2);

    /* Below all samples */
    QVERIFY(series.findBegin(0, false) == series.constBegin());

    /* Above all samples */
    QVERIFY(series.findBegin(6, false) == series.constEnd());
}

void TestGraphDataSeries::findBeginExpandedIncludesSampleBelow()
{
    GraphDataSeries series;
    series.add(1, 10);
    series.add(3, 30);
    series.add(5, 50);

    /* Between timestamps: one sample earlier is included */
    QVERIFY(series.findBegin(4, true) == series.constBegin() + 1);

    /* Exact hit on first sample: clamped at constBegin */
    QVERIFY(series.findBegin(1, true) == series.constBegin());

    /* Below all samples: clamped at constBegin */
    QVERIFY(series.findBegin(0, true) == series.constBegin());

    /* Above all samples: last sample is included */
    QVERIFY(series.findBegin(6, true) == series.constEnd() - 1);
}

void TestGraphDataSeries::findEndNotExpandedReturnsOnePastAtOrBelowTimestamp()
{
    GraphDataSeries series;
    series.add(1, 10);
    series.add(3, 30);
    series.add(5, 50);

    /* Exact hit: one past the matching sample */
    QVERIFY(series.findEnd(3, false) == series.constBegin() + 2);

    /* Between timestamps */
    QVERIFY(series.findEnd(4, false) == series.constBegin() + 2);

    /* Below all samples */
    QVERIFY(series.findEnd(0, false) == series.constBegin());

    /* Above all samples */
    QVERIFY(series.findEnd(6, false) == series.constEnd());
}

void TestGraphDataSeries::findEndExpandedIncludesSampleAbove()
{
    GraphDataSeries series;
    series.add(1, 10);
    series.add(3, 30);
    series.add(5, 50);

    /* Between timestamps: one sample further is included */
    QVERIFY(series.findEnd(2, true) == series.constBegin() + 2);

    /* Above all samples: clamped at constEnd */
    QVERIFY(series.findEnd(6, true) == series.constEnd());

    /* Exact hit on last sample: clamped at constEnd */
    QVERIFY(series.findEnd(5, true) == series.constEnd());
}

void TestGraphDataSeries::iteratorSupportsRandomAccess()
{
    GraphDataSeries series;
    series.add(1, 10);
    series.add(2, 20);
    series.add(3, 30);

    QCOMPARE(series.constEnd() - series.constBegin(), 3);

    GraphDataSeries::const_iterator it = series.constBegin();
    QCOMPARE((it + 1)->timestamp, 2.0);

    QCOMPARE((series.constEnd() - 2)->timestamp, 2.0);

    it++;
    QCOMPARE(it->value, 20.0);
}

void TestGraphDataSeries::timestampRangeValidAndInvalid()
{
    GraphDataSeries series;
    bool bValid = true;

    (void)series.timestampRange(bValid);
    QVERIFY(!bValid);

    series.add(1, 10);
    series.add(5, 50);

    GraphDataSeries::TimestampRange range = series.timestampRange(bValid);
    QVERIFY(bValid);
    QCOMPARE(range.min, 1.0);
    QCOMPARE(range.max, 5.0);
}

void TestGraphDataSeries::parallelIterationMatchesExporterPattern()
{
    /* Mimics the lockstep iteration over multiple graphs in DataFileExporter::exportDataFile */
    GraphDataSeries series1;
    GraphDataSeries series2;

    for (int idx = 0; idx < 3; idx++)
    {
        series1.add(idx, idx * 10);
        series2.add(idx, idx * 100);
    }

    QList<GraphDataSeries::const_iterator> iterators;
    iterators.append(series1.constBegin());
    iterators.append(series2.constBegin());

    for (qsizetype row = 0; row < series1.size(); row++)
    {
        QCOMPARE(iterators[0]->timestamp, static_cast<double>(row));
        QCOMPARE(iterators[0]->value, row * 10.0);
        QCOMPARE(iterators[1]->value, row * 100.0);

        iterators[0]++;
        iterators[1]++;
    }

    QVERIFY(iterators[0] == series1.constEnd());
    QVERIFY(iterators[1] == series2.constEnd());
}

QTEST_GUILESS_MAIN(TestGraphDataSeries)
