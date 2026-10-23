#include "tst_datapointservice.h"

#include "../mocks/mockadapterhub.h"
#include "../models/devicelisthelpers.h"
#include "ProtocolAdapter/datapointservice.h"

#include <QJsonArray>
#include <QSignalSpy>
#include <QTest>

namespace {

constexpr deviceId_t cModbusDeviceId = 1;
constexpr deviceId_t cSimDeviceId = 2;
constexpr int cShortTimeoutMs = 50;

} // namespace

/*!
 * \brief Builds a data point schema whose address schema has its own deviceId property.
 */
QJsonObject TestDataPointService::buildSchemaWithDeviceId()
{
    QJsonObject properties;
    properties["address"] = QJsonObject{ { "type", "integer" } };
    properties["deviceId"] = QJsonObject{ { "type", "integer" } };

    QJsonObject addressSchema;
    addressSchema["type"] = QStringLiteral("object");
    addressSchema["properties"] = properties;
    addressSchema["required"] = QJsonArray{ QStringLiteral("address"), QStringLiteral("deviceId") };

    QJsonObject schema;
    schema["addressSchema"] = addressSchema;
    schema["defaults"] = QJsonObject{ { "address", 0 }, { "dataType", "16b" } };
    return schema;
}

void TestDataPointService::init()
{
    _pSettingsModel = new SettingsModel();
    DeviceListHelpers::clearDevices(_pSettingsModel);
    DeviceListHelpers::seedDevice(_pSettingsModel, cModbusDeviceId, QStringLiteral("modbus"));
    DeviceListHelpers::seedDevice(_pSettingsModel, cSimDeviceId, QStringLiteral("sim"));
    _pSettingsModel->setAdapterDataPointSchema(QStringLiteral("modbus"), buildSchemaWithDeviceId());

    _pModbusManager = new MockDataPointManager(QStringLiteral("modbus"));
    _pSimManager = new MockDataPointManager(QStringLiteral("sim"));
    _pMockHub = new MockAdapterHub();
    _pMockHub->addManager(QStringLiteral("modbus"), _pModbusManager);
    _pMockHub->addManager(QStringLiteral("sim"), _pSimManager);

    _pService = new DataPointService(_pSettingsModel, _pMockHub, nullptr, cShortTimeoutMs);
}

void TestDataPointService::cleanup()
{
    delete _pService;
    _pService = nullptr;
    delete _pMockHub;
    _pMockHub = nullptr;
    delete _pModbusManager;
    _pModbusManager = nullptr;
    delete _pSimManager;
    _pSimManager = nullptr;
    delete _pSettingsModel;
    _pSettingsModel = nullptr;
}

void TestDataPointService::requestsRoutedToDeviceAdapter()
{
    _pService->describeDataPoint(cModbusDeviceId, QStringLiteral("${40001}"));
    _pService->describeDataPoint(cSimDeviceId, QStringLiteral("${1@2}"));

    QCOMPARE(_pModbusManager->describeCalls, QStringList{ QStringLiteral("${40001}") });
    QCOMPARE(_pSimManager->describeCalls, QStringList{ QStringLiteral("${1@2}") });
}

void TestDataPointService::describeResultCarriesTicket()
{
    QSignalSpy spyDescribed(_pService, &DataPointService::dataPointDescribed);

    const quint64 ticket = _pService->describeDataPoint(cModbusDeviceId, QStringLiteral("${40001}"));
    emit _pModbusManager->describeDataPointResult(1, QJsonObject{ { "valid", true } });

    QCOMPARE(spyDescribed.count(), 1);
    QCOMPARE(spyDescribed.at(0).at(0).value<quint64>(), ticket);
    QVERIFY(spyDescribed.at(0).at(1).value<QJsonObject>().value("valid").toBool());
}

void TestDataPointService::interleavedRepliesMatchTheirTickets()
{
    QSignalSpy spyBuilt(_pService, &DataPointService::expressionBuilt);

    const quint64 firstTicket = _pService->buildExpression(cModbusDeviceId, QJsonObject{ { "address", 0 } });
    const quint64 secondTicket = _pService->buildExpression(cModbusDeviceId, QJsonObject{ { "address", 1 } });

    emit _pModbusManager->buildExpressionResult(2, QStringLiteral("${h1}"));
    emit _pModbusManager->buildExpressionResult(1, QStringLiteral("${h0}"));

    QCOMPARE(spyBuilt.count(), 2);
    QCOMPARE(spyBuilt.at(0).at(0).value<quint64>(), secondTicket);
    QCOMPARE(spyBuilt.at(0).at(1).toString(), QStringLiteral("${h1}"));
    QCOMPARE(spyBuilt.at(1).at(0).value<quint64>(), firstTicket);
    QCOMPARE(spyBuilt.at(1).at(1).toString(), QStringLiteral("${h0}"));
}

