#include "tst_adapterclient.h"

#include "ProtocolAdapter/adapterclient.h"
#include "ProtocolAdapter/adapterprocess.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTest>

/* ---- Mock AdapterProcess ---- */

/*!
 * \brief Test double for AdapterProcess.
 *
 * Overrides start/stop/sendRequest so no real process is spawned.
 * Records all outgoing requests in sentRequests and exposes inject helpers
 * to fire responseReceived/errorReceived as if the adapter had replied.
 */
class MockAdapterProcess : public AdapterProcess
{
public:
    struct SentRequest
    {
        QString method;
        QJsonObject params;
    };

    QList<SentRequest> sentRequests;

    bool start(const QString&) override
    {
        return true;
    }

    void stop() override
    {
    }

    int sendRequest(const QString& method, const QJsonObject& params) override
    {
        int id = _nextMockId++;
        sentRequests.append({ method, params });
        return id;
    }

    //! Simulate a successful response from the adapter for the given request ID.
    void injectResponse(int id, const QString& method, const QJsonValue& result)
    {
        emit responseReceived(id, method, result);
    }

    //! Simulate an error response from the adapter for the given request ID.
    void injectError(int id, const QString& method, const QJsonObject& error)
    {
        emit errorReceived(id, method, error);
    }

    //! Simulate a process-level error (crash, write failure, etc.).
    void injectProcessError(const QString& message)
    {
        emit processError(message);
    }

    //! Simulate a server-initiated notification from the adapter.
    void injectNotification(const QString& method, const QJsonValue& params)
    {
        emit notificationReceived(method, params);
    }

private:
    int _nextMockId{ 1 };
};

/* ---- Helpers ---- */

static QJsonObject describeResult()
{
    QJsonObject caps;
    caps["supportsHotReload"] = false;
    caps["requiresRestartOn"] = QJsonArray{ "connections", "devices" };

    QJsonObject result;
    result["name"] = "modbusAdapter";
    result["version"] = "1.0.0";
    result["configVersion"] = 1;
    result["schema"] = QJsonObject{};
    result["defaults"] = QJsonObject{};
    result["capabilities"] = caps;
    return result;
}

/* ---- Tests ---- */

void TestAdapterClient::init()
{
}

void TestAdapterClient::cleanup()
{
}

void TestAdapterClient::lifecycleInitializeToStart()
{
    auto* mock = new MockAdapterProcess();
    AdapterClient client(mock);

    QSignalSpy spyStarted(&client, &AdapterClient::sessionStarted);
    QSignalSpy spyError(&client, &AdapterClient::sessionError);
    QSignalSpy spyDescribe(&client, &AdapterClient::describeResult);

    client.prepareAdapter(QStringLiteral("./dummy"));

    /* 1. initialize was sent */
    QCOMPARE(mock->sentRequests.size(), 1);
    QCOMPARE(mock->sentRequests[0].method, QStringLiteral("adapter.initialize"));

    /* 2. initialize ok → describe sent */
    mock->injectResponse(1, "adapter.initialize", QJsonObject{ { "status", "ok" } });
    QCOMPARE(mock->sentRequests.size(), 2);
    QCOMPARE(mock->sentRequests[1].method, QStringLiteral("adapter.describe"));
    QCOMPARE(spyStarted.count(), 0);

    /* 3. describe ok → describeResult emitted, awaiting config */
    mock->injectResponse(2, "adapter.describe", describeResult());
    QCOMPARE(mock->sentRequests.size(), 2);
    QCOMPARE(spyDescribe.count(), 1);
    QCOMPARE(spyStarted.count(), 0);

    /* 4. provide config → configure sent */
    client.provideConfig(QJsonObject(), QStringList());
    QCOMPARE(mock->sentRequests.size(), 3);
    QCOMPARE(mock->sentRequests[2].method, QStringLiteral("adapter.configure"));
    QCOMPARE(spyStarted.count(), 0);

    /* 5. configure ok → start sent */
    mock->injectResponse(3, "adapter.configure", QJsonObject{ { "status", "ok" } });
    QCOMPARE(mock->sentRequests.size(), 4);
    QCOMPARE(mock->sentRequests[3].method, QStringLiteral("adapter.start"));
    QCOMPARE(spyStarted.count(), 0);

    /* 6. start ok → sessionStarted emitted */
    mock->injectResponse(4, "adapter.start", QJsonObject{ { "status", "ok" } });
    QCOMPARE(spyStarted.count(), 1);
    QCOMPARE(spyError.count(), 0);
}

