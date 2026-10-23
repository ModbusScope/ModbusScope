
#ifndef TEST_DATAPOINTUSAGE_H__
#define TEST_DATAPOINTUSAGE_H__

#include "communication/datapoint.h"

#include <QJsonObject>
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

    void limitWarningsEmptyWithinLimit();
    void limitWarningsEmptyAtLimit();
    void limitWarningsReportsAdapterOverLimit();
    void limitWarningsIgnoresUnknownAdapter();
    void limitWarningsUsesIdForAdapterWithoutName();

private:
    static QJsonObject describeWithLimit(const QString& protocolName, int maxRegisters);
    QList<DataPoint> seedDataPoints(int count);

    SettingsModel* _pSettingsModel;
    GraphDataModel* _pGraphDataModel;
};

#endif /* TEST_DATAPOINTUSAGE_H__ */
