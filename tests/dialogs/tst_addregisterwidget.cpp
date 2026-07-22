
#include "tst_addregisterwidget.h"

#include "customwidgets/schemaformwidget.h"
#include "dialogs/addregisterwidget.h"
#include "models/device.h"
#include "ui_addregisterwidget.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTest>

QJsonObject TestAddRegisterWidget::buildAddressSchema()
{
    QJsonObject objectTypeSchema;
    objectTypeSchema["type"] = QStringLiteral("string");
    objectTypeSchema["title"] = QStringLiteral("Object type");
    objectTypeSchema["enum"] = QJsonArray{ QStringLiteral("coil"), QStringLiteral("discrete input"),
                                           QStringLiteral("input register"), QStringLiteral("holding register") };
    objectTypeSchema["x-enumLabels"] =
      QJsonArray{ QStringLiteral("Coil"), QStringLiteral("Discrete Input"), QStringLiteral("Input Register"),
                  QStringLiteral("Holding Register") };

    QJsonObject addressField;
    addressField["type"] = QStringLiteral("integer");
    addressField["title"] = QStringLiteral("Address");
    addressField["minimum"] = 0;
    addressField["maximum"] = 65535;

    QJsonObject dataTypeSchema;
    dataTypeSchema["type"] = QStringLiteral("string");
    dataTypeSchema["title"] = QStringLiteral("Data type");
    dataTypeSchema["enum"] = QJsonArray{ QStringLiteral("16b"), QStringLiteral("s16b"), QStringLiteral("32b"),
                                         QStringLiteral("s32b"), QStringLiteral("f32b") };
    dataTypeSchema["x-enumLabels"] =
      QJsonArray{ QStringLiteral("unsigned 16-bit"), QStringLiteral("signed 16-bit"), QStringLiteral("unsigned 32-bit"),
                  QStringLiteral("signed 32-bit"), QStringLiteral("32-bit float") };
    dataTypeSchema["default"] = QStringLiteral("16b");

    QJsonObject deviceIdSchema;
    deviceIdSchema["type"] = QStringLiteral("integer");
    deviceIdSchema["title"] = QStringLiteral("Device ID");
    deviceIdSchema["minimum"] = 1;
    /* Tests inject enum values directly via setSchema; the constructor patches these from SettingsModel */
    deviceIdSchema["enum"] = QJsonArray{ 1, 2 };
    deviceIdSchema["x-enumLabels"] = QJsonArray{ QStringLiteral("Device 1"), QStringLiteral("Device 2") };

    QJsonObject properties;
    properties["objectType"] = objectTypeSchema;
    properties["address"] = addressField;
    properties["dataType"] = dataTypeSchema;
    properties["deviceId"] = deviceIdSchema;

    QJsonObject schema;
    schema["type"] = QStringLiteral("object");
    schema["properties"] = properties;
    schema["required"] = QJsonArray{ QStringLiteral("objectType"), QStringLiteral("address") };
    return schema;
}

QJsonObject TestAddRegisterWidget::buildTestRegisterSchema()
{
    QJsonObject defaults;
    defaults["objectType"] = QStringLiteral("holding register");
    defaults["address"] = 0;
    defaults["dataType"] = QStringLiteral("16b");

    QJsonObject schema;
    schema["addressSchema"] = buildAddressSchema();
    schema["defaults"] = defaults;
    return schema;
}

QJsonObject TestAddRegisterWidget::buildSimRegisterSchema()
{
    QJsonObject channelField;
    channelField["type"] = QStringLiteral("integer");
    channelField["title"] = QStringLiteral("Channel");
    channelField["minimum"] = 0;

    QJsonObject properties;
    properties["channel"] = channelField;

    QJsonObject addressSchema;
    addressSchema["type"] = QStringLiteral("object");
    addressSchema["properties"] = properties;

    QJsonObject defaults;
    defaults["channel"] = 3;

    QJsonObject schema;
    schema["addressSchema"] = addressSchema;
    schema["defaults"] = defaults;
    return schema;
}

void TestAddRegisterWidget::init()
{
    _settingsModel.removeAllDevice();
    /* Drop the sim adapter data a previous multi-adapter test may have registered */
    _settingsModel.removeAdapter(QStringLiteral("sim"));
    _settingsModel.setAdapterDataPointSchema("modbus", buildTestRegisterSchema());
    _settingsModel.deviceSettings(Device::cFirstDeviceId)->setAdapterId("modbus");

    _pMockAdapterManager = new MockAdapterManager(&_settingsModel);
    _pMockHub = new MockAdapterHub();
    _pMockHub->addManager(QStringLiteral("modbus"), _pMockAdapterManager);
    _pRegWidget = new AddRegisterWidget(&_settingsModel, _pMockHub);
}

void TestAddRegisterWidget::cleanup()
{
    delete _pRegWidget;
    _pRegWidget = nullptr;
    delete _pMockAdapterManager;
    _pMockAdapterManager = nullptr;
    delete _pMockSimAdapterManager;
    _pMockSimAdapterManager = nullptr;
    delete _pMockHub;
    _pMockHub = nullptr;
}

