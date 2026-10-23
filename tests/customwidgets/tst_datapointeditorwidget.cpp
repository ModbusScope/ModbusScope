#include "tst_datapointeditorwidget.h"

#include "../mocks/mockadapterhub.h"
#include "../mocks/mockdatapointmanager.h"
#include "../models/devicelisthelpers.h"
#include "ProtocolAdapter/datapointservice.h"
#include "customwidgets/datapointeditorwidget.h"
#include "customwidgets/devicecombobox.h"
#include "customwidgets/schemaformwidget.h"

#include <QJsonArray>
#include <QLabel>
#include <QSignalSpy>
#include <QTest>

namespace {

constexpr deviceId_t cSimDeviceId = 2;
constexpr int cShortTimeoutMs = 50;

} // namespace

QJsonObject TestDataPointEditorWidget::modbusSchema()
{
    QJsonObject properties;
    properties["objectType"] =
      QJsonObject{ { "type", "string" },
                   { "enum", QJsonArray{ QStringLiteral("holding register"), QStringLiteral("input register") } } };
    properties["address"] = QJsonObject{ { "type", "integer" }, { "minimum", 0 }, { "maximum", 65535 } };
    properties["dataType"] =
      QJsonObject{ { "type", "string" }, { "enum", QJsonArray{ QStringLiteral("16b"), QStringLiteral("s16b") } } };

    QJsonObject addressSchema;
    addressSchema["type"] = QStringLiteral("object");
    addressSchema["properties"] = properties;

    QJsonObject schema;
    schema["addressSchema"] = addressSchema;
    schema["defaults"] = QJsonObject{ { "objectType", "holding register" }, { "address", 0 }, { "dataType", "16b" } };
    return schema;
}

QJsonObject TestDataPointEditorWidget::simSchema()
{
    QJsonObject addressSchema;
    addressSchema["type"] = QStringLiteral("object");
    addressSchema["properties"] = QJsonObject{ { "channel", QJsonObject{ { "type", "integer" } } } };

    QJsonObject schema;
    schema["addressSchema"] = addressSchema;
    schema["defaults"] = QJsonObject{ { "channel", 3 } };
    return schema;
}

void TestDataPointEditorWidget::init()
{
    _pSettingsModel = new SettingsModel();
    DeviceListHelpers::clearDevices(_pSettingsModel);
    DeviceListHelpers::seedDevice(_pSettingsModel, Device::cFirstDeviceId, QStringLiteral("modbus"));
    DeviceListHelpers::seedDevice(_pSettingsModel, cSimDeviceId, QStringLiteral("sim"));
    _pSettingsModel->setAdapterDataPointSchema(QStringLiteral("modbus"), modbusSchema());
    _pSettingsModel->setAdapterDataPointSchema(QStringLiteral("sim"), simSchema());

    _pModbusManager = new MockDataPointManager(QStringLiteral("modbus"));
    _pSimManager = new MockDataPointManager(QStringLiteral("sim"));
    _pMockHub = new MockAdapterHub();
    _pMockHub->addManager(QStringLiteral("modbus"), _pModbusManager);
    _pMockHub->addManager(QStringLiteral("sim"), _pSimManager);

    _pService = new DataPointService(_pSettingsModel, _pMockHub, nullptr, cShortTimeoutMs);
}

