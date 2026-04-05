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
constexpr const char* DUMMY_IP = "127.0.0.1";
constexpr quint16 DUMMY_PORT = 5020;
constexpr quint8 DUMMY_SLAVE_ID = 1;

//! Minimal adapter configuration with no real connections or devices.
//! The dummymodbusadapter accepts this without attempting any real I/O.
QJsonObject minimalConfig()
{
    QJsonObject config;
    config["version"] = 1;
    config["general"] = QJsonObject();
    config["connections"] = QJsonArray();
    config["devices"] = QJsonArray();
    return config;
}

//! Full configuration pointing at the dummymodbusadapter's built-in TCP server.
QJsonObject realConfig()
{
    QJsonObject connection;
    connection["id"] = 0;
    connection["type"] = QStringLiteral("tcp");
    connection["ip"] = DUMMY_IP;
    connection["port"] = static_cast<int>(DUMMY_PORT);
    connection["persistent"] = true;
    connection["timeout"] = 1000;

    QJsonObject device;
    device["id"] = 1;
    device["connectionId"] = 0;
    device["slaveId"] = static_cast<int>(DUMMY_SLAVE_ID);

    QJsonObject config;
    config["version"] = 1;
    config["general"] = QJsonObject();
    config["connections"] = QJsonArray({ connection });
    config["devices"] = QJsonArray({ device });
    return config;
}

} // namespace

void TestDummyAdapter::describeReturnsRequiredFields()
{
    auto* process = new AdapterProcess();
    AdapterClient client(process);

    QSignalSpy spyDescribe(&client, &AdapterClient::describeResult);
    QSignalSpy spyStarted(&client, &AdapterClient::sessionStarted);
    QSignalSpy spyError(&client, &AdapterClient::sessionError);

    client.prepareAdapter(QString::fromUtf8(DUMMY_ADAPTER_EXECUTABLE));

    QVERIFY2(spyDescribe.wait(cSessionTimeoutMs), "No describeResult signal received");
    QCOMPARE(spyError.count(), 0);
    QCOMPARE(spyStarted.count(), 0);

    QJsonObject result = spyDescribe.at(0).at(0).value<QJsonObject>();

    QVERIFY2(result.contains("name"), "describe result missing 'name'");
    QVERIFY2(result.contains("version"), "describe result missing 'version'");
    QVERIFY2(result.contains("configVersion"), "describe result missing 'configVersion'");
    QVERIFY2(result.contains("schema"), "describe result missing 'schema'");
    QVERIFY2(result.contains("defaults"), "describe result missing 'defaults'");
    QVERIFY2(result.contains("capabilities"), "describe result missing 'capabilities'");

    client.stopSession();
}

void TestDummyAdapter::describeNameIsModbusAdapter()
{
    auto* process = new AdapterProcess();
    AdapterClient client(process);

    QSignalSpy spyDescribe(&client, &AdapterClient::describeResult);
    QSignalSpy spyStarted(&client, &AdapterClient::sessionStarted);

    client.prepareAdapter(QString::fromUtf8(DUMMY_ADAPTER_EXECUTABLE));

    QVERIFY(spyDescribe.wait(cSessionTimeoutMs));
    QCOMPARE(spyStarted.count(), 0);

    QJsonObject result = spyDescribe.at(0).at(0).value<QJsonObject>();
    QCOMPARE(result["name"].toString(), QStringLiteral("modbusAdapter"));

    client.stopSession();
}

