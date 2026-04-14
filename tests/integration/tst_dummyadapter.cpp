#include "tst_dummyadapter.h"

#include "ProtocolAdapter/adapterclient.h"
#include "ProtocolAdapter/adapterprocess.h"
#include "util/result.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTest>

namespace {

constexpr int cSessionTimeoutMs = 10000;
constexpr int cReadTimeoutMs = 5000;

// Dummy adapter built-in server settings
constexpr const char* const DUMMY_IP = "127.0.0.1";
constexpr quint16 DUMMY_PORT = 5020;
constexpr quint8 DUMMY_SLAVE_ID = 1;

//! Minimal adapter configuration with no real connections or devices.
//! The dummymodbusadapter accepts this without attempting any real I/O.
QJsonObject minimalConfig()
{
    QJsonObject config;
    config[QStringLiteral("version")] = 1;
    config[QStringLiteral("general")] = QJsonObject();
    config[QStringLiteral("connections")] = QJsonArray();
    config[QStringLiteral("devices")] = QJsonArray();
    return config;
}

//! Full configuration pointing at the dummymodbusadapter's built-in TCP server.
QJsonObject realConfig()
{
    QJsonObject connection;
    connection[QStringLiteral("id")] = 0;
    connection[QStringLiteral("type")] = QStringLiteral("tcp");
    connection[QStringLiteral("ip")] = DUMMY_IP;
    connection[QStringLiteral("port")] = static_cast<int>(DUMMY_PORT);
    connection[QStringLiteral("persistent")] = true;
    connection[QStringLiteral("timeout")] = 1000;

    QJsonObject device;
    device[QStringLiteral("id")] = 1;
    device[QStringLiteral("connectionId")] = 0;
    device[QStringLiteral("slaveId")] = static_cast<int>(DUMMY_SLAVE_ID);

    QJsonObject config;
    config[QStringLiteral("version")] = 1;
    config[QStringLiteral("general")] = QJsonObject();
    config[QStringLiteral("connections")] = QJsonArray({ connection });
    config[QStringLiteral("devices")] = QJsonArray({ device });
    return config;
}

} // namespace

/*!
 * \brief Creates a fresh AdapterClient before each test.
 *
 * Parenting _pClient to this ensures it is always deleted — even when a
 * QVERIFY/QCOMPARE macro exits the test function early — so no AdapterProcess
 * child is ever leaked across tests.
 */
void TestDummyAdapter::init()
{
    _pClient = new AdapterClient(new AdapterProcess(), this);
}

/*!
 * \brief Destroys the AdapterClient after each test.
 *
 * Deleting _pClient also kills the owned AdapterProcess/QProcess child via
 * Qt's object tree, regardless of which state the session is in.
 */
void TestDummyAdapter::cleanup()
{
    delete _pClient;
    _pClient = nullptr;
}

void TestDummyAdapter::describeResultHasRequiredFields()
{
    QSignalSpy spyDescribe(_pClient, &AdapterClient::describeResult);
    QSignalSpy spyStarted(_pClient, &AdapterClient::sessionStarted);
    QSignalSpy spyError(_pClient, &AdapterClient::sessionError);

    _pClient->prepareAdapter(QString::fromUtf8(DUMMY_ADAPTER_EXECUTABLE));

    QVERIFY2(spyDescribe.wait(cSessionTimeoutMs), "No describeResult signal received");
    QCOMPARE(spyError.count(), 0);
    QCOMPARE(spyStarted.count(), 0);

    const QJsonObject result = spyDescribe.at(0).at(0).value<QJsonObject>();

    QVERIFY2(result.contains(QStringLiteral("name")), "describe result missing 'name'");
    QVERIFY2(result.contains(QStringLiteral("version")), "describe result missing 'version'");
    QVERIFY2(result.contains(QStringLiteral("configVersion")), "describe result missing 'configVersion'");
    QVERIFY2(result.contains(QStringLiteral("schema")), "describe result missing 'schema'");
    QVERIFY2(result.contains(QStringLiteral("defaults")), "describe result missing 'defaults'");
    QVERIFY2(result.contains(QStringLiteral("capabilities")), "describe result missing 'capabilities'");
    QCOMPARE(result[QStringLiteral("name")].toString(), QStringLiteral("modbusAdapter"));
}