void TestDataPointEditorWidget::cleanup()
{
    delete _pEditor;
    _pEditor = nullptr;
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

void TestDataPointEditorWidget::buildsDefaultReferenceOnOpen()
{
    createEditor();

    QCOMPARE(_pModbusManager->buildCalls.size(), 1);
    QCOMPARE(_pModbusManager->buildCalls[0].fields["objectType"].toString(), QStringLiteral("holding register"));
    QCOMPARE(_pModbusManager->buildCalls[0].dataType, QStringLiteral("16b"));
    QVERIFY(!_pEditor->hasValidExpression());
}

void TestDataPointEditorWidget::builtReferenceIsValid()
{
    createEditor();
    QSignalSpy spy(_pEditor, &DataPointEditorWidget::expressionChanged);

    emit _pModbusManager->buildExpressionResult(1, QStringLiteral("${h0}"));

    QVERIFY(_pEditor->hasValidExpression());
    QCOMPARE(_pEditor->expression(), QStringLiteral("${h0}"));
    QCOMPARE(spy.count(), 1);
}

void TestDataPointEditorWidget::fieldChangeBuildsReference()
{
    createEditor();
    emit _pModbusManager->buildExpressionResult(1, QStringLiteral("${h0}"));

    editField();

    QCOMPARE(_pModbusManager->buildCalls.size(), 2);
    QCOMPARE(_pModbusManager->buildCalls[1].fields["address"].toInt(), 7);
    QVERIFY(!_pEditor->hasValidExpression());

    /* The last built reference is kept while the new one is built */
    QCOMPARE(_pEditor->expression(), QStringLiteral("${h0}"));

    emit _pModbusManager->buildExpressionResult(2, QStringLiteral("${h7}"));
    QCOMPARE(_pEditor->expression(), QStringLiteral("${h7}"));
    QVERIFY(_pEditor->hasValidExpression());
}

//! A change notification without a changed value (a spin box emits one on Enter) keeps the built reference.
void TestDataPointEditorWidget::unchangedValuesKeepBuiltReference()
{
    createEditor();
    emit _pModbusManager->buildExpressionResult(1, QStringLiteral("${h0}"));

    emit _pEditor->_pAddressForm->valuesChanged();

    QCOMPARE(_pModbusManager->buildCalls.size(), 1);
    QVERIFY(_pEditor->hasValidExpression());
    QCOMPARE(_pEditor->expression(), QStringLiteral("${h0}"));
}

//! A change notification without a changed value keeps a reference set with setReference().
void TestDataPointEditorWidget::unchangedValuesKeepSetReference()
{
    createEditor();
    emit _pModbusManager->buildExpressionResult(1, QStringLiteral("${h0}"));
    _pEditor->setReference(
      QStringLiteral("${40003:s16b}"),
      QJsonObject{ { "deviceId", 1 }, { "objectType", "holding register" }, { "address", 2 }, { "dataType", "s16b" } });

    emit _pEditor->_pAddressForm->valuesChanged();

    QCOMPARE(_pModbusManager->buildCalls.size(), 1);
    QCOMPARE(_pEditor->expression(), QStringLiteral("${40003:s16b}"));
}

void TestDataPointEditorWidget::staleReplyIgnored()
{
    createEditor();
    editField();

    emit _pModbusManager->buildExpressionResult(1, QStringLiteral("${h0}"));
    QVERIFY(!_pEditor->hasValidExpression());
    QCOMPARE(_pEditor->expression(), QString());

    emit _pModbusManager->buildExpressionResult(2, QStringLiteral("${h7}"));
    QCOMPARE(_pEditor->expression(), QStringLiteral("${h7}"));
}

void TestDataPointEditorWidget::deviceSwitchRebuildsForm()
{
    createEditor();
    QSignalSpy spy(_pEditor, &DataPointEditorWidget::deviceChanged);

    _pEditor->setDevice(cSimDeviceId);

    QCOMPARE(_pEditor->deviceId(), cSimDeviceId);
    QCOMPARE(_pEditor->_pAddressForm->values().value("channel").toInt(), 3);
    QCOMPARE(_pSimManager->buildCalls.size(), 1);
    QCOMPARE(_pSimManager->buildCalls[0].deviceId, cSimDeviceId);
    QCOMPARE(spy.count(), 1);
}

void TestDataPointEditorWidget::setReferenceKeepsTextUntilFieldChange()
{
    createEditor();
    emit _pModbusManager->buildExpressionResult(1, QStringLiteral("${h0}"));

    _pEditor->setReference(
      QStringLiteral("${40003:s16b}"),
      QJsonObject{ { "deviceId", 1 }, { "objectType", "holding register" }, { "address", 2 }, { "dataType", "s16b" } });

    QCOMPARE(_pEditor->expression(), QStringLiteral("${40003:s16b}"));
    QVERIFY(_pEditor->hasValidExpression());
    QCOMPARE(_pModbusManager->buildCalls.size(), 1);
    QCOMPARE(_pEditor->_pAddressForm->values().value("address").toInt(), 2);
    QCOMPARE(_pEditor->_pAddressForm->values().value("dataType").toString(), QStringLiteral("s16b"));

    editField();
    QCOMPARE(_pModbusManager->buildCalls.size(), 2);
    QVERIFY(!_pEditor->hasValidExpression());

    emit _pModbusManager->buildExpressionResult(2, QStringLiteral("${h7:s16b}"));
    QCOMPARE(_pEditor->expression(), QStringLiteral("${h7:s16b}"));
}

void TestDataPointEditorWidget::setReferenceIgnoresUnknownFieldsAndFillsDefaults()
{
    createEditor();

    _pEditor->setReference(QStringLiteral("${40008}"),
                           QJsonObject{ { "deviceId", 1 }, { "address", 7 }, { "bit", 3 } });

    const QJsonObject values = _pEditor->_pAddressForm->values();
    QCOMPARE(values.value("address").toInt(), 7);
    QCOMPARE(values.value("objectType").toString(), QStringLiteral("holding register"));
    QVERIFY(!values.contains(QStringLiteral("bit")));
}

void TestDataPointEditorWidget::setReferenceSelectsDevice()
{
    createEditor();

    _pEditor->setReference(QStringLiteral("${5@2}"), QJsonObject{ { "deviceId", 2 }, { "channel", 5 } });

    QCOMPARE(_pEditor->deviceId(), cSimDeviceId);
    QCOMPARE(_pEditor->_pAddressForm->values().value("channel").toInt(), 5);
    QCOMPARE(_pEditor->expression(), QStringLiteral("${5@2}"));
    QVERIFY(_pSimManager->buildCalls.isEmpty());
}

void TestDataPointEditorWidget::adapterNotRunningShowsError()
{
    _pSimManager->mockReady = false;
    createEditor();
    QSignalSpy spy(_pEditor, &DataPointEditorWidget::errorChanged);

    _pEditor->setDevice(cSimDeviceId);

    QTRY_COMPARE(spy.count(), 1);
    QVERIFY(spy.at(0).at(0).toString().startsWith(QStringLiteral("The adapter is not running")));
    QVERIFY(!_pEditor->hasValidExpression());
    QVERIFY(_pSimManager->buildCalls.isEmpty());
}

void TestDataPointEditorWidget::buildFailureShowsError()
{
    createEditor();
    emit _pModbusManager->buildExpressionResult(1, QStringLiteral("${h0}"));
    editField();
    QSignalSpy spy(_pEditor, &DataPointEditorWidget::errorChanged);

    emit _pModbusManager->dataPointRequestFailed(2, QStringLiteral("Address out of range"));

    QTRY_COMPARE(spy.count(), 1);
    const QString error = spy.at(0).at(0).toString();
    QVERIFY(error.startsWith(QStringLiteral("Address out of range")));
    QVERIFY(error.contains(QStringLiteral("retry")));
    QVERIFY(!_pEditor->hasValidExpression());
    QCOMPARE(_pEditor->expression(), QString());
}

void TestDataPointEditorWidget::emptyBuildResultShowsError()
{
    createEditor();
    QSignalSpy spy(_pEditor, &DataPointEditorWidget::errorChanged);

    emit _pModbusManager->buildExpressionResult(1, QString());

    QCOMPARE(spy.count(), 1);
    QVERIFY(!spy.at(0).at(0).toString().isEmpty());
    QVERIFY(!_pEditor->hasValidExpression());
}

void TestDataPointEditorWidget::timeoutShowsError()
{
    createEditor();
    QSignalSpy spy(_pEditor, &DataPointEditorWidget::errorChanged);

    QTRY_COMPARE(spy.count(), 1);
    QVERIFY(spy.at(0).at(0).toString().startsWith(QStringLiteral("The adapter did not respond")));
    QVERIFY(!_pEditor->hasValidExpression());
}

void TestDataPointEditorWidget::fieldChangeAfterFailureSendsNewRequest()
{
    createEditor();
    QSignalSpy spy(_pEditor, &DataPointEditorWidget::errorChanged);
    emit _pModbusManager->dataPointRequestFailed(1, QStringLiteral("Address out of range"));
    QTRY_COMPARE(spy.count(), 1);

    editField();

    QCOMPARE(_pModbusManager->buildCalls.size(), 2);
    /* The error of the previous request is cleared */
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).at(0).toString(), QString());
}