void TestDummyAdapter::fullLifecycleSessionStarts()
{
    auto* process = new AdapterProcess();
    AdapterClient client(process);

    /* Provide config when describe result arrives so the lifecycle can continue */
    connect(&client, &AdapterClient::describeResult, &client,
            [&client]() { client.provideConfig(minimalConfig(), QStringList()); });

    QSignalSpy spyStarted(&client, &AdapterClient::sessionStarted);
    QSignalSpy spyError(&client, &AdapterClient::sessionError);

    client.prepareAdapter(QString::fromUtf8(DUMMY_ADAPTER_EXECUTABLE));

    QVERIFY2(spyStarted.wait(cSessionTimeoutMs), "sessionStarted not emitted");
    QCOMPARE(spyError.count(), 0);

    QSignalSpy spyData(&client, &AdapterClient::readDataResult);
    client.requestReadData();
    QVERIFY2(spyData.wait(cReadTimeoutMs), "readDataResult not emitted");

    client.stopSession();
}

void TestDummyAdapter::readRegistersReturnsValidData()
{
    auto* process = new AdapterProcess();
    AdapterClient client(process);

    const QStringList registers = { "${40001}", "${40002}", "${40010}" };

    connect(&client, &AdapterClient::describeResult, &client,
            [&client, &registers]() { client.provideConfig(realConfig(), registers); });

    QSignalSpy spyStarted(&client, &AdapterClient::sessionStarted);
    QSignalSpy spyError(&client, &AdapterClient::sessionError);

    client.prepareAdapter(QString::fromUtf8(DUMMY_ADAPTER_EXECUTABLE));

    QVERIFY2(spyStarted.wait(cSessionTimeoutMs), "sessionStarted not emitted");
    QCOMPARE(spyError.count(), 0);

    QSignalSpy spyData(&client, &AdapterClient::readDataResult);
    client.requestReadData();
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

    client.stopSession();
}

void TestDummyAdapter::multipleReadCyclesAllSucceed()
{
    auto* process = new AdapterProcess();
    AdapterClient client(process);

    const QStringList registers = { "${40001}", "${40005}" };

    connect(&client, &AdapterClient::describeResult, &client,
            [&client, &registers]() { client.provideConfig(realConfig(), registers); });

    QSignalSpy spyStarted(&client, &AdapterClient::sessionStarted);
    client.prepareAdapter(QString::fromUtf8(DUMMY_ADAPTER_EXECUTABLE));
    QVERIFY2(spyStarted.wait(cSessionTimeoutMs), "sessionStarted not emitted");

    QSignalSpy spyData(&client, &AdapterClient::readDataResult);

    constexpr int cReadCycles = 3;
    for (int cycle = 0; cycle < cReadCycles; ++cycle)
    {
        client.requestReadData();
        QVERIFY2(spyData.wait(cReadTimeoutMs), "readDataResult not emitted");

        const auto results = spyData.takeLast().at(0).value<ResultDoubleList>();
        QCOMPARE(results.size(), registers.size());
        // The dummymodbusadapter stores the 0-based register offset as the value.
        QVERIFY2(results[0].isValid(), "Expected SUCCESS for register 40001 on each cycle");
        QCOMPARE(results[0].value(), 0.0);
        QVERIFY2(results[1].isValid(), "Expected SUCCESS for register 40005 on each cycle");
        QCOMPARE(results[1].value(), 4.0);
    }

    client.stopSession();
}

void TestDummyAdapter::stopSessionEmitsSessionStopped()
{
    auto* process = new AdapterProcess();
    AdapterClient client(process);

    connect(&client, &AdapterClient::describeResult, &client,
            [&client]() { client.provideConfig(realConfig(), QStringList()); });

    QSignalSpy spyStarted(&client, &AdapterClient::sessionStarted);
    client.prepareAdapter(QString::fromUtf8(DUMMY_ADAPTER_EXECUTABLE));
    QVERIFY2(spyStarted.wait(cSessionTimeoutMs), "sessionStarted not emitted");

    QSignalSpy spyStopped(&client, &AdapterClient::sessionStopped);
    client.stopSession();
    QVERIFY2(spyStopped.wait(cSessionTimeoutMs), "sessionStopped not emitted after stopSession");
}

QTEST_GUILESS_MAIN(TestDummyAdapter)
