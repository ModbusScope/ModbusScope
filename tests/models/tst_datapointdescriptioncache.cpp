#include "tst_datapointdescriptioncache.h"

#include "../dialogs/mockadapterhub.h"
#include "../mocks/mockdatapointmanager.h"
#include "ProtocolAdapter/datapointservice.h"
#include "devicelisthelpers.h"
#include "models/datapointdescriptioncache.h"

#include <QSignalSpy>
#include <QTest>

using State = DataPointDescription::State;

namespace {

constexpr int cShortRetryMs = 50;

QJsonObject validResult(const QString& description)
{
    return QJsonObject{ { "valid", true },
                        { "description", description },
                        { "fields", QJsonObject{ { "address", 1 } } } };
}

} // namespace

void TestDataPointDescriptionCache::init()
{
    _pSettingsModel = new SettingsModel();
    DeviceListHelpers::clearDevices(_pSettingsModel);
    DeviceListHelpers::seedDevice(_pSettingsModel, Device::cFirstDeviceId, QStringLiteral("modbus"));

    _pManager = new MockDataPointManager(QStringLiteral("modbus"));
    _pMockHub = new MockAdapterHub();
    _pMockHub->addManager(QStringLiteral("modbus"), _pManager);

    _pService = new DataPointService(_pSettingsModel, _pMockHub);
    _pCache = new DataPointDescriptionCache(_pService, nullptr, cShortRetryMs);
}

void TestDataPointDescriptionCache::cleanup()
{
    delete _pCache;
    _pCache = nullptr;
    delete _pService;
    _pService = nullptr;
    delete _pMockHub;
    _pMockHub = nullptr;
    delete _pManager;
    _pManager = nullptr;
    delete _pSettingsModel;
    _pSettingsModel = nullptr;
}

void TestDataPointDescriptionCache::missSendsRequest()
{
    const DataPointDescription description = _pCache->lookup(reference(QStringLiteral("${40001}")));

    QCOMPARE(description.state, State::Pending);
    QCOMPARE(_pManager->describeCalls, QStringList{ QStringLiteral("${40001}") });
}

void TestDataPointDescriptionCache::repeatedLookupWhilePendingSendsOnce()
{
    _pCache->lookup(reference(QStringLiteral("${40001}")));
    _pCache->lookup(reference(QStringLiteral("${40001}")));

    QCOMPARE(_pManager->describeCalls.size(), 1);
}

void TestDataPointDescriptionCache::validResultIsCached()
{
    QSignalSpy spy(_pCache, &DataPointDescriptionCache::descriptionChanged);
    _pCache->lookup(reference(QStringLiteral("${40001}")));

    emit _pManager->describeDataPointResult(1, validResult(QStringLiteral("Holding register 0")));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("${40001}"));

    const DataPointDescription description = _pCache->lookup(reference(QStringLiteral("${40001}")));
    QCOMPARE(description.state, State::Valid);
    QCOMPARE(description.description, QStringLiteral("Holding register 0"));
    QCOMPARE(description.fields.value("address").toInt(), 1);
    QCOMPARE(_pManager->describeCalls.size(), 1);
}

void TestDataPointDescriptionCache::invalidResultNeverRetried()
{
    _pCache->lookup(reference(QStringLiteral("${abc}")));
    emit _pManager->describeDataPointResult(
      1, QJsonObject{ { "valid", false }, { "error", QStringLiteral("Unknown address") } });

    emit _pMockHub->adapterReady();
    const DataPointDescription description = _pCache->lookup(reference(QStringLiteral("${abc}")));

    QCOMPARE(description.state, State::Invalid);
    QCOMPARE(description.error, QStringLiteral("Unknown address"));
    QCOMPARE(_pManager->describeCalls.size(), 1);
}

void TestDataPointDescriptionCache::failedRetriedOnlyAfterInterval()
{
    _pCache->lookup(reference(QStringLiteral("${40001}")));
    emit _pManager->dataPointRequestFailed(1, QStringLiteral("Request cancelled"));
    QTest::qWait(0); /* the service reports failures on the next event loop turn */

    const DataPointDescription failed = _pCache->lookup(reference(QStringLiteral("${40001}")));
    QCOMPARE(failed.state, State::Failed);
    QCOMPARE(failed.error, QStringLiteral("Request cancelled"));
    QCOMPARE(_pManager->describeCalls.size(), 1);

    QTest::qWait(cShortRetryMs + 10);

    QCOMPARE(_pCache->lookup(reference(QStringLiteral("${40001}"))).state, State::Pending);
    _pCache->lookup(reference(QStringLiteral("${40001}")));
    QCOMPARE(_pManager->describeCalls.size(), 2);
}

void TestDataPointDescriptionCache::unavailableWithoutRequest()
{
    _pManager->mockReady = false;

    const DataPointDescription description = _pCache->lookup(reference(QStringLiteral("${40001}")));

    QCOMPARE(description.state, State::Unavailable);
    QVERIFY(_pManager->describeCalls.isEmpty());
}

void TestDataPointDescriptionCache::unavailableRetriedAfterAdaptersChanged()
{
    _pManager->mockReady = false;
    _pCache->lookup(reference(QStringLiteral("${40001}")));
    QSignalSpy spy(_pCache, &DataPointDescriptionCache::descriptionChanged);

    _pManager->mockReady = true;
    emit _pMockHub->adapterReady();

    QCOMPARE(spy.count(), 1);
    QCOMPARE(_pCache->lookup(reference(QStringLiteral("${40001}"))).state, State::Pending);
    QCOMPARE(_pManager->describeCalls.size(), 1);
}

void TestDataPointDescriptionCache::failedRetriedAfterAdaptersChanged()
{
    _pCache->lookup(reference(QStringLiteral("${40001}")));
    emit _pManager->dataPointRequestFailed(1, QStringLiteral("Request cancelled"));
    QTest::qWait(0);

    emit _pMockHub->sessionStarted();

    QCOMPARE(_pCache->lookup(reference(QStringLiteral("${40001}"))).state, State::Pending);
    QCOMPARE(_pManager->describeCalls.size(), 2);
}

void TestDataPointDescriptionCache::pendingReplyAppliedAfterAdaptersChanged()
{
    _pCache->lookup(reference(QStringLiteral("${40001}")));
    emit _pMockHub->adapterReady();

    emit _pManager->describeDataPointResult(1, validResult(QStringLiteral("Holding register 0")));

    QCOMPARE(_pCache->lookup(reference(QStringLiteral("${40001}"))).state, State::Valid);
    QCOMPARE(_pManager->describeCalls.size(), 1);
}

void TestDataPointDescriptionCache::otherTicketsIgnored()
{
    QSignalSpy spy(_pCache, &DataPointDescriptionCache::descriptionChanged);
    const quint64 ticket = _pService->describeDataPoint(Device::cFirstDeviceId, QStringLiteral("${40001}"));
    QVERIFY(ticket != 0);

    emit _pManager->describeDataPointResult(1, validResult(QStringLiteral("Holding register 0")));

    QCOMPARE(spy.count(), 0);
}

ExpressionReferences::Reference TestDataPointDescriptionCache::reference(const QString& text)
{
    return ExpressionReferences::find(text).value(0);
}

QTEST_GUILESS_MAIN(TestDataPointDescriptionCache)