void TestDataPointEditorWidget::noDevicesHasNoExpression()
{
    DeviceListHelpers::clearDevices(_pSettingsModel);
    createEditor();

    QCOMPARE(_pEditor->deviceId(), deviceId_t(0));
    QVERIFY(!_pEditor->hasValidExpression());
    QVERIFY(_pModbusManager->buildCalls.isEmpty());
}

void TestDataPointEditorWidget::deviceSwitchClearsPreviousReference()
{
    createEditor();
    emit _pModbusManager->buildExpressionResult(1, QStringLiteral("${h0}"));

    _pEditor->setDevice(cSimDeviceId);

    QCOMPARE(_pEditor->expression(), QString());
    QVERIFY(!_pEditor->hasValidExpression());
}

void TestDataPointEditorWidget::setReferenceWithUnknownDeviceIsInvalid()
{
    createEditor();
    QSignalSpy spy(_pEditor, &DataPointEditorWidget::errorChanged);

    _pEditor->setReference(QStringLiteral("${1@9}"), QJsonObject{ { "deviceId", 9 }, { "address", 1 } });

    QVERIFY(!_pEditor->hasValidExpression());
    QCOMPARE(spy.count(), 1);
    QVERIFY(!spy.at(0).at(0).toString().isEmpty());
}