/*!
 * \brief Register a second ("sim") adapter and rebuild the widget with both adapters.
 */
void TestAddRegisterWidget::addSimAdapter()
{
    _settingsModel.setAdapterDataPointSchema("sim", buildSimRegisterSchema());

    QJsonObject describeResult;
    describeResult["name"] = QStringLiteral("Simulator");
    _settingsModel.updateAdapterFromDescribe(QStringLiteral("sim"), describeResult);

    _pMockSimAdapterManager = new MockAdapterManager(&_settingsModel, QStringLiteral("sim"));
    _pMockHub->addManager(QStringLiteral("sim"), _pMockSimAdapterManager);

    delete _pRegWidget;
    _pRegWidget = new AddRegisterWidget(&_settingsModel, _pMockHub);
}

void TestAddRegisterWidget::registerDefault()
{
    _pRegWidget->_pUi->lineName->selectAll();
    QTest::keyClicks(_pRegWidget->_pUi->lineName, "Register 1");

    _pRegWidget->_pAddressForm->setSchema(
      buildAddressSchema(), QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding register") },
                                         { QStringLiteral("address"), 100 },
                                         { QStringLiteral("dataType"), QStringLiteral("16b") },
                                         { QStringLiteral("deviceId"), 1 } });

    GraphData graphData;
    addRegister(graphData, QStringLiteral("${h100}"));

    QCOMPARE(graphData.label(), QStringLiteral("Register 1"));
    QCOMPARE(graphData.expression(), QStringLiteral("${h100}"));
    QVERIFY(graphData.isActive());

    QCOMPARE(_pMockAdapterManager->buildCalls.size(), 1);
    QCOMPARE(_pMockAdapterManager->buildCalls[0].fields["objectType"].toString(), QStringLiteral("holding register"));
    QCOMPARE(_pMockAdapterManager->buildCalls[0].fields["address"].toInt(), 100);
    QCOMPARE(_pMockAdapterManager->buildCalls[0].dataType, QStringLiteral("16b"));
    QCOMPARE(_pMockAdapterManager->buildCalls[0].deviceId, Device::cFirstDeviceId);
}

void TestAddRegisterWidget::registerType()
{
    _pRegWidget->_pAddressForm->setSchema(
      buildAddressSchema(), QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding register") },
                                         { QStringLiteral("address"), 0 },
                                         { QStringLiteral("dataType"), QStringLiteral("32b") } });

    GraphData graphData;
    addRegister(graphData, QStringLiteral("${h0:32b}"));

    QCOMPARE(graphData.expression(), QStringLiteral("${h0:32b}"));
    QCOMPARE(_pMockAdapterManager->buildCalls[0].dataType, QStringLiteral("32b"));
}

void TestAddRegisterWidget::registerObjectType()
{
    _pRegWidget->_pAddressForm->setSchema(
      buildAddressSchema(), QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("input register") },
                                         { QStringLiteral("address"), 0 } });

    GraphData graphData;
    addRegister(graphData, QStringLiteral("${i0}"));

    QCOMPARE(graphData.expression(), QStringLiteral("${i0}"));
    QCOMPARE(_pMockAdapterManager->buildCalls[0].fields["objectType"].toString(), QStringLiteral("input register"));
}

void TestAddRegisterWidget::registerDevice()
{
    _pRegWidget->_pAddressForm->setSchema(
      buildAddressSchema(), QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding register") },
                                         { QStringLiteral("address"), 0 },
                                         { QStringLiteral("deviceId"), 2 } });

    GraphData graphData;
    addRegister(graphData, QStringLiteral("${h0@2}"));

    QCOMPARE(graphData.expression(), QStringLiteral("${h0@2}"));
    QCOMPARE(_pMockAdapterManager->buildCalls[0].deviceId, static_cast<deviceId_t>(2));
}

void TestAddRegisterWidget::registerValueAxis()
{
    QTest::mouseClick(_pRegWidget->_pUi->radioSecondary, Qt::LeftButton);

    GraphData graphData;
    addRegister(graphData, QStringLiteral("${h0}"));

    QCOMPARE(graphData.valueAxis(), GraphData::VALUE_AXIS_SECONDARY);
}

void TestAddRegisterWidget::buildExpressionEmptyResponseIgnored()
{
    QSignalSpy spy(_pRegWidget, &AddRegisterWidget::graphDataConfigured);

    clickAdd();

    /* Adapter returns empty expression — graphDataConfigured must not be emitted */
    _pMockAdapterManager->injectBuildExpressionResult(QString());

    QCOMPARE(spy.count(), 0);
    /* Button should be re-enabled even on empty response */
    QVERIFY(_pRegWidget->_pUi->btnAdd->isEnabled());
}