//! Each adapter process numbers its requests on its own, so request ids repeat across adapters.
void TestDataPointService::sameRequestIdOnTwoAdaptersKeptApart()
{
    QSignalSpy spyBuilt(_pService, &DataPointService::expressionBuilt);

    _pService->buildExpression(cModbusDeviceId, QJsonObject());
    const quint64 simTicket = _pService->buildExpression(cSimDeviceId, QJsonObject());

    emit _pSimManager->buildExpressionResult(1, QStringLiteral("${s1}"));

    QCOMPARE(spyBuilt.count(), 1);
    QCOMPARE(spyBuilt.at(0).at(0).value<quint64>(), simTicket);
}

void TestDataPointService::unknownDeviceFailsOnNextEventLoopTurn()
{
    QSignalSpy spyFailed(_pService, &DataPointService::requestFailed);

    const quint64 ticket = _pService->describeDataPoint(99, QStringLiteral("${40001@99}"));
    QCOMPARE(spyFailed.count(), 0);

    QVERIFY(spyFailed.wait(1000));
    QCOMPARE(spyFailed.count(), 1);
    QCOMPARE(spyFailed.at(0).at(0).value<quint64>(), ticket);
    QVERIFY(_pModbusManager->describeCalls.isEmpty());
    QVERIFY(!_pService->isDeviceUsable(99));
}

void TestDataPointService::adapterNotRunningFails()
{
    _pModbusManager->mockReady = false;
    QSignalSpy spyFailed(_pService, &DataPointService::requestFailed);

    QVERIFY(!_pService->isDeviceUsable(cModbusDeviceId));
    const quint64 ticket = _pService->buildExpression(cModbusDeviceId, QJsonObject());

    QVERIFY(spyFailed.wait(1000));
    QCOMPARE(spyFailed.at(0).at(0).value<quint64>(), ticket);
    QCOMPARE(spyFailed.at(0).at(1).toString(), QStringLiteral("The adapter is not running"));
    QVERIFY(_pModbusManager->buildCalls.isEmpty());
}

void TestDataPointService::refusedRequestFails()
{
    _pModbusManager->refuseRequests = true;
    QSignalSpy spyFailed(_pService, &DataPointService::requestFailed);

    const quint64 ticket = _pService->describeDataPoint(cModbusDeviceId, QStringLiteral("${40001}"));

    QVERIFY(spyFailed.wait(1000));
    QCOMPARE(spyFailed.at(0).at(0).value<quint64>(), ticket);
}

void TestDataPointService::adapterErrorFails()
{
    QSignalSpy spyFailed(_pService, &DataPointService::requestFailed);

    const quint64 ticket = _pService->buildExpression(cModbusDeviceId, QJsonObject{ { "address", 70000 } });
    emit _pModbusManager->dataPointRequestFailed(1, QStringLiteral("Address out of range"));

    /* Delivered on the next event loop turn, never from inside the adapter's reply handling */
    QCOMPARE(spyFailed.count(), 0);
    QVERIFY(spyFailed.wait(1000));
    QCOMPARE(spyFailed.count(), 1);
    QCOMPARE(spyFailed.at(0).at(0).value<quint64>(), ticket);
    QCOMPARE(spyFailed.at(0).at(1).toString(), QStringLiteral("Address out of range"));
}

void TestDataPointService::failedTicketNotFailedAgainByTimeout()
{
    QSignalSpy spyFailed(_pService, &DataPointService::requestFailed);

    _pService->buildExpression(cModbusDeviceId, QJsonObject());
    emit _pModbusManager->dataPointRequestFailed(1, QStringLiteral("Address out of range"));
    QVERIFY(spyFailed.wait(1000));

    /* The timeout of the ended ticket expires without a second failure */
    QTest::qWait(cShortTimeoutMs * 2);
    QCOMPARE(spyFailed.count(), 1);
}

void TestDataPointService::queuedFailuresKeepTicketOrder()
{
    QSignalSpy spyFailed(_pService, &DataPointService::requestFailed);

    const quint64 firstTicket = _pService->describeDataPoint(97, QStringLiteral("${1@97}"));
    const quint64 secondTicket = _pService->describeDataPoint(98, QStringLiteral("${1@98}"));
    const quint64 thirdTicket = _pService->describeDataPoint(99, QStringLiteral("${1@99}"));

    QVERIFY(spyFailed.wait(1000));
    QCOMPARE(spyFailed.count(), 3);
    QCOMPARE(spyFailed.at(0).at(0).value<quint64>(), firstTicket);
    QCOMPARE(spyFailed.at(1).at(0).value<quint64>(), secondTicket);
    QCOMPARE(spyFailed.at(2).at(0).value<quint64>(), thirdTicket);
}