//! An editor opened before the adapter was usable builds once the adapter becomes usable.
void TestDataPointEditorWidget::retriedWhenAdapterBecomesUsable()
{
    _pModbusManager->mockReady = false;
    createEditor();
    QTest::qWait(0); /* deliver the queued failure */

    _pModbusManager->mockReady = true;
    emit _pMockHub->adapterReady();

    QCOMPARE(_pModbusManager->buildCalls.size(), 1);
    emit _pModbusManager->buildExpressionResult(1, QStringLiteral("${h0}"));
    QVERIFY(_pEditor->hasValidExpression());
}

//! A build the adapter rejected is not reset when adapters change, so the user's fields are kept.
void TestDataPointEditorWidget::rejectedBuildKeptWhenAdaptersChange()
{
    createEditor();
    editField();
    emit _pModbusManager->dataPointRequestFailed(2, QStringLiteral("Address out of range"));
    QTest::qWait(0);

    emit _pMockHub->adapterReady();

    QCOMPARE(_pModbusManager->buildCalls.size(), 2);
    QCOMPARE(_pEditor->_pAddressForm->values().value("address").toInt(), 7);
}

void TestDataPointEditorWidget::createEditor()
{
    _pEditor = new DataPointEditorWidget(_pSettingsModel, _pService);
}

//! Change the address field as the user would, which sends a new build request.
void TestDataPointEditorWidget::editField()
{
    QJsonObject values = _pEditor->_pAddressForm->values();
    values["address"] = 7;
    _pEditor->_pAddressForm->setSchema(_pService->addressSchema(_pEditor->deviceId()), values);
    emit _pEditor->_pAddressForm->valuesChanged();
}

QTEST_MAIN(TestDataPointEditorWidget)