void TestDummyAdapter::fullLifecycleSessionStarts()
{
    connect(
      _pClient, &AdapterClient::describeResult, _pClient,
      [this]() { _pClient->provideConfig(minimalConfig(), QStringList()); }, Qt::SingleShotConnection);

    QSignalSpy spyStarted(_pClient, &AdapterClient::sessionStarted);
    QSignalSpy spyError(_pClient, &AdapterClient::sessionError);

    _pClient->prepareAdapter(QString::fromUtf8(DUMMY_ADAPTER_EXECUTABLE));

    QVERIFY2(spyStarted.wait(cSessionTimeoutMs), "sessionStarted not emitted");
    QCOMPARE(spyError.count(), 0);

    QSignalSpy spyData(_pClient, &AdapterClient::readDataResult);
    _pClient->requestReadData();
    QVERIFY2(spyData.wait(cReadTimeoutMs), "readDataResult not emitted");
}

void TestDummyAdapter::readRegistersReturnsValidData()
{
    const QStringList registers = { "${40001}", "${40002}", "${40010}" };

    connect(
      _pClient, &AdapterClient::describeResult, _pClient,
      [this, registers]() { _pClient->provideConfig(realConfig(), registers); }, Qt::SingleShotConnection);

    QSignalSpy spyStarted(_pClient, &AdapterClient::sessionStarted);
    QSignalSpy spyError(_pClient, &AdapterClient::sessionError);

    _pClient->prepareAdapter(QString::fromUtf8(DUMMY_ADAPTER_EXECUTABLE));

    QVERIFY2(spyStarted.wait(cSessionTimeoutMs), "sessionStarted not emitted");
    QCOMPARE(spyError.count(), 0);

    QSignalSpy spyData(_pClient, &AdapterClient::readDataResult);
    _pClient->requestReadData();
    QVERIFY2(spyData.wait(cReadTimeoutMs), "readDataResult not emitted");

    const auto results = spyData.at(0).at(0).value<ResultDoubleList>();
    QCOMPARE(results.size(), registers.size());
    // The dummymodbusadapter stores the 0-based register offset as the register value:
    // register 40001 → offset 0, register 40002 → offset 1, register 40010 → offset 9.
    QVERIFY2(results[0].isValid(), "Expected SUCCESS for register 40001");
    QCOMPARE(results[0].value(), 0.0);
    QVERIFY2(results[1].isValid(), "Expected SUCCESS for register 40002");
    QCOMPARE(results[1].value(), 1.0);
    QVERIFY2(results[2].isValid(), "Expected SUCCESS for register 40010");
    QCOMPARE(results[2].value(), 9.0);
}

void TestDummyAdapter::multipleReadCyclesAllSucceed()
{
    const QStringList registers = { "${40001}", "${40005}" };

    connect(
      _pClient, &AdapterClient::describeResult, _pClient,
      [this, registers]() { _pClient->provideConfig(realConfig(), registers); }, Qt::SingleShotConnection);

    QSignalSpy spyStarted(_pClient, &AdapterClient::sessionStarted);
    _pClient->prepareAdapter(QString::fromUtf8(DUMMY_ADAPTER_EXECUTABLE));
    QVERIFY2(spyStarted.wait(cSessionTimeoutMs), "sessionStarted not emitted");

    QSignalSpy spyData(_pClient, &AdapterClient::readDataResult);

    constexpr int cReadCycles = 3;
    for (int cycle = 0; cycle < cReadCycles; ++cycle)
    {
        _pClient->requestReadData();
        QVERIFY2(spyData.wait(cReadTimeoutMs), "readDataResult not emitted");

        const auto results = spyData.takeLast().at(0).value<ResultDoubleList>();
        QCOMPARE(results.size(), registers.size());
        // The dummymodbusadapter stores the 0-based register offset as the value.
        QVERIFY2(results[0].isValid(), "Expected SUCCESS for register 40001 on each cycle");
        QCOMPARE(results[0].value(), 0.0);
        QVERIFY2(results[1].isValid(), "Expected SUCCESS for register 40005 on each cycle");
        QCOMPARE(results[1].value(), 4.0);
    }
}

void TestDummyAdapter::stopSessionEmitsSessionStopped()
{
    connect(
      _pClient, &AdapterClient::describeResult, _pClient,
      [this]() { _pClient->provideConfig(minimalConfig(), QStringList()); }, Qt::SingleShotConnection);

    QSignalSpy spyStarted(_pClient, &AdapterClient::sessionStarted);
    _pClient->prepareAdapter(QString::fromUtf8(DUMMY_ADAPTER_EXECUTABLE));
    QVERIFY2(spyStarted.wait(cSessionTimeoutMs), "sessionStarted not emitted");

    QSignalSpy spyStopped(_pClient, &AdapterClient::sessionStopped);
    _pClient->stopSession();
    QVERIFY2(spyStopped.wait(cSessionTimeoutMs), "sessionStopped not emitted after stopSession");
}

