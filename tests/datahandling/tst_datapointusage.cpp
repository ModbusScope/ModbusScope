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

//! Describe response of an adapter with protocol name \a protocolName that allows \a maxRegisters data points.
QJsonObject TestDataPointUsage::describeWithLimit(const QString& protocolName, int maxRegisters)
{
    QJsonObject capabilities;
    capabilities["maxRegisters"] = maxRegisters;

    QJsonObject describeResult;
    describeResult["protocolName"] = protocolName;
    describeResult["capabilities"] = capabilities;
    return describeResult;
}

//! Add \a count signals on the first device, each with its own data point, and return those data points.
QList<DataPoint> TestDataPointUsage::seedDataPoints(int count)
{
    QStringList exprList;
    for (int idx = 0; idx < count; idx++)
    {
        exprList.append(QString("${%1}").arg(40001 + idx));
    }
    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    return DataPointUsage::activeDataPoints(_pGraphDataModel);
}

void TestDataPointUsage::limitWarningsEmptyWithinLimit()
{
    DeviceListHelpers::seedDevice(_pSettingsModel, Device::cFirstDeviceId, QString(cModbusAdapterId));
    _pSettingsModel->updateAdapterFromDescribe(QString(cModbusAdapterId), describeWithLimit("Modbus", 3));

    QVERIFY(DataPointUsage::limitWarnings(seedDataPoints(2), _pSettingsModel).isEmpty());
}

void TestDataPointUsage::limitWarningsEmptyAtLimit()
{
    DeviceListHelpers::seedDevice(_pSettingsModel, Device::cFirstDeviceId, QString(cModbusAdapterId));
    _pSettingsModel->updateAdapterFromDescribe(QString(cModbusAdapterId), describeWithLimit("Modbus", 3));

    QVERIFY(DataPointUsage::limitWarnings(seedDataPoints(3), _pSettingsModel).isEmpty());
}

void TestDataPointUsage::limitWarningsReportsAdapterOverLimit()
{
    DeviceListHelpers::seedDevice(_pSettingsModel, Device::cFirstDeviceId, QString(cModbusAdapterId));
    _pSettingsModel->updateAdapterFromDescribe(QString(cModbusAdapterId), describeWithLimit("Modbus", 3));

    const QStringList warnings = DataPointUsage::limitWarnings(seedDataPoints(4), _pSettingsModel);

    QCOMPARE(warnings.size(), 1);
    QCOMPARE(warnings.first(), QString("Modbus allows at most 3 data point(s), but 4 are configured."));
}

/*!
 * \brief An adapter SettingsModel does not know cannot have reported a limit, and checking it must
 * not insert a phantom adapter entry.
 */
void TestDataPointUsage::limitWarningsIgnoresUnknownAdapter()
{
    DeviceListHelpers::seedDevice(_pSettingsModel, Device::cFirstDeviceId, "ghost");
    const QStringList adapterIdsBefore = _pSettingsModel->adapterIds();

    QVERIFY(DataPointUsage::limitWarnings(seedDataPoints(1), _pSettingsModel).isEmpty());
    QCOMPARE(_pSettingsModel->adapterIds(), adapterIdsBefore);
}

void TestDataPointUsage::limitWarningsUsesIdForAdapterWithoutName()
{
    DeviceListHelpers::seedDevice(_pSettingsModel, Device::cFirstDeviceId, QString(cModbusAdapterId));
    _pSettingsModel->updateAdapterFromDescribe(QString(cModbusAdapterId), describeWithLimit(QString(), 1));

    const QStringList warnings = DataPointUsage::limitWarnings(seedDataPoints(2), _pSettingsModel);

    QCOMPARE(warnings.size(), 1);
    QVERIFY(warnings.first().startsWith(QString("%1 allows at most 1").arg(cModbusAdapterId)));
}

QTEST_GUILESS_MAIN(TestDataPointUsage)
