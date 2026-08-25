
#include "tst_registerdialog.h"

#include "../models/devicelisthelpers.h"
#include "dialogs/registerdialog.h"
#include "models/graphdatamodel.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QTest>

/*!
 * \brief Builds a minimal "modbus" data-point schema with non-empty defaults.
 *
 * RegisterDialog::requestDefaultExpression() bails out early when "defaults" is
 * empty, so the schema needs at least one default field for the flow under test
 * to reach buildExpression().
 */
QJsonObject TestRegisterDialog::buildTestRegisterSchema()
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

void TestRegisterDialog::init()
{
    DeviceListHelpers::clearDevices(&_settingsModel);
    _settingsModel.setAdapterDataPointSchema("modbus", buildTestRegisterSchema());

    _pGraphDataModel = new GraphDataModel();
    _pMockAdapterManager = new MockAdapterManager(&_settingsModel);
    _pMockHub = new MockAdapterHub();
    _pMockHub->addManager(QStringLiteral("modbus"), _pMockAdapterManager);
}

void TestRegisterDialog::cleanup()
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
void TestRegisterDialog::defaultRegisterUsesLowestConfiguredDeviceId()
{
    DeviceListHelpers::seedDevice(&_settingsModel, 2, QStringLiteral("modbus"));
    DeviceListHelpers::seedDevice(&_settingsModel, 5, QStringLiteral("modbus"));

    _pDialog = new RegisterDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    _pMockAdapterManager->injectSessionStarted();

    QCOMPARE(_pMockAdapterManager->buildCalls.count(), 1);
    QCOMPARE(_pMockAdapterManager->buildCalls.last().deviceId, deviceId_t(2));
}

//! Confirms the fix doesn't disturb the already-correct case where device 1 is present.
void TestRegisterDialog::defaultRegisterUsesDeviceOneWhenPresent()
{
    DeviceListHelpers::seedDevice(&_settingsModel, 1, QStringLiteral("modbus"));
    DeviceListHelpers::seedDevice(&_settingsModel, 3, QStringLiteral("modbus"));

    _pDialog = new RegisterDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    _pMockAdapterManager->injectSessionStarted();

    QCOMPARE(_pMockAdapterManager->buildCalls.count(), 1);
    QCOMPARE(_pMockAdapterManager->buildCalls.last().deviceId, deviceId_t(1));
}

//! With no devices configured, RegisterDialog must not wire up a default-expression manager at all.
void TestRegisterDialog::defaultRegisterSkipsBuildExpressionWhenNoDevices()
{
    _pDialog = new RegisterDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    _pMockAdapterManager->injectSessionStarted();

    QCOMPARE(_pMockAdapterManager->buildCalls.count(), 0);
}

/*!
 * \brief sessionStarted() fires only once per session. When the dialog is opened after the
 * adapter session is already active (the common case in practice), that emission is already
 * in the past, so RegisterDialog must request the default expression directly instead of
 * waiting for a signal that will never come again.
 */
void TestRegisterDialog::defaultRegisterRequestedImmediatelyWhenSessionAlreadyActive()
{
    DeviceListHelpers::seedDevice(&_settingsModel, 2, QStringLiteral("modbus"));
    _pMockAdapterManager->mockActive = true;

    _pDialog = new RegisterDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    QCOMPARE(_pMockAdapterManager->buildCalls.count(), 1);
    QCOMPARE(_pMockAdapterManager->buildCalls.last().deviceId, deviceId_t(2));
}

/*!
 * \brief AdapterClient::buildExpression() also accepts the AWAITING_CONFIG state (adapter
 * described, schema available, session not yet started) — RegisterDialog must not wait for
 * sessionStarted() in that state either, since the schema is already known.
 */
void TestRegisterDialog::defaultRegisterRequestedImmediatelyWhenAdapterAlreadyReady()
{
    DeviceListHelpers::seedDevice(&_settingsModel, 2, QStringLiteral("modbus"));
    _pMockAdapterManager->mockReady = true;

    _pDialog = new RegisterDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    QCOMPARE(_pMockAdapterManager->buildCalls.count(), 1);
    QCOMPARE(_pMockAdapterManager->buildCalls.last().deviceId, deviceId_t(2));
}

/*!
 * \brief The immediate request for an already-active session must not prevent a later, genuinely
 * new session from also triggering a request: requestDefaultExpression() disconnects any pending
 * buildExpressionResult connection before reconnecting, so the two requests don't interfere.
 */
void TestRegisterDialog::defaultRegisterRequestedAgainWhenSessionRestartsAfterAlreadyActive()
{
    DeviceListHelpers::seedDevice(&_settingsModel, 2, QStringLiteral("modbus"));
    _pMockAdapterManager->mockActive = true;

    _pDialog = new RegisterDialog(_pGraphDataModel, &_settingsModel, _pMockHub);
    QCOMPARE(_pMockAdapterManager->buildCalls.count(), 1);

    _pMockAdapterManager->injectSessionStarted();

    QCOMPARE(_pMockAdapterManager->buildCalls.count(), 2);
}

QTEST_MAIN(TestRegisterDialog)