void TestAdapterClient::describeSignalEmitted()
{
    auto* mock = new MockAdapterProcess();
    AdapterClient client(mock);

    QSignalSpy spyDescribe(&client, &AdapterClient::describeResult);

    client.prepareAdapter(QStringLiteral("./dummy"));

    mock->injectResponse(1, "adapter.initialize", QJsonObject{ { "status", "ok" } });

    QJsonObject expected = describeResult();
    mock->injectResponse(2, "adapter.describe", expected);

    QCOMPARE(spyDescribe.count(), 1);
    QJsonObject received = spyDescribe.at(0).at(0).value<QJsonObject>();
    QCOMPARE(received["name"].toString(), QStringLiteral("modbusAdapter"));
    QCOMPARE(received["configVersion"].toInt(), 1);
}

void TestAdapterClient::readDataValidResults()
{
    auto* mock = new MockAdapterProcess();
    AdapterClient client(mock);

    QSignalSpy spy(&client, &AdapterClient::readDataResult);

    /* Drive to ACTIVE state */
    client.prepareAdapter(QStringLiteral("./dummy"));
    mock->injectResponse(1, "adapter.initialize", QJsonObject{ { "status", "ok" } });
    mock->injectResponse(2, "adapter.describe", describeResult());
    client.provideConfig(QJsonObject(), QStringList());
    mock->injectResponse(3, "adapter.configure", QJsonObject{ { "status", "ok" } });
    mock->injectResponse(4, "adapter.start", QJsonObject{ { "status", "ok" } });

    client.requestReadData();

    QJsonArray registers;
    registers.append(QJsonObject{ { "valid", true }, { "value", 42.0 } });
    registers.append(QJsonObject{ { "valid", false }, { "value", 0.0 } });
    mock->injectResponse(5, "adapter.readData", QJsonObject{ { "registers", registers } });

    QCOMPARE(spy.count(), 1);
    ResultDoubleList results = spy.at(0).at(0).value<ResultDoubleList>();
    QCOMPARE(results.size(), 2);
    QVERIFY(results[0].isValid());
    QCOMPARE(results[0].value(), 42.0);
    QVERIFY(!results[1].isValid());
}

void TestAdapterClient::readDataEmptyRegisters()
{
    auto* mock = new MockAdapterProcess();
    AdapterClient client(mock);

    QSignalSpy spy(&client, &AdapterClient::readDataResult);

    client.prepareAdapter(QStringLiteral("./dummy"));
    mock->injectResponse(1, "adapter.initialize", QJsonObject{ { "status", "ok" } });
    mock->injectResponse(2, "adapter.describe", describeResult());
    client.provideConfig(QJsonObject(), QStringList());
    mock->injectResponse(3, "adapter.configure", QJsonObject{ { "status", "ok" } });
    mock->injectResponse(4, "adapter.start", QJsonObject{ { "status", "ok" } });

    client.requestReadData();
    mock->injectResponse(5, "adapter.readData", QJsonObject{ { "registers", QJsonArray{} } });

    QCOMPARE(spy.count(), 1);
    ResultDoubleList results = spy.at(0).at(0).value<ResultDoubleList>();
    QCOMPARE(results.size(), 0);
}

