
#include "tst_signalsdialog.h"

#include "../models/devicelisthelpers.h"
#include "dialogs/signalsdialog.h"
#include "models/graphdatamodel.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QTest>

/*!
 * \brief Builds a minimal "modbus" data-point schema with non-empty defaults.
 *
 * SignalsDialog::requestDefaultExpression() bails out early when "defaults" is
 * empty, so the schema needs at least one default field for the flow under test
 * to reach buildExpression().
 */
QJsonObject TestSignalsDialog::buildTestRegisterSchema()
{
    QJsonObject addressField;
    addressField["type"] = QStringLiteral("integer");
    addressField["title"] = QStringLiteral("Address");

    QJsonObject properties;
    properties["address"] = addressField;

    QJsonObject addressSchema;
    addressSchema["type"] = QStringLiteral("object");
    addressSchema["properties"] = properties;

    QJsonObject defaults;
    defaults["address"] = 0;
    defaults["dataType"] = QStringLiteral("16b");

    QJsonObject schema;
    schema["addressSchema"] = addressSchema;
    schema["defaults"] = defaults;
    return schema;
}

void TestSignalsDialog::init()
{
    DeviceListHelpers::clearDevices(&_settingsModel);
    _settingsModel.setAdapterDataPointSchema("modbus", buildTestRegisterSchema());

    _pGraphDataModel = new GraphDataModel();
    _pMockAdapterManager = new MockAdapterManager(&_settingsModel);
    _pMockHub = new MockAdapterHub();
    _pMockHub->addManager(QStringLiteral("modbus"), _pMockAdapterManager);
}

void TestSignalsDialog::cleanup()
{
    delete _pDialog;
    _pDialog = nullptr;
    delete _pGraphDataModel;
    _pGraphDataModel = nullptr;
    delete _pMockAdapterManager;
    _pMockAdapterManager = nullptr;
    delete _pMockHub;
    _pMockHub = nullptr;
}

/*!
 * \brief Reproduces F3: after device 1 is deleted, the default register must target the
 * lowest remaining device id, not the buildExpression() sentinel that resolves to device 1.
 */
void TestSignalsDialog::defaultSignalUsesLowestConfiguredDeviceId()
{
    DeviceListHelpers::seedDevice(&_settingsModel, 2, QStringLiteral("modbus"));
    DeviceListHelpers::seedDevice(&_settingsModel, 5, QStringLiteral("modbus"));

    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    _pMockAdapterManager->injectSessionStarted();

    QCOMPARE(_pMockAdapterManager->buildCalls.count(), 1);
    QCOMPARE(_pMockAdapterManager->buildCalls.last().deviceId, deviceId_t(2));
}

//! Confirms the fix doesn't disturb the already-correct case where device 1 is present.
void TestSignalsDialog::defaultSignalUsesDeviceOneWhenPresent()
{
    DeviceListHelpers::seedDevice(&_settingsModel, 1, QStringLiteral("modbus"));
    DeviceListHelpers::seedDevice(&_settingsModel, 3, QStringLiteral("modbus"));

    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    _pMockAdapterManager->injectSessionStarted();

    QCOMPARE(_pMockAdapterManager->buildCalls.count(), 1);
    QCOMPARE(_pMockAdapterManager->buildCalls.last().deviceId, deviceId_t(1));
}

//! With no devices configured, SignalsDialog must not wire up a default-expression manager at all.
void TestSignalsDialog::defaultSignalSkipsBuildExpressionWhenNoDevices()
{
    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    _pMockAdapterManager->injectSessionStarted();

    QCOMPARE(_pMockAdapterManager->buildCalls.count(), 0);
}

/*!
 * \brief sessionStarted() fires only once per session. When the dialog is opened after the
 * adapter session is already active (the common case in practice), that emission is already
 * in the past, so SignalsDialog must request the default expression directly instead of
 * waiting for a signal that will never come again.
 */
void TestSignalsDialog::defaultSignalRequestedImmediatelyWhenSessionAlreadyActive()
{
    DeviceListHelpers::seedDevice(&_settingsModel, 2, QStringLiteral("modbus"));
    _pMockAdapterManager->mockActive = true;

    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    QCOMPARE(_pMockAdapterManager->buildCalls.count(), 1);
    QCOMPARE(_pMockAdapterManager->buildCalls.last().deviceId, deviceId_t(2));
}

/*!
 * \brief AdapterClient::buildExpression() also accepts the AWAITING_CONFIG state (adapter
 * described, schema available, session not yet started) — SignalsDialog must not wait for
 * sessionStarted() in that state either, since the schema is already known.
 */
void TestSignalsDialog::defaultSignalRequestedImmediatelyWhenAdapterAlreadyReady()
{
    DeviceListHelpers::seedDevice(&_settingsModel, 2, QStringLiteral("modbus"));
    _pMockAdapterManager->mockReady = true;

    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    QCOMPARE(_pMockAdapterManager->buildCalls.count(), 1);
    QCOMPARE(_pMockAdapterManager->buildCalls.last().deviceId, deviceId_t(2));
}