void TestDataPointService::timeoutFails()
{
    QSignalSpy spyFailed(_pService, &DataPointService::requestFailed);

    const quint64 ticket = _pService->describeDataPoint(cModbusDeviceId, QStringLiteral("${40001}"));

    QVERIFY(spyFailed.wait(1000));
    QCOMPARE(spyFailed.count(), 1);
    QCOMPARE(spyFailed.at(0).at(0).value<quint64>(), ticket);
    QCOMPARE(spyFailed.at(0).at(1).toString(), QStringLiteral("The adapter did not respond"));
}

void TestDataPointService::replyAfterTimeoutIgnored()
{
    QSignalSpy spyFailed(_pService, &DataPointService::requestFailed);
    QSignalSpy spyDescribed(_pService, &DataPointService::dataPointDescribed);

    _pService->describeDataPoint(cModbusDeviceId, QStringLiteral("${40001}"));
    QVERIFY(spyFailed.wait(1000));

    emit _pModbusManager->describeDataPointResult(1, QJsonObject{ { "valid", true } });

    QCOMPARE(spyDescribed.count(), 0);
    QCOMPARE(spyFailed.count(), 1);
}

void TestDataPointService::destroyedManagerFailsPendingRequests()
{
    QSignalSpy spyFailed(_pService, &DataPointService::requestFailed);

    const quint64 ticket = _pService->buildExpression(cSimDeviceId, QJsonObject());
    _pMockHub->addManager(QStringLiteral("sim"), nullptr);
    delete _pSimManager;
    _pSimManager = nullptr;

    QVERIFY(spyFailed.wait(cShortTimeoutMs / 2));
    QCOMPARE(spyFailed.at(0).at(0).value<quint64>(), ticket);
    QCOMPARE(spyFailed.at(0).at(1).toString(), QStringLiteral("The adapter stopped"));
}

/*!
 * \brief The fields of a describeDataPoint result can be fed back as they are: the data type goes into
 * its own parameter and the device always comes from the device argument.
 */
void TestDataPointService::buildExpressionSplitsFields()
{
    QJsonObject fields;
    fields["objectType"] = QStringLiteral("holding register");
    fields["address"] = 1;
    fields["dataType"] = QStringLiteral("s16b");
    fields["deviceId"] = 7;

    _pService->buildExpression(cModbusDeviceId, fields);

    QCOMPARE(_pModbusManager->buildCalls.size(), 1);
    const MockDataPointManager::BuildCall call = _pModbusManager->buildCalls.first();
    QCOMPARE(call.dataType, QStringLiteral("s16b"));
    QCOMPARE(call.deviceId, cModbusDeviceId);
    QVERIFY(!call.fields.contains(QStringLiteral("dataType")));
    QVERIFY(!call.fields.contains(QStringLiteral("deviceId")));
    QCOMPARE(call.fields["objectType"].toString(), QStringLiteral("holding register"));
    QCOMPARE(call.fields["address"].toInt(), 1);
}

void TestDataPointService::addressSchemaStripsDeviceId()
{
    const QJsonObject schema = _pService->addressSchema(cModbusDeviceId);

    QVERIFY(schema["properties"].toObject().contains(QStringLiteral("address")));
    QVERIFY(!schema["properties"].toObject().contains(QStringLiteral("deviceId")));
    QCOMPARE(schema["required"].toArray(), QJsonArray{ QStringLiteral("address") });
}

void TestDataPointService::defaultsReturned()
{
    const QJsonObject defaults = _pService->defaults(cModbusDeviceId);

    QCOMPARE(defaults["address"].toInt(), 0);
    QCOMPARE(defaults["dataType"].toString(), QStringLiteral("16b"));
}

void TestDataPointService::unknownAdapterAddsNoEntry()
{
    DeviceListHelpers::seedDevice(_pSettingsModel, 3, QStringLiteral("ghost"));

    QVERIFY(_pService->addressSchema(3).isEmpty());
    QVERIFY(_pService->defaults(3).isEmpty());
    QVERIFY(!_pSettingsModel->adapterIds().contains(QStringLiteral("ghost")));
}

void TestDataPointService::hubReadySignalsEmitAdaptersChanged()
{
    QSignalSpy spy(_pService, &DataPointService::adaptersChanged);

    emit _pMockHub->adapterReady();
    emit _pMockHub->sessionStarted();

    QCOMPARE(spy.count(), 2);
}

QTEST_GUILESS_MAIN(TestDataPointService)
