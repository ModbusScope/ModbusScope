#include "tst_modbusadapterquality.h"

#include "modbusconfighelpers.h"

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
constexpr char cAdapterId[] = "modbus";

//! Nothing listens here, so a device on it can never be read.
constexpr int cUnreachablePort = 1;
} // namespace

void TestModbusAdapterQuality::init()
{
    _pSettingsModel = new SettingsModel;
    _pAdapterManager = new AdapterManager(QString::fromUtf8(cAdapterId), QString::fromUtf8(DUMMY_ADAPTER_EXECUTABLE),
                                          _pSettingsModel, this);
}

void TestModbusAdapterQuality::cleanup()
{
    delete _pAdapterManager;
    _pAdapterManager = nullptr;
    delete _pSettingsModel;
    _pSettingsModel = nullptr;
}

/*!
 * \brief The describe response must echo this application's protocol version, and the Modbus
 * adapter must not claim to report any quality flags.
 */
void TestModbusAdapterQuality::describeAdvertisesProtocolVersionWithoutQualityCapability()
{
    QSignalSpy spyReady(_pAdapterManager, &AdapterManager::adapterReady);
    _pAdapterManager->initAdapter();
    QVERIFY2(spyReady.wait(cSessionTimeoutMs), "adapterReady not emitted");

    const AdapterData* pAdapterData = _pSettingsModel->adapterData(QString::fromUtf8(cAdapterId));
    QVERIFY(pAdapterData != nullptr);
    QCOMPARE(pAdapterData->protocolVersion(), cProtocolVersion);
    QVERIFY(!pAdapterData->reportsQuality());
    QVERIFY(pAdapterData->qualityFlags().isEmpty());
}

/*!
 * \brief Readable registers must decode as plain Good and an unreachable device as Invalid, both
 * without flags, through the real v2 wire encoding.
 */
void TestModbusAdapterQuality::readDataIsGoodOrInvalidWithoutFlags()
{
    QSignalSpy spyReady(_pAdapterManager, &AdapterManager::adapterReady);
    _pAdapterManager->initAdapter();
    QVERIFY2(spyReady.wait(cSessionTimeoutMs), "adapterReady not emitted");

    const QJsonArray connections{ ModbusConfigHelpers::connection(1),
                                  ModbusConfigHelpers::connection(2, cUnreachablePort, 300) };
    const QJsonArray devices{ ModbusConfigHelpers::device(1, 1), ModbusConfigHelpers::device(2, 2) };
    _pSettingsModel->setAdapterCurrentConfig(QString::fromUtf8(cAdapterId),
                                             ModbusConfigHelpers::config(connections, devices));

    QSignalSpy spyStarted(_pAdapterManager, &AdapterManager::sessionStarted);
    QSignalSpy spyError(_pAdapterManager, &AdapterManager::sessionError);

    _pAdapterManager->startSession(
      QStringList{ QStringLiteral("${40001}"), QStringLiteral("${40002}"), QStringLiteral("${40001@2}") });
    QVERIFY2(spyStarted.wait(cSessionTimeoutMs), "sessionStarted not emitted");
    QCOMPARE(spyError.count(), 0);

    QSignalSpy spyData(_pAdapterManager, &AdapterManager::readDataResult);
    _pAdapterManager->requestReadData();
    QVERIFY2(spyData.wait(cReadTimeoutMs), "readDataResult not emitted");

    const auto results = spyData.at(0).at(0).value<ResultDoubleList>();
    QCOMPARE(results.size(), 3);

    /* The stub server stores the 0-based register offset as the register value. */
    QCOMPARE(results[0].state(), DataQuality::State::Good);
    QCOMPARE(results[0].value(), 0.0);
    QCOMPARE(results[0].flags(), DataQuality::Flags(DataQuality::Flag::NoFlags));

    QCOMPARE(results[1].state(), DataQuality::State::Good);
    QCOMPARE(results[1].value(), 1.0);
    QCOMPARE(results[1].flags(), DataQuality::Flags(DataQuality::Flag::NoFlags));

    QCOMPARE(results[2].state(), DataQuality::State::Invalid);
    QCOMPARE(results[2].flags(), DataQuality::Flags(DataQuality::Flag::NoFlags));
}

QTEST_GUILESS_MAIN(TestModbusAdapterQuality)