/*!
 * \brief The immediate request for an already-active session must not prevent a later, genuinely
 * new session from also triggering a request: requestDefaultExpression() disconnects any pending
 * buildExpressionResult connection before reconnecting, so the two requests don't interfere.
 */
void TestSignalsDialog::defaultSignalRequestedAgainWhenSessionRestartsAfterAlreadyActive()
{
    DeviceListHelpers::seedDevice(&_settingsModel, 2, QStringLiteral("modbus"));
    _pMockAdapterManager->mockActive = true;

    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);
    QCOMPARE(_pMockAdapterManager->buildCalls.count(), 1);

    _pMockAdapterManager->injectSessionStarted();

    QCOMPARE(_pMockAdapterManager->buildCalls.count(), 2);
}

/*!
 * \brief Builds a describe response reporting a data point limit in its capabilities.
 * \param maxRegisters The limit to report, or a negative value to leave the key out entirely
 *        (which is how the adapter reports that it enforces no limit).
 */
QJsonObject TestSignalsDialog::buildDescribeWithMaxRegisters(int maxRegisters)
{
    QJsonObject capabilities;
    if (maxRegisters >= 0)
    {
        capabilities["maxRegisters"] = maxRegisters;
    }

    QJsonObject describeResult;
    describeResult["name"] = QStringLiteral("modbus");
    describeResult["capabilities"] = capabilities;
    return describeResult;
}

//! Add \a count active signals, each reading its own data point from the default device.
void TestSignalsDialog::seedSignals(int count)
{
    for (int idx = 0; idx < count; idx++)
    {
        _pGraphDataModel->add();
        _pGraphDataModel->setExpression(GraphIdx(_pGraphDataModel->size() - 1), QString("${%1}").arg(40001 + idx));
    }
}

void TestSignalsDialog::dataPointLimitWarningHiddenWhenWithinLimit()
{
    DeviceListHelpers::seedDevice(&_settingsModel, 1, QStringLiteral("modbus"));
    _settingsModel.updateAdapterFromDescribe(QStringLiteral("modbus"), buildDescribeWithMaxRegisters(5));
    seedSignals(5);

    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    auto* warningLabel = _pDialog->findChild<QLabel*>("dataPointLimitWarningLabel");
    QVERIFY(warningLabel != nullptr);
    QVERIFY(warningLabel->isHidden());
}

void TestSignalsDialog::dataPointLimitWarningShownWhenExceeded()
{
    DeviceListHelpers::seedDevice(&_settingsModel, 1, QStringLiteral("modbus"));
    _settingsModel.updateAdapterFromDescribe(QStringLiteral("modbus"), buildDescribeWithMaxRegisters(5));
    seedSignals(6);

    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    auto* warningLabel = _pDialog->findChild<QLabel*>("dataPointLimitWarningLabel");
    QVERIFY(warningLabel != nullptr);
    QVERIFY(!warningLabel->isHidden());
    QVERIFY(warningLabel->text().contains("modbus"));
    QVERIFY(warningLabel->text().contains("5"));
    QVERIFY(warningLabel->text().contains("6"));
    QVERIFY(warningLabel->text().contains("Request License"));
}

/*!
 * \brief Deactivating a signal drops its data point from the session, so the warning must
 * disappear again while the dialog stays open.
 */
void TestSignalsDialog::dataPointLimitWarningHiddenWhenSignalDeactivated()
{
    DeviceListHelpers::seedDevice(&_settingsModel, 1, QStringLiteral("modbus"));
    _settingsModel.updateAdapterFromDescribe(QStringLiteral("modbus"), buildDescribeWithMaxRegisters(5));
    seedSignals(6);

    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    auto* warningLabel = _pDialog->findChild<QLabel*>("dataPointLimitWarningLabel");
    QVERIFY(warningLabel != nullptr);
    QVERIFY(!warningLabel->isHidden());

    _pGraphDataModel->setActive(GraphIdx(0), false);

    QVERIFY(warningLabel->isHidden());
}

//! A licensed adapter reports no maxRegisters at all, so no number of signals may warn.
void TestSignalsDialog::dataPointLimitWarningHiddenWhenAdapterReportsNoLimit()
{
    DeviceListHelpers::seedDevice(&_settingsModel, 1, QStringLiteral("modbus"));
    _settingsModel.updateAdapterFromDescribe(QStringLiteral("modbus"), buildDescribeWithMaxRegisters(-1));
    seedSignals(20);

    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    auto* warningLabel = _pDialog->findChild<QLabel*>("dataPointLimitWarningLabel");
    QVERIFY(warningLabel != nullptr);
    QVERIFY(warningLabel->isHidden());
}

QTEST_MAIN(TestSignalsDialog)
