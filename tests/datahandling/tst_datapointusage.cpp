#include "tst_datapointusage.h"

#include "../communication/communicationhelpers.h"
#include "../models/devicelisthelpers.h"
#include "ProtocolAdapter/adapterhub.h"
#include "datahandling/datapointusage.h"
#include "models/graphdatamodel.h"
#include "models/settingsmodel.h"

#include <QTest>

void TestDataPointUsage::init()
{
    _pSettingsModel = new SettingsModel;
    _pGraphDataModel = new GraphDataModel(_pSettingsModel);
}

void TestDataPointUsage::cleanup()
{
    delete _pGraphDataModel;
    delete _pSettingsModel;
}

void TestDataPointUsage::activeExpressionsSkipsInactive()
{
    auto exprList = QStringList() << "${40001}" << "${40002}" << "${40003}";
    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);
    _pGraphDataModel->setActive(GraphIdx(1), false);

    auto expExpressions = QStringList() << "${40001}" << "${40003}";

    QCOMPARE(DataPointUsage::activeExpressions(_pGraphDataModel), expExpressions);
}

void TestDataPointUsage::activeDataPointsSkipsInactive()
{
    auto exprList = QStringList() << "${40001}" << "${40002}";
    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);
    _pGraphDataModel->setActive(GraphIdx(1), false);

    auto expDataPoints = QList<DataPoint>() << DataPoint("${40001}", Device::cFirstDeviceId);

    QCOMPARE(DataPointUsage::activeDataPoints(_pGraphDataModel), expDataPoints);
}

/*!
 * \brief A data point used by several signals is read once, so it is counted once.
 */
void TestDataPointUsage::activeDataPointsDedupesSharedDataPoint()
{
    auto exprList = QStringList() << "${40001}" << "${40001} + 1";
    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto expDataPoints = QList<DataPoint>() << DataPoint("${40001}", Device::cFirstDeviceId);

    QCOMPARE(DataPointUsage::activeDataPoints(_pGraphDataModel), expDataPoints);
}

/*!
 * \brief A single signal referencing several data points needs each of them read.
 */
void TestDataPointUsage::activeDataPointsCountsEveryReferenceInExpression()
{
    auto exprList = QStringList() << "${40001} + ${40002}";
    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto expDataPoints = QList<DataPoint>() << DataPoint("${40001}", Device::cFirstDeviceId)
                                            << DataPoint("${40002}", Device::cFirstDeviceId);

    QCOMPARE(DataPointUsage::activeDataPoints(_pGraphDataModel), expDataPoints);
}

void TestDataPointUsage::countPerAdapterEmptyWithoutDataPoints()
{
    QVERIFY(DataPointUsage::countPerAdapter(QList<DataPoint>(), _pSettingsModel).isEmpty());
}

void TestDataPointUsage::countPerAdapterCountsSingleAdapter()
{
    DeviceListHelpers::seedDevice(_pSettingsModel, Device::cFirstDeviceId, QString(cModbusAdapterId));

    auto exprList = QStringList() << "${40001} + ${40002}" << "${40003}";
    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    const QMap<QString, int> counts =
      DataPointUsage::countPerAdapter(DataPointUsage::activeDataPoints(_pGraphDataModel), _pSettingsModel);

    QCOMPARE(counts.size(), 1);
    QCOMPARE(counts.value(QString(cModbusAdapterId)), 3);
}

/*!
 * \brief Every adapter enforces its own limit, so data points are counted per owning adapter.
 */
void TestDataPointUsage::countPerAdapterSplitsOverAdapters()
{
    const deviceId_t modbusDeviceId = Device::cFirstDeviceId;
    const deviceId_t dummyDeviceId = Device::cFirstDeviceId + 1;
    DeviceListHelpers::seedDevice(_pSettingsModel, modbusDeviceId, QString(cModbusAdapterId));
    DeviceListHelpers::seedDevice(_pSettingsModel, dummyDeviceId, "dummy");

    auto exprList = QStringList() << QString("${40001@%1}").arg(modbusDeviceId)
                                  << QString("${40002@%1}").arg(dummyDeviceId)
                                  << QString("${40003@%1}").arg(dummyDeviceId);
    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    const QMap<QString, int> counts =
      DataPointUsage::countPerAdapter(DataPointUsage::activeDataPoints(_pGraphDataModel), _pSettingsModel);

    QCOMPARE(counts.size(), 2);
    QCOMPARE(counts.value(QString(cModbusAdapterId)), 1);
    QCOMPARE(counts.value("dummy"), 2);
}

QTEST_GUILESS_MAIN(TestDataPointUsage)
