#include "tst_dummyadapterstarterror.h"

#include "ProtocolAdapter/adaptermanager.h"
#include "models/settingsmodel.h"
#include "util/result.h"

#include <QJsonObject>
#include <QSignalSpy>
#include <QTest>

namespace {
constexpr int cSessionTimeoutMs = 10000;
constexpr int cReadTimeoutMs = 5000;
} // namespace

void TestDummyAdapterStartError::init()
{
    _pSettingsModel = new SettingsModel;
    _pAdapterManager = new AdapterManager(
      QStringLiteral("dummy"), QString::fromUtf8(DUMMY_STANDALONE_ADAPTER_EXECUTABLE), _pSettingsModel, this);
}

void TestDummyAdapterStartError::cleanup()
{
    delete _pAdapterManager;
    _pAdapterManager = nullptr;
    delete _pSettingsModel;
    _pSettingsModel = nullptr;
}

/*!
 * \brief Reproduces the original bug report against the real "dummy" adapter binary.
 *
 * "${h0}" is rejected by the adapter's RegisterAddress::parse, which previously caused
 * AdapterClient to force-kill the subprocess and emit a fatal sessionError — silently halting
 * polling for every adapter. The session must now be reported as started (degraded) instead,
 * readData must keep returning invalid results without contacting the process, and the process
 * itself must remain alive and usable for a subsequent stop/reconfigure/restart with a valid
 * expression.
 */
void TestDummyAdapterStartError::startWithInvalidExpressionKeepsAdapterAliveAndPollable()
{
    QSignalSpy spyReady(_pAdapterManager, &AdapterManager::adapterReady);
    _pAdapterManager->initAdapter();
    QVERIFY2(spyReady.wait(cSessionTimeoutMs), "adapterReady not emitted");

    _pSettingsModel->setAdapterCurrentConfig(QStringLiteral("dummy"), QJsonObject());

    QSignalSpy spyStarted(_pAdapterManager, &AdapterManager::sessionStarted);
    QSignalSpy spyError(_pAdapterManager, &AdapterManager::sessionError);

    _pAdapterManager->startSession(QStringList{ QStringLiteral("${h0}") });

    QVERIFY2(spyStarted.wait(cSessionTimeoutMs), "sessionStarted not emitted after a rejected adapter.start");
    QCOMPARE(spyError.count(), 0);

    QSignalSpy spyData(_pAdapterManager, &AdapterManager::readDataResult);
    _pAdapterManager->requestReadData();
    /* A degraded session answers locally and synchronously (no adapter round-trip), so the signal
       may already have fired before wait() starts watching for a new one — check count() first. */
    if (spyData.isEmpty())
    {
        QVERIFY2(spyData.wait(cReadTimeoutMs), "readDataResult not emitted for a degraded session");
    }
    const auto results = spyData.at(0).at(0).value<ResultDoubleList>();
    QCOMPARE(results.size(), 1);
    QVERIFY2(!results[0].isValid(), "Expected an invalid result for the never-configured register");

    /* Prove the real subprocess is still alive and responsive — not just that the client-side
       state machine thinks so — by driving a full stop/reconfigure/restart cycle against it with
       a valid expression. */
    QSignalSpy spyStopped(_pAdapterManager, &AdapterManager::sessionStopped);
    _pAdapterManager->stopSession();
    QVERIFY2(spyStopped.wait(cSessionTimeoutMs), "sessionStopped not emitted after stopSession");

    QSignalSpy spyRestarted(_pAdapterManager, &AdapterManager::sessionStarted);
    _pAdapterManager->startSession(QStringList{ QStringLiteral("${0}") });
    QVERIFY2(spyRestarted.wait(cSessionTimeoutMs), "sessionStarted not emitted on retry with a valid expression");

    QSignalSpy spyRetryData(_pAdapterManager, &AdapterManager::readDataResult);
    _pAdapterManager->requestReadData();
    QVERIFY2(spyRetryData.wait(cReadTimeoutMs), "readDataResult not emitted after successful retry");
    const auto retryResults = spyRetryData.at(0).at(0).value<ResultDoubleList>();
    QCOMPARE(retryResults.size(), 1);
    QVERIFY2(retryResults[0].isValid(), "Expected a real SUCCESS result from the still-running adapter process");
}

QTEST_GUILESS_MAIN(TestDummyAdapterStartError)