void TestAdapterClient::requestStatusEmitsSignal()
{
    auto* mock = new MockAdapterProcess();
    AdapterClient client(mock);

    QSignalSpy spy(&client, &AdapterClient::statusResult);

    client.prepareAdapter(QStringLiteral("./dummy"));
    mock->injectResponse(1, "adapter.initialize", QJsonObject{ { "status", "ok" } });
    mock->injectResponse(2, "adapter.describe", describeResult());
    client.provideConfig(QJsonObject(), QStringList());
    mock->injectResponse(3, "adapter.configure", QJsonObject{ { "status", "ok" } });
    mock->injectResponse(4, "adapter.start", QJsonObject{ { "status", "ok" } });

    client.requestStatus();
    mock->injectResponse(5, "adapter.getStatus", QJsonObject{ { "active", true } });

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toBool(), true);
}

void TestAdapterClient::errorResponseEmitsSessionError()
{
    auto* mock = new MockAdapterProcess();
    AdapterClient client(mock);

    QSignalSpy spy(&client, &AdapterClient::sessionError);

    client.prepareAdapter(QStringLiteral("./dummy"));

    QJsonObject error;
    error["code"] = -32602;
    error["message"] = "bad params";
    mock->injectError(1, "adapter.initialize", error);

    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.at(0).at(0).toString().contains("bad params"));
}

void TestAdapterClient::unexpectedResponseEmitsNoSignals()
{
    auto* mock = new MockAdapterProcess();
    AdapterClient client(mock);

    QSignalSpy spyStarted(&client, &AdapterClient::sessionStarted);
    QSignalSpy spyError(&client, &AdapterClient::sessionError);
    QSignalSpy spyData(&client, &AdapterClient::readDataResult);

    /* Inject a response for an ID that was never requested */
    mock->injectResponse(99, "adapter.readData", QJsonObject{ { "registers", QJsonArray{} } });

    QCOMPARE(spyStarted.count(), 0);
    QCOMPARE(spyError.count(), 0);
    QCOMPARE(spyData.count(), 0);
}

void TestAdapterClient::notificationIgnored()
{
    auto* mock = new MockAdapterProcess();
    AdapterClient client(mock);

    QSignalSpy spyStarted(&client, &AdapterClient::sessionStarted);
    QSignalSpy spyError(&client, &AdapterClient::sessionError);
    QSignalSpy spyData(&client, &AdapterClient::readDataResult);
    QSignalSpy spyDiagnostic(&client, &AdapterClient::diagnosticReceived);

    /* Simulate a non-diagnostic notification and verify all signals stay silent */
    mock->injectNotification("adapter.progress", QJsonObject{ { "percent", 50 } });

    QCOMPARE(spyStarted.count(), 0);
    QCOMPARE(spyError.count(), 0);
    QCOMPARE(spyData.count(), 0);
    QCOMPARE(spyDiagnostic.count(), 0);
}

void TestAdapterClient::diagnosticNotificationForwarded()
{
    auto* mock = new MockAdapterProcess();
    AdapterClient client(mock);

    QSignalSpy spyDiagnostic(&client, &AdapterClient::diagnosticReceived);

    mock->injectNotification("adapter.diagnostic",
                             QJsonObject{ { "level", "warning" }, { "message", "connection lost" } });

    QCOMPARE(spyDiagnostic.count(), 1);
    QCOMPARE(spyDiagnostic.at(0).at(0).toString(), QStringLiteral("warning"));
    QCOMPARE(spyDiagnostic.at(0).at(1).toString(), QStringLiteral("connection lost"));
}

void TestAdapterClient::diagnosticNotificationDebugLevel()
{
    auto* mock = new MockAdapterProcess();
    AdapterClient client(mock);

    QSignalSpy spyDiagnostic(&client, &AdapterClient::diagnosticReceived);

    mock->injectNotification("adapter.diagnostic",
                             QJsonObject{ { "level", "debug" }, { "message", "polling started" } });

    QCOMPARE(spyDiagnostic.count(), 1);
    QCOMPARE(spyDiagnostic.at(0).at(0).toString(), QStringLiteral("debug"));
    QCOMPARE(spyDiagnostic.at(0).at(1).toString(), QStringLiteral("polling started"));
}