void TestAddRegisterWidget::buildExpressionDoesNotInterfereWithOtherConnections()
{
    /* Simulate a persistent connection already active (e.g. RegisterDialog's requestDefaultExpression) */
    int secondaryReceiveCount = 0;
    QObject::connect(_pMockAdapterManager, &AdapterManager::buildExpressionResult, _pMockAdapterManager,
                     [&secondaryReceiveCount](const QString&) { secondaryReceiveCount++; });

    /* AddRegisterWidget connects with Qt::SingleShotConnection and receives the result */
    GraphData graphData;
    addRegister(graphData, QStringLiteral("${h0}"));
    QCOMPARE(graphData.expression(), QStringLiteral("${h0}"));

    /* The secondary persistent connection also received the signal */
    QCOMPARE(secondaryReceiveCount, 1);

    /* A second injection must not re-trigger AddRegisterWidget (its SingleShotConnection auto-disconnected) */
    QSignalSpy spy(_pRegWidget, &AddRegisterWidget::graphDataConfigured);
    _pMockAdapterManager->injectBuildExpressionResult(QStringLiteral("${h0}"));
    QCOMPARE(spy.count(), 0);

    /* The persistent secondary connection still fires on subsequent emissions */
    QCOMPARE(secondaryReceiveCount, 2);
}

void TestAddRegisterWidget::adapterComboHiddenWithSingleAdapter()
{
    QVERIFY(_pRegWidget->_pUi->cmbAdapter->isHidden());
}

void TestAddRegisterWidget::adapterComboListsAdapters()
{
    addSimAdapter();

    QVERIFY(!_pRegWidget->_pUi->cmbAdapter->isHidden());
    QCOMPARE(_pRegWidget->_pUi->cmbAdapter->count(), 2);

    /* Adapter IDs are listed alphabetically; label falls back to the ID without describe data */
    QCOMPARE(_pRegWidget->_pUi->cmbAdapter->itemText(0), QStringLiteral("modbus"));
    QCOMPARE(_pRegWidget->_pUi->cmbAdapter->itemData(0).toString(), QStringLiteral("modbus"));
    QCOMPARE(_pRegWidget->_pUi->cmbAdapter->itemText(1), QStringLiteral("Simulator"));
    QCOMPARE(_pRegWidget->_pUi->cmbAdapter->itemData(1).toString(), QStringLiteral("sim"));
}

void TestAddRegisterWidget::switchAdapterRebuildsSchema()
{
    addSimAdapter();

    _pRegWidget->_pUi->cmbAdapter->setCurrentIndex(1);

    QVERIFY(_pRegWidget->_addressSchema["properties"].toObject().contains(QStringLiteral("channel")));
    QCOMPARE(_pRegWidget->_dataPointDefaults["channel"].toInt(), 3);
    QCOMPARE(_pRegWidget->_pAddressForm->values()["channel"].toInt(), 3);
}

void TestAddRegisterWidget::buildExpressionRoutedToSelectedAdapter()
{
    _settingsModel.addDevice(2);
    _settingsModel.deviceSettings(2)->setAdapterId("sim");

    addSimAdapter();

    _pRegWidget->_pUi->cmbAdapter->setCurrentIndex(1);

    QSignalSpy spy(_pRegWidget, &AddRegisterWidget::graphDataConfigured);
    clickAdd();

    QCOMPARE(_pMockSimAdapterManager->buildCalls.size(), 1);
    QCOMPARE(_pMockAdapterManager->buildCalls.size(), 0);

    _pMockSimAdapterManager->injectBuildExpressionResult(QStringLiteral("${c3}"));
    QCOMPARE(spy.count(), 1);
}

void TestAddRegisterWidget::btnAddDisabledWhenSelectedAdapterHasNoDevices()
{
    addSimAdapter();

    /* Only device 1 exists and it belongs to the modbus adapter */
    QVERIFY(_pRegWidget->_pUi->btnAdd->isEnabled());

    _pRegWidget->_pUi->cmbAdapter->setCurrentIndex(1);
    QVERIFY(!_pRegWidget->_pUi->btnAdd->isEnabled());

    _pRegWidget->_pUi->cmbAdapter->setCurrentIndex(0);
    QVERIFY(_pRegWidget->_pUi->btnAdd->isEnabled());
}

void TestAddRegisterWidget::clickAdd()
{
    QTest::mouseClick(_pRegWidget->_pUi->btnAdd, Qt::LeftButton);
}

void TestAddRegisterWidget::addRegister(GraphData& graphData, const QString& expression)
{
    QSignalSpy spyGraphDataConfigured(_pRegWidget, &AddRegisterWidget::graphDataConfigured);

    clickAdd();

    /* Simulate the adapter returning the expression string */
    _pMockAdapterManager->injectBuildExpressionResult(expression);

    QCOMPARE(spyGraphDataConfigured.count(), 1);

    QList<QVariant> arguments = spyGraphDataConfigured.takeFirst();
    QCOMPARE(arguments.count(), 1);

    QVERIFY(arguments.first().canConvert<GraphData>());

    graphData = arguments.first().value<GraphData>();
}

QTEST_MAIN(TestAddRegisterWidget)