void TestDummyAdapter::dataPointSchemaReturnsRequiredFields()
{
    QSignalSpy spyDescribe(_pClient, &AdapterClient::describeResult);
    QSignalSpy spyError(_pClient, &AdapterClient::sessionError);

    _pClient->prepareAdapter(QString::fromUtf8(DUMMY_ADAPTER_EXECUTABLE));
    QVERIFY2(spyDescribe.wait(cSessionTimeoutMs), "No describeResult signal received");
    QCOMPARE(spyError.count(), 0);

    QSignalSpy spySchema(_pClient, &AdapterClient::dataPointSchemaResult);
    _pClient->requestDataPointSchema();
    QVERIFY2(spySchema.wait(cSessionTimeoutMs), "No dataPointSchemaResult signal received");

    const QJsonObject schema = spySchema.at(0).at(0).value<QJsonObject>();
    QVERIFY2(schema.contains(QStringLiteral("addressSchema")), "dataPointSchema missing 'addressSchema'");
    QVERIFY2(schema.contains(QStringLiteral("dataTypes")), "dataPointSchema missing 'dataTypes'");
    QVERIFY2(schema.contains(QStringLiteral("defaultDataType")), "dataPointSchema missing 'defaultDataType'");
}

void TestDummyAdapter::describeDataPointReturnsStructuredFields()
{
    QSignalSpy spyDescribe(_pClient, &AdapterClient::describeResult);
    QSignalSpy spyError(_pClient, &AdapterClient::sessionError);

    _pClient->prepareAdapter(QString::fromUtf8(DUMMY_ADAPTER_EXECUTABLE));
    QVERIFY2(spyDescribe.wait(cSessionTimeoutMs), "No describeResult signal received");
    QCOMPARE(spyError.count(), 0);

    QSignalSpy spyResult(_pClient, &AdapterClient::describeDataPointResult);
    _pClient->describeDataPoint(QStringLiteral("${40001}"));
    QVERIFY2(spyResult.wait(cSessionTimeoutMs), "No describeDataPointResult signal received");

    const QJsonObject result = spyResult.at(0).at(0).value<QJsonObject>();
    QVERIFY2(result[QStringLiteral("valid")].toBool(), "Expected valid=true for known expression '${40001}'");
    QVERIFY2(result.contains(QStringLiteral("fields")), "describeDataPoint result missing 'fields'");
    QVERIFY2(result.contains(QStringLiteral("description")), "describeDataPoint result missing 'description'");
}

void TestDummyAdapter::validateDataPointReturnsTrueForValidExpression()
{
    QSignalSpy spyDescribe(_pClient, &AdapterClient::describeResult);
    QSignalSpy spyError(_pClient, &AdapterClient::sessionError);

    _pClient->prepareAdapter(QString::fromUtf8(DUMMY_ADAPTER_EXECUTABLE));
    QVERIFY2(spyDescribe.wait(cSessionTimeoutMs), "No describeResult signal received");
    QCOMPARE(spyError.count(), 0);

    QSignalSpy spyResult(_pClient, &AdapterClient::validateDataPointResult);
    _pClient->validateDataPoint(QStringLiteral("${40001}"));
    QVERIFY2(spyResult.wait(cSessionTimeoutMs), "No validateDataPointResult signal received");

    QVERIFY2(spyResult.at(0).at(0).toBool(), "Expected valid=true for known expression '${40001}'");
    QVERIFY2(spyResult.at(0).at(1).toString().isEmpty(), "Expected empty error for valid expression");
}

void TestDummyAdapter::validateDataPointReturnsFalseForInvalidExpression()
{
    QSignalSpy spyDescribe(_pClient, &AdapterClient::describeResult);
    QSignalSpy spyError(_pClient, &AdapterClient::sessionError);

    _pClient->prepareAdapter(QString::fromUtf8(DUMMY_ADAPTER_EXECUTABLE));
    QVERIFY2(spyDescribe.wait(cSessionTimeoutMs), "No describeResult signal received");
    QCOMPARE(spyError.count(), 0);

    QSignalSpy spyResult(_pClient, &AdapterClient::validateDataPointResult);
    _pClient->validateDataPoint(QStringLiteral("not_an_expression"));
    QVERIFY2(spyResult.wait(cSessionTimeoutMs), "No validateDataPointResult signal received");

    QVERIFY2(!spyResult.at(0).at(0).toBool(), "Expected valid=false for invalid expression");
    QVERIFY2(!spyResult.at(0).at(1).toString().isEmpty(), "Expected non-empty error for invalid expression");
}

QTEST_GUILESS_MAIN(TestDummyAdapter)