void TestAdapterClient::diagnosticMalformedParams()
{
    auto* mock = new MockAdapterProcess();
    AdapterClient client(mock);

    QSignalSpy spyDiagnostic(&client, &AdapterClient::diagnosticReceived);

    /* params is not an object — diagnosticReceived must not be emitted */
    mock->injectNotification("adapter.diagnostic", QJsonValue(42));

    QCOMPARE(spyDiagnostic.count(), 0);
}

void TestAdapterClient::processErrorEmitsSessionError()
{
    auto* mock = new MockAdapterProcess();
    AdapterClient client(mock);

    QSignalSpy spy(&client, &AdapterClient::sessionError);

    client.prepareAdapter(QStringLiteral("./dummy"));
    mock->injectProcessError(QStringLiteral("Adapter process crashed"));

    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.at(0).at(0).toString().contains("crashed"));
}

void TestAdapterClient::stopSessionDuringLifecycle()
{
    auto* mock = new MockAdapterProcess();
    AdapterClient client(mock);

    QSignalSpy spyError(&client, &AdapterClient::sessionError);

    client.prepareAdapter(QStringLiteral("./dummy"));

    /* Drive to DESCRIBING state */
    mock->injectResponse(1, "adapter.initialize", QJsonObject{ { "status", "ok" } });
    QCOMPARE(mock->sentRequests.size(), 2);

    /* Stop during lifecycle — should not crash, should go to IDLE */
    client.stopSession();

    /* No sessionError should be emitted for an intentional stop */
    QCOMPARE(spyError.count(), 0);
}

void TestAdapterClient::doubleStopSession()
{
    auto* mock = new MockAdapterProcess();
    AdapterClient client(mock);

    QSignalSpy spyError(&client, &AdapterClient::sessionError);

    /* Drive to ACTIVE state */
    client.prepareAdapter(QStringLiteral("./dummy"));
    mock->injectResponse(1, "adapter.initialize", QJsonObject{ { "status", "ok" } });
    mock->injectResponse(2, "adapter.describe", describeResult());
    client.provideConfig(QJsonObject(), QStringList());
    mock->injectResponse(3, "adapter.configure", QJsonObject{ { "status", "ok" } });
    mock->injectResponse(4, "adapter.start", QJsonObject{ { "status", "ok" } });

    /* First stop sends shutdown */
    client.stopSession();
    int requestsAfterFirstStop = mock->sentRequests.size();

    /* Second stop should be a no-op (state is STOPPING) */
    client.stopSession();
    QCOMPARE(mock->sentRequests.size(), requestsAfterFirstStop);
    QCOMPARE(spyError.count(), 0);
}

void TestAdapterClient::requestReadDataWhenNotActive()
{
    auto* mock = new MockAdapterProcess();
    AdapterClient client(mock);

    QSignalSpy spyData(&client, &AdapterClient::readDataResult);
    QSignalSpy spyError(&client, &AdapterClient::sessionError);

    /* Call requestReadData in IDLE state — should be silently ignored */
    client.requestReadData();

    QCOMPARE(spyData.count(), 0);
    QCOMPARE(spyError.count(), 0);
    QCOMPARE(mock->sentRequests.size(), 0);
}

void TestAdapterClient::nonObjectResultEmitsSessionError()
{
    auto* mock = new MockAdapterProcess();
    AdapterClient client(mock);

    QSignalSpy spyError(&client, &AdapterClient::sessionError);

    client.prepareAdapter(QStringLiteral("./dummy"));

    /* Inject a non-object result (string instead of object) */
    mock->injectResponse(1, "adapter.initialize", QJsonValue(QString("unexpected string")));

    QCOMPARE(spyError.count(), 1);
    QVERIFY(spyError.at(0).at(0).toString().contains("non-object"));
}

