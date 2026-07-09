
#ifndef TEST_GRAPHDATASERIES_H__
#define TEST_GRAPHDATASERIES_H__

#include <QObject>

class TestGraphDataSeries: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void addAppendsInOrder();
    void addInsertsSortedWhenOutOfOrder();
    void addAllowsDuplicateTimestamps();
    void sizeIsEmptyClear();
    void setSamplesReplacesExistingData();

    void findBeginOnEmptyReturnsConstEnd();
    void findEndOnEmptyReturnsConstEnd();
    void findBeginNotExpandedReturnsFirstAtOrAboveTimestamp();
    void findBeginExpandedIncludesSampleBelow();
    void findEndNotExpandedReturnsOnePastAtOrBelowTimestamp();
    void findEndExpandedIncludesSampleAbove();

    void iteratorSupportsRandomAccess();
    void timestampRangeValidAndInvalid();
    void parallelIterationMatchesExporterPattern();

private:

};

#endif /* TEST_GRAPHDATASERIES_H__ */
