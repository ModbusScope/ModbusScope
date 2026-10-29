#include "tst_modbusadapter.h"

#include "ProtocolAdapter/adapterclient.h"
#include "ProtocolAdapter/adapterprocess.h"
#include "util/result.h"

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTest>

#include <memory>

namespace {

constexpr int cSessionTimeoutMs = 10000;
constexpr int cReadTimeoutMs = 5000;

//! Minimal adapter configuration with no connections or devices, so no network I/O is attempted.
QJsonObject minimalConfig()
{
    QJsonObject config;
    config[QStringLiteral("version")] = 1;
    config[QStringLiteral("general")] = QJsonObject();
    config[QStringLiteral("connections")] = QJsonArray();
    config[QStringLiteral("devices")] = QJsonArray();
    return config;
}

} // namespace

/*!
 * \brief Asserts that the shipped adapter binary is executable before any test runs.
 *
 * If the binary is missing or not executable every QSignalSpy::wait() would
 * run to its full ceiling, making the suite hang for minutes before failing.
 * Catching it here gives an immediate, actionable error message instead.
 */
void TestModbusAdapter::initTestCase()
{
    const QString executable = QString::fromUtf8(MODBUS_ADAPTER_EXECUTABLE);
    QVERIFY2(QFileInfo::exists(executable),
             qPrintable(QStringLiteral("MODBUS_ADAPTER_EXECUTABLE not found on disk: %1").arg(executable)));
    QVERIFY2(QFileInfo(executable).isExecutable(),
             qPrintable(QStringLiteral("MODBUS_ADAPTER_EXECUTABLE is not executable: %1").arg(executable)));
}

/*!
 * \brief Creates a fresh AdapterClient before each test.
 *
 * Parenting _pClient to this ensures it is always deleted — even when a
 * QVERIFY/QCOMPARE macro exits the test function early — so no AdapterProcess
 * child is ever leaked across tests.
 */
void TestModbusAdapter::init()
{
    _pClient = new AdapterClient(std::make_unique<AdapterProcess>(), QString(), this);
}

/*!
 * \brief Destroys the AdapterClient after each test.
 *
 * Deleting _pClient also kills the owned AdapterProcess/QProcess child via
 * Qt's object tree, regardless of which state the session is in.
 */
void TestModbusAdapter::cleanup()
{
    delete _pClient;
    _pClient = nullptr;
}

void TestModbusAdapter::describeResultHasRequiredFields()
{
    QSignalSpy spyDescribe(_pClient, &AdapterClient::describeResult);
    QSignalSpy spyStarted(_pClient, &AdapterClient::sessionStarted);
    QSignalSpy spyError(_pClient, &AdapterClient::sessionError);

    _pClient->prepareAdapter(QString::fromUtf8(MODBUS_ADAPTER_EXECUTABLE));

    QVERIFY2(spyDescribe.wait(cSessionTimeoutMs), "No describeResult signal received");
    QCOMPARE(spyError.count(), 0);
    QCOMPARE(spyStarted.count(), 0);

    const QJsonObject result = spyDescribe.at(0).at(0).value<QJsonObject>();

    QVERIFY2(result.contains(QStringLiteral("version")), "describe result missing 'version'");
    QVERIFY2(result.contains(QStringLiteral("configVersion")), "describe result missing 'configVersion'");
    QVERIFY2(result.contains(QStringLiteral("schema")), "describe result missing 'schema'");
    QVERIFY2(result.contains(QStringLiteral("defaults")), "describe result missing 'defaults'");
    QVERIFY2(result.contains(QStringLiteral("capabilities")), "describe result missing 'capabilities'");
    QCOMPARE(result[QStringLiteral("name")].toString(), QStringLiteral("modbusAdapter"));
    QCOMPARE(result[QStringLiteral("protocolName")].toString(), QStringLiteral("modbus"));
}

void TestModbusAdapter::fullLifecycleSessionStarts()
{
    connect(
      _pClient, &AdapterClient::describeResult, _pClient,
      [this]() { _pClient->provideConfig(minimalConfig(), QStringList()); }, Qt::SingleShotConnection);

    QSignalSpy spyStarted(_pClient, &AdapterClient::sessionStarted);
    QSignalSpy spyError(_pClient, &AdapterClient::sessionError);

    _pClient->prepareAdapter(QString::fromUtf8(MODBUS_ADAPTER_EXECUTABLE));

    QVERIFY2(spyStarted.wait(cSessionTimeoutMs), "sessionStarted not emitted");
    QCOMPARE(spyError.count(), 0);

    QSignalSpy spyData(_pClient, &AdapterClient::readDataResult);
    _pClient->requestReadData();
    QVERIFY2(spyData.wait(cReadTimeoutMs), "readDataResult not emitted");

    const auto results = spyData.at(0).at(0).value<ResultDoubleList>();
    QVERIFY(results.isEmpty());
    QCOMPARE(spyError.count(), 0);
}

