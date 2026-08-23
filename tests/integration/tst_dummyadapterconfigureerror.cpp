#include "tst_dummyadapterconfigureerror.h"

#include "ProtocolAdapter/adaptermanager.h"
#include "models/adapterdata.h"
#include "models/settingsmodel.h"
#include "util/result.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTest>

namespace {
constexpr int cSessionTimeoutMs = 10000;
constexpr int cReadTimeoutMs = 5000;
constexpr char cAdapterId[] = "dummy";

QJsonObject connectionObject(int id)
{
    QJsonObject connection;
    connection["id"] = id;
    connection["name"] = QStringLiteral("Connection %1").arg(id);
    return connection;
}

QJsonObject deviceObject(int id, int connectionId)
{
    QJsonObject device;
    device["id"] = id;
    device["connectionId"] = connectionId;
    return device;
}

/*!
 * \brief Rebuild a describe object for \a pAdapterData, with the client-side device limit removed.
 *
 * The real dummy adapter binary always enforces its own device limit (1) at adapter.configure
 * regardless of what the client believes; removing the schema's devices.maxItems and
 * capabilities.maxDevices disables AdapterData::configForWire()'s local truncation (see
 * AdapterData::maxDevices()), so an over-limit config actually reaches the real subprocess —
 * reproducing the scenario (as seen with the real Modbus adapter) where the client's belief
 * about the limit doesn't match what the adapter actually enforces.
 */
QJsonObject describeWithoutDeviceLimit(const AdapterData* pAdapterData)
{
    QJsonObject schema = pAdapterData->schema();
    QJsonObject properties = schema["properties"].toObject();
    QJsonObject devicesSchema = properties["devices"].toObject();
    devicesSchema.remove("maxItems");
    properties["devices"] = devicesSchema;
    schema["properties"] = properties;

    QJsonObject capabilities = pAdapterData->capabilities();
    capabilities.remove("maxDevices");

    QJsonObject describe;
    describe["name"] = pAdapterData->name();
    describe["version"] = pAdapterData->version();
    describe["configVersion"] = pAdapterData->configVersion();
    describe["schema"] = schema;
    describe["defaults"] = pAdapterData->defaults();
    describe["capabilities"] = capabilities;
    describe["license"] = pAdapterData->license();
    return describe;
}

} // namespace

void TestDummyAdapterConfigureError::init()
{
    _pSettingsModel = new SettingsModel;
    _pAdapterManager = new AdapterManager(
      QString::fromUtf8(cAdapterId), QString::fromUtf8(DUMMY_STANDALONE_ADAPTER_EXECUTABLE), _pSettingsModel, this);
}

void TestDummyAdapterConfigureError::cleanup()
{
    delete _pAdapterManager;
    _pAdapterManager = nullptr;
    delete _pSettingsModel;
    _pSettingsModel = nullptr;
}

/*!
 * \brief Reproduces the reported bug against the real "dummy" adapter binary: a device count that
 * exceeds the adapter's actual device limit is rejected by adapter.configure. This previously
 * caused AdapterClient to force-kill the subprocess and emit a fatal sessionError — silently
 * halting polling for every adapter, with no readData ever happening. The session must now be
 * reported as started (degraded) instead, readData must keep returning invalid results without
 * contacting the process, and the process itself must remain alive and usable for a subsequent
 * stop/reconfigure/restart with a compliant device count.
 */
void TestDummyAdapterConfigureError::configureOverDeviceLimitKeepsAdapterAliveAndPollable()
{
    QSignalSpy spyReady(_pAdapterManager, &AdapterManager::adapterReady);
    _pAdapterManager->initAdapter();
    QVERIFY2(spyReady.wait(cSessionTimeoutMs), "adapterReady not emitted");

    /* The dummy adapter's own schema already declares devices.maxItems = 1, matching its real
       enforced limit, so ModbusScope's own truncation would normally prevent this configure from
       ever being rejected. Remove the client-side limit so the over-limit config actually reaches
       the real subprocess. */
    const QJsonObject unboundedDescribe =
      describeWithoutDeviceLimit(_pSettingsModel->adapterData(QString::fromUtf8(cAdapterId)));
    _pSettingsModel->updateAdapterFromDescribe(QString::fromUtf8(cAdapterId), unboundedDescribe);

    QJsonObject config;
    config["general"] = QJsonObject();
    config["connections"] = QJsonArray{ connectionObject(1) };
    config["devices"] = QJsonArray{ deviceObject(1, 1), deviceObject(2, 1) };
    _pSettingsModel->setAdapterCurrentConfig(QString::fromUtf8(cAdapterId), config);

    QSignalSpy spyStarted(_pAdapterManager, &AdapterManager::sessionStarted);
    QSignalSpy spyError(_pAdapterManager, &AdapterManager::sessionError);

    _pAdapterManager->startSession(QStringList{ QStringLiteral("${0}") });

    QVERIFY2(spyStarted.wait(cSessionTimeoutMs), "sessionStarted not emitted after a rejected adapter.configure");
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
       state machine thinks so — by driving a full stop/reconfigure/restart cycle against it with a
       device count within its real limit. */
    QSignalSpy spyStopped(_pAdapterManager, &AdapterManager::sessionStopped);
    _pAdapterManager->stopSession();
    /* A degraded session's stopSession() transitions locally and synchronously (no adapter
       round-trip), so the signal may already have fired before wait() starts watching for a new
       one — check count() first. */
    if (spyStopped.isEmpty())
    {
        QVERIFY2(spyStopped.wait(cSessionTimeoutMs), "sessionStopped not emitted after stopSession");
    }

    QJsonObject compliantConfig;
    compliantConfig["general"] = QJsonObject();
    compliantConfig["connections"] = QJsonArray{ connectionObject(1) };
    compliantConfig["devices"] = QJsonArray{ deviceObject(1, 1) };
    _pSettingsModel->setAdapterCurrentConfig(QString::fromUtf8(cAdapterId), compliantConfig);

    QSignalSpy spyRestarted(_pAdapterManager, &AdapterManager::sessionStarted);
    _pAdapterManager->startSession(QStringList{ QStringLiteral("${0}") });
    QVERIFY2(spyRestarted.wait(cSessionTimeoutMs), "sessionStarted not emitted on retry with a compliant device count");

    QSignalSpy spyRetryData(_pAdapterManager, &AdapterManager::readDataResult);
    _pAdapterManager->requestReadData();
    QVERIFY2(spyRetryData.wait(cReadTimeoutMs), "readDataResult not emitted after successful retry");
    const auto retryResults = spyRetryData.at(0).at(0).value<ResultDoubleList>();
    QCOMPARE(retryResults.size(), 1);
    QVERIFY2(retryResults[0].isValid(), "Expected a real SUCCESS result from the still-running adapter process");
}

QTEST_GUILESS_MAIN(TestDummyAdapterConfigureError)