void TestAdapterClient::errorDuringShutdownSuppressed()
{
    auto* mock = new MockAdapterProcess();
    AdapterClient client(mock);

    QSignalSpy spyError(&client, &AdapterClient::sessionError);

    /* Drive to ACTIVE state */
    client.prepareAdapter(QStringLiteral("./dummy"));
    mock->injectResponse(1, "adapter.initialize", QJsonObject{ { "status", "ok" } });
    mock->injectResponse(2, "adapter.describe", describeResult());
    client.provideConfig(QJsonObject(), QStringList());
    mock->injectResponse(3, "adapter.configure", QJsonObject{ { "status", "ok" } });
    mock->injectResponse(4, "adapter.start", QJsonObject{ { "status", "ok" } });

    /* Initiate shutdown */
    client.stopSession();

    /* Simulate an error response for the shutdown request */
    QJsonObject error;
    error["code"] = -32603;
    error["message"] = "internal error";
    mock->injectError(5, "adapter.shutdown", error);

    /* sessionError should NOT be emitted during intentional shutdown */
    QCOMPARE(spyError.count(), 0);
}

void TestAdapterClient::awaitingConfigPausesBeforeConfigure()
{
    auto* mock = new MockAdapterProcess();
    AdapterClient client(mock);

    QSignalSpy spyDescribe(&client, &AdapterClient::describeResult);

    client.prepareAdapter(QStringLiteral("./dummy"));
    mock->injectResponse(1, "adapter.initialize", QJsonObject{ { "status", "ok" } });
    mock->injectResponse(2, "adapter.describe", describeResult());

    /* After describe, no configure should have been sent yet */
    QCOMPARE(spyDescribe.count(), 1);
    QCOMPARE(mock->sentRequests.size(), 2);
    QCOMPARE(mock->sentRequests[0].method, QStringLiteral("adapter.initialize"));
    QCOMPARE(mock->sentRequests[1].method, QStringLiteral("adapter.describe"));

    /* Now provide config → configure is sent */
    client.provideConfig(QJsonObject{ { "version", 1 } }, QStringList());
    QCOMPARE(mock->sentRequests.size(), 3);
    QCOMPARE(mock->sentRequests[2].method, QStringLiteral("adapter.configure"));
}

void TestAdapterClient::stopSessionDuringAwaitingConfig()
{
    auto* mock = new MockAdapterProcess();
    AdapterClient client(mock);

    QSignalSpy spyError(&client, &AdapterClient::sessionError);

    client.prepareAdapter(QStringLiteral("./dummy"));
    mock->injectResponse(1, "adapter.initialize", QJsonObject{ { "status", "ok" } });
    mock->injectResponse(2, "adapter.describe", describeResult());

    /* Stop during AWAITING_CONFIG — should go to IDLE without error */
    client.stopSession();

    QCOMPARE(spyError.count(), 0);

    /* provideConfig after stop should be silently ignored */
    client.provideConfig(QJsonObject(), QStringList());
    QCOMPARE(mock->sentRequests.size(), 2);
}

void TestAdapterClient::shutdownNoAckTimesOutToSessionStopped()
{
    auto* mock = new MockAdapterProcess();
    AdapterClient client(mock, nullptr, 50 /* ms */);

    QSignalSpy spyStopped(&client, &AdapterClient::sessionStopped);
    QSignalSpy spyError(&client, &AdapterClient::sessionError);

    /* Drive to ACTIVE state */
    client.prepareAdapter(QStringLiteral("./dummy"));
    mock->injectResponse(1, "adapter.initialize", QJsonObject{ { "status", "ok" } });
    mock->injectResponse(2, "adapter.describe", describeResult());
    client.provideConfig(QJsonObject(), QStringList());
    mock->injectResponse(3, "adapter.configure", QJsonObject{ { "status", "ok" } });
    mock->injectResponse(4, "adapter.start", QJsonObject{ { "status", "ok" } });

    /* Initiate shutdown — adapter never responds */
    client.stopSession();
    QCOMPARE(mock->sentRequests.last().method, QStringLiteral("adapter.shutdown"));

    /* Wait for the shutdown timer to fire */
    QTest::qWait(150);

    /* sessionStopped must be emitted, not sessionError */
    QCOMPARE(spyStopped.count(), 1);
    QCOMPARE(spyError.count(), 0);
}

QTEST_GUILESS_MAIN(TestAdapterClient)