void TestModbusAdapter::stopSessionEmitsSessionStopped()
{
    connect(
      _pClient, &AdapterClient::describeResult, _pClient,
      [this]() { _pClient->provideConfig(minimalConfig(), QStringList()); }, Qt::SingleShotConnection);

    QSignalSpy spyStarted(_pClient, &AdapterClient::sessionStarted);
    _pClient->prepareAdapter(QString::fromUtf8(MODBUS_ADAPTER_EXECUTABLE));
    QVERIFY2(spyStarted.wait(cSessionTimeoutMs), "sessionStarted not emitted");

    QSignalSpy spyStopped(_pClient, &AdapterClient::sessionStopped);
    _pClient->stopSession();
    QVERIFY2(spyStopped.wait(cSessionTimeoutMs), "sessionStopped not emitted after stopSession");
}

void TestModbusAdapter::dataPointSchemaReturnsRequiredFields()
{
    QSignalSpy spyDescribe(_pClient, &AdapterClient::describeResult);
    QSignalSpy spyError(_pClient, &AdapterClient::sessionError);

    _pClient->prepareAdapter(QString::fromUtf8(MODBUS_ADAPTER_EXECUTABLE));
    QVERIFY2(spyDescribe.wait(cSessionTimeoutMs), "No describeResult signal received");
    QCOMPARE(spyError.count(), 0);

    QSignalSpy spySchema(_pClient, &AdapterClient::dataPointSchemaResult);
    _pClient->requestDataPointSchema();
    QVERIFY2(spySchema.wait(cSessionTimeoutMs), "No dataPointSchemaResult signal received");

    const QJsonObject schema = spySchema.at(0).at(0).value<QJsonObject>();
    QVERIFY2(schema.contains(QStringLiteral("addressSchema")), "dataPointSchema missing 'addressSchema'");
    QVERIFY2(schema.contains(QStringLiteral("defaults")), "dataPointSchema missing 'defaults'");
}

void TestModbusAdapter::describeDataPointReturnsStructuredFields()
{
    QSignalSpy spyDescribe(_pClient, &AdapterClient::describeResult);
    QSignalSpy spyError(_pClient, &AdapterClient::sessionError);

    _pClient->prepareAdapter(QString::fromUtf8(MODBUS_ADAPTER_EXECUTABLE));
    QVERIFY2(spyDescribe.wait(cSessionTimeoutMs), "No describeResult signal received");
    QCOMPARE(spyError.count(), 0);

    QSignalSpy spyResult(_pClient, &AdapterClient::describeDataPointResult);
    _pClient->describeDataPoint(QStringLiteral("${40001}"));
    QVERIFY2(spyResult.wait(cSessionTimeoutMs), "No describeDataPointResult signal received");

    const QJsonObject result = spyResult.at(0).at(1).value<QJsonObject>();
    QVERIFY2(result[QStringLiteral("valid")].toBool(), "Expected valid=true for expression '${40001}'");
    QVERIFY2(result.contains(QStringLiteral("description")), "describeDataPoint result missing 'description'");

    const QJsonObject fields = result[QStringLiteral("fields")].toObject();
    QVERIFY2(fields.contains(QStringLiteral("address")), "describeDataPoint fields missing 'address'");
    QCOMPARE(fields[QStringLiteral("address")].toInt(), 0);
    QCOMPARE(fields[QStringLiteral("deviceId")].toInt(), 1);
}

void TestModbusAdapter::validateDataPointReturnsTrueForValidExpression()
{
    QSignalSpy spyDescribe(_pClient, &AdapterClient::describeResult);
    QSignalSpy spyError(_pClient, &AdapterClient::sessionError);

    _pClient->prepareAdapter(QString::fromUtf8(MODBUS_ADAPTER_EXECUTABLE));
    QVERIFY2(spyDescribe.wait(cSessionTimeoutMs), "No describeResult signal received");
    QCOMPARE(spyError.count(), 0);

    QSignalSpy spyResult(_pClient, &AdapterClient::validateDataPointResult);
    _pClient->validateDataPoint(QStringLiteral("${40001}"));
    QVERIFY2(spyResult.wait(cSessionTimeoutMs), "No validateDataPointResult signal received");

    QVERIFY2(spyResult.at(0).at(0).toBool(), "Expected valid=true for expression '${40001}'");
    QVERIFY2(spyResult.at(0).at(1).toString().isEmpty(), "Expected empty error for valid expression");
}

void TestModbusAdapter::validateDataPointReturnsFalseForInvalidExpression()
{
    QSignalSpy spyDescribe(_pClient, &AdapterClient::describeResult);
    QSignalSpy spyError(_pClient, &AdapterClient::sessionError);

    _pClient->prepareAdapter(QString::fromUtf8(MODBUS_ADAPTER_EXECUTABLE));
    QVERIFY2(spyDescribe.wait(cSessionTimeoutMs), "No describeResult signal received");
    QCOMPARE(spyError.count(), 0);

    QSignalSpy spyResult(_pClient, &AdapterClient::validateDataPointResult);
    _pClient->validateDataPoint(QStringLiteral("not_an_expression"));
    QVERIFY2(spyResult.wait(cSessionTimeoutMs), "No validateDataPointResult signal received");

    QVERIFY2(!spyResult.at(0).at(0).toBool(), "Expected valid=false for invalid expression");
    QVERIFY2(!spyResult.at(0).at(1).toString().isEmpty(), "Expected non-empty error for invalid expression");
    QCOMPARE(spyError.count(), 0);
}

QTEST_GUILESS_MAIN(TestModbusAdapter)
