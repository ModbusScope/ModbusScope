
#ifndef TEST_DATAPOINTUSAGE_H__
#define TEST_DATAPOINTUSAGE_H__

#include <QObject>

/* Forward declaration */
class GraphDataModel;
class SettingsModel;

class TestDataPointUsage : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void activeExpressionsSkipsInactive();

    void activeDataPointsSkipsInactive();
    void activeDataPointsDedupesSharedDataPoint();
    void activeDataPointsCountsEveryReferenceInExpression();

    void countPerAdapterEmptyWithoutDataPoints();
    void countPerAdapterCountsSingleAdapter();
    void countPerAdapterSplitsOverAdapters();

private:
    SettingsModel* _pSettingsModel;
    GraphDataModel* _pGraphDataModel;
};

#endif /* TEST_DATAPOINTUSAGE_H__ */
