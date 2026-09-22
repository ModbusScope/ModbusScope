#include "tst_dummyadapterquality.h"

#include "ProtocolAdapter/adaptermanager.h"
#include "models/adapterdata.h"
#include "models/settingsmodel.h"
#include "util/result.h"

#include <QJsonObject>
#include <QSignalSpy>
#include <QTest>

namespace {
constexpr int cSessionTimeoutMs = 10000;
constexpr int cReadTimeoutMs = 5000;
constexpr char cAdapterId[] = "dummy";
} // namespace

void TestDummyAdapterQuality::init()
{
    _pSettingsModel = new SettingsModel;
    _pAdapterManager = new AdapterManager(
      QString::fromUtf8(cAdapterId), QString::fromUtf8(DUMMY_STANDALONE_ADAPTER_EXECUTABLE), _pSettingsModel, this);
}

void TestDummyAdapterQuality::cleanup()
{
    delete _pAdapterManager;
    _pAdapterManager = nullptr;
    delete _pSettingsModel;
    _pSettingsModel = nullptr;
}

/*!
 * \brief The describe response must echo this application's protocol version and declare the
 * quality flags the adapter may report.
 */
void TestDummyAdapterQuality::describeAdvertisesProtocolVersionAndQualityCapability()
{
    QSignalSpy spyReady(_pAdapterManager, &AdapterManager::adapterReady);
    _pAdapterManager->initAdapter();
    QVERIFY2(spyReady.wait(cSessionTimeoutMs), "adapterReady not emitted");

    const AdapterData* pAdapterData = _pSettingsModel->adapterData(QString::fromUtf8(cAdapterId));
    QVERIFY(pAdapterData != nullptr);
    QCOMPARE(pAdapterData->protocolVersion(), cProtocolVersion);
    QVERIFY(pAdapterData->reportsQuality());
    QVERIFY(pAdapterData->qualityFlags().contains(QStringLiteral("substituted")));
    QVERIFY(pAdapterData->qualityFlags().contains(QStringLiteral("oldData")));
}

/*!
 * \brief One read of addresses 0..4 must yield each state and flag combination the dummy adapter
 * documents for address % 5: Good, Degraded+Substituted, Degraded+Overflow|Blocked,
 * Invalid+OldData and NoValue.
 */
void TestDummyAdapterQuality::readDataCarriesAllStatesAndFlags()
{
    QSignalSpy spyReady(_pAdapterManager, &AdapterManager::adapterReady);
    _pAdapterManager->initAdapter();
    QVERIFY2(spyReady.wait(cSessionTimeoutMs), "adapterReady not emitted");

    _pSettingsModel->setAdapterCurrentConfig(QString::fromUtf8(cAdapterId), QJsonObject());

    QSignalSpy spyStarted(_pAdapterManager, &AdapterManager::sessionStarted);
    QSignalSpy spyError(_pAdapterManager, &AdapterManager::sessionError);

    _pAdapterManager->startSession(QStringList{ QStringLiteral("${0}"), QStringLiteral("${1}"), QStringLiteral("${2}"),
                                                QStringLiteral("${3}"), QStringLiteral("${4}") });
    QVERIFY2(spyStarted.wait(cSessionTimeoutMs), "sessionStarted not emitted");
    QCOMPARE(spyError.count(), 0);

    QSignalSpy spyData(_pAdapterManager, &AdapterManager::readDataResult);
    _pAdapterManager->requestReadData();
    QVERIFY2(spyData.wait(cReadTimeoutMs), "readDataResult not emitted");

    const auto results = spyData.at(0).at(0).value<ResultDoubleList>();
    QCOMPARE(results.size(), 5);

    QCOMPARE(results[0].state(), DataQuality::State::Good);
    QCOMPARE(results[0].flags(), DataQuality::Flags(DataQuality::Flag::NoFlags));

    QCOMPARE(results[1].state(), DataQuality::State::Degraded);
    QCOMPARE(results[1].value(), 1.0);
    QCOMPARE(results[1].flags(), DataQuality::Flags(DataQuality::Flag::Substituted));

    QCOMPARE(results[2].state(), DataQuality::State::Degraded);
    QCOMPARE(results[2].value(), 2.0);
    QCOMPARE(results[2].flags(), DataQuality::Flags(DataQuality::Flag::Overflow | DataQuality::Flag::Blocked));

    QCOMPARE(results[3].state(), DataQuality::State::Invalid);
    QCOMPARE(results[3].flags(), DataQuality::Flags(DataQuality::Flag::OldData));

    QCOMPARE(results[4].state(), DataQuality::State::NoValue);
}

QTEST_GUILESS_MAIN(TestDummyAdapterQuality)
