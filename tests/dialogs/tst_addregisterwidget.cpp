
#include "tst_addregisterwidget.h"

#include "customwidgets/schemaformwidget.h"
#include "dialogs/addregisterwidget.h"
#include "models/device.h"
#include "ui_addregisterwidget.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QMenu>
#include <QSignalSpy>
#include <QTest>
#include <QWidgetAction>

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

    QJsonObject properties;
    properties["objectType"] = objectTypeSchema;
    properties["address"] = addressField;
    properties["dataType"] = dataTypeSchema;

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
 * \brief Register a second ("sim") adapter with a device bound to it, and rebuild the widget.
 * \return The id of the newly created device.
 */
deviceId_t TestAddRegisterWidget::addSimAdapter()
{
    _settingsModel.setAdapterDataPointSchema("sim", buildSimRegisterSchema());

    QJsonObject describeResult;
    describeResult["name"] = QStringLiteral("Simulator");
    _settingsModel.updateAdapterFromDescribe(QStringLiteral("sim"), describeResult);

    _pMockSimAdapterManager = new MockAdapterManager(&_settingsModel, QStringLiteral("sim"));
    _pMockHub->addManager(QStringLiteral("sim"), _pMockSimAdapterManager);

    const deviceId_t simDeviceId = _settingsModel.addNewDevice();
    _settingsModel.deviceSettings(simDeviceId)->setAdapterId(QStringLiteral("sim"));

    delete _pRegWidget;
    _pRegWidget = new AddRegisterWidget(&_settingsModel, _pMockHub);

    return simDeviceId;
}

/*!
 * \brief Returns the cmbDevice index whose item data matches the given device id.
 */
int TestAddRegisterWidget::indexForDevice(deviceId_t devId) const
{
    return _pRegWidget->_pUi->cmbDevice->findData(QVariant(devId));
}

void TestAddRegisterWidget::registerDefault()
{
    _pRegWidget->_pUi->lineName->selectAll();
    QTest::keyClicks(_pRegWidget->_pUi->lineName, "Register 1");

    _pRegWidget->_pAddressForm->setSchema(
      buildAddressSchema(), QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding register") },
                                         { QStringLiteral("address"), 100 },
                                         { QStringLiteral("dataType"), QStringLiteral("16b") } });

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
    const deviceId_t secondDeviceId = _settingsModel.addNewDevice();
    _settingsModel.deviceSettings(secondDeviceId)->setAdapterId(QStringLiteral("modbus"));
    delete _pRegWidget;
    _pRegWidget = new AddRegisterWidget(&_settingsModel, _pMockHub);

    _pRegWidget->_pUi->cmbDevice->setCurrentIndex(indexForDevice(secondDeviceId));
    _pRegWidget->_pAddressForm->setSchema(
      buildAddressSchema(), QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding register") },
                                         { QStringLiteral("address"), 0 } });

    GraphData graphData;
    addRegister(graphData, QStringLiteral("${h0@2}"));

    QCOMPARE(graphData.expression(), QStringLiteral("${h0@2}"));
    QCOMPARE(_pMockAdapterManager->buildCalls[0].deviceId, secondDeviceId);
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

void TestAddRegisterWidget::deviceComboVisibleWithSingleDevice()
{
    QVERIFY(!_pRegWidget->_pUi->cmbDevice->isHidden());
}

void TestAddRegisterWidget::deviceComboListsDevices()
{
    const deviceId_t simDeviceId = addSimAdapter();

    QVERIFY(!_pRegWidget->_pUi->cmbDevice->isHidden());
    QCOMPARE(_pRegWidget->_pUi->cmbDevice->count(), 2);

    /* Devices are listed in ascending id order; label defaults to "Device N" */
    QCOMPARE(_pRegWidget->_pUi->cmbDevice->itemText(0), QStringLiteral("Device 1"));
    QCOMPARE(_pRegWidget->_pUi->cmbDevice->itemData(0).value<deviceId_t>(), Device::cFirstDeviceId);
    QCOMPARE(_pRegWidget->_pUi->cmbDevice->itemText(1), QString("Device %1").arg(simDeviceId));
    QCOMPARE(_pRegWidget->_pUi->cmbDevice->itemData(1).value<deviceId_t>(), simDeviceId);
}

void TestAddRegisterWidget::switchDeviceRebuildsSchema()
{
    const deviceId_t simDeviceId = addSimAdapter();

    _pRegWidget->_pUi->cmbDevice->setCurrentIndex(indexForDevice(simDeviceId));

    QVERIFY(_pRegWidget->_addressSchema["properties"].toObject().contains(QStringLiteral("channel")));
    QCOMPARE(_pRegWidget->_dataPointDefaults["channel"].toInt(), 3);
    QCOMPARE(_pRegWidget->_pAddressForm->values()["channel"].toInt(), 3);
}

void TestAddRegisterWidget::switchDeviceWhileMenuOpenResizesPopup()
{
    const deviceId_t simDeviceId = addSimAdapter();

    /* QWidgetAction::setDefaultWidget() takes ownership of the widget: its destructor
     * unconditionally deletes it. Hand it off and clear the fixture pointer up front - before
     * the QVERIFY below, which returns early on failure - so cleanup()'s `delete _pRegWidget`
     * can never race with the action's destructor deleting the same object. */
    AddRegisterWidget* pWidget = _pRegWidget;
    _pRegWidget = nullptr;

    QMenu menu;
    auto* action = new QWidgetAction(&menu);
    action->setDefaultWidget(pWidget);
    /* Reparents pWidget into `menu` synchronously (QMenu::actionEvent). */
    menu.addAction(action);

    /* Showing lays out the menu for the first time, sizing it to fit the modbus schema (3 fields). */
    menu.show();
    const int heightWithThreeFields = menu.height();

    /* Switch, while the popup is open, from modbus (3 fields) to sim (1 field) */
    pWidget->_pUi->cmbDevice->setCurrentIndex(pWidget->_pUi->cmbDevice->findData(QVariant(simDeviceId)));
    const int heightWithOneField = menu.height();

    menu.hide();

    QVERIFY(heightWithOneField < heightWithThreeFields);
}

void TestAddRegisterWidget::switchToLargerSchemaWhileMenuOpenShowsAllFields()
{
    const deviceId_t simDeviceId = addSimAdapter();

    /* See switchDeviceWhileMenuOpenResizesPopup for why ownership is handed off up front. */
    AddRegisterWidget* pWidget = _pRegWidget;
    _pRegWidget = nullptr;

    QMenu menu;
    auto* action = new QWidgetAction(&menu);
    action->setDefaultWidget(pWidget);
    menu.addAction(action);

    /* Start from sim (1 field) so the switch below grows the form. */
    pWidget->_pUi->cmbDevice->setCurrentIndex(pWidget->_pUi->cmbDevice->findData(QVariant(simDeviceId)));
    menu.show();

    /* Switch, while the popup is open, from sim (1 field) to modbus (3 fields). Newly created
     * field widgets used to stay hidden until the next event loop turn, which made QFormLayout
     * treat them as zero-sized and left the popup too small to show the rebuilt form - see
     * SchemaFormWidget::addFieldRow(). Check the state right after the switch, with no event
     * loop turn in between, so a regression here fails immediately instead of only intermittently. */
    pWidget->_pUi->cmbDevice->setCurrentIndex(pWidget->_pUi->cmbDevice->findData(QVariant(Device::cFirstDeviceId)));

    QLayout* addressFormLayout = pWidget->_pAddressForm->layout();
    QCOMPARE(addressFormLayout->count(), 6); /* modbus schema: 3 fields x (label + field) */
    for (int i = 0; i < addressFormLayout->count(); ++i)
    {
        QVERIFY(addressFormLayout->itemAt(i)->widget()->isVisible());
    }

    /* isVisible() alone doesn't catch a too-small popup: a widget stays visible even when its
     * geometry falls outside the QMenu window bounds. Map the last field's bottom edge into the
     * menu's coordinate space to confirm resizeContainingMenu() actually grew the popup enough to
     * show it, rather than leaving it clipped by the window edge. */
    QWidget* pLastField = addressFormLayout->itemAt(addressFormLayout->count() - 1)->widget();
    const int lastFieldBottomInMenu = pLastField->mapTo(&menu, QPoint(0, pLastField->height())).y();
    QVERIFY(lastFieldBottomInMenu <= menu.height());

    menu.hide();
}

void TestAddRegisterWidget::buildExpressionRoutedToSelectedDevice()
{
    const deviceId_t simDeviceId = addSimAdapter();

    _pRegWidget->_pUi->cmbDevice->setCurrentIndex(indexForDevice(simDeviceId));

    QSignalSpy spy(_pRegWidget, &AddRegisterWidget::graphDataConfigured);
    clickAdd();

    QCOMPARE(_pMockSimAdapterManager->buildCalls.size(), 1);
    QCOMPARE(_pMockAdapterManager->buildCalls.size(), 0);
    QCOMPARE(_pMockSimAdapterManager->buildCalls[0].deviceId, simDeviceId);

    _pMockSimAdapterManager->injectBuildExpressionResult(QStringLiteral("${c3}"));
    QCOMPARE(spy.count(), 1);
}

void TestAddRegisterWidget::btnAddDisabledWhenSelectedDeviceAdapterUnavailable()
{
    /* Bind a second device to an adapter id that is never registered in the mock hub */
    const deviceId_t unavailableDeviceId = _settingsModel.addNewDevice();
    _settingsModel.deviceSettings(unavailableDeviceId)->setAdapterId(QStringLiteral("unavailable"));
    delete _pRegWidget;
    _pRegWidget = new AddRegisterWidget(&_settingsModel, _pMockHub);

    /* Only device 1 is initially selected and it belongs to the modbus adapter */
    QVERIFY(_pRegWidget->_pUi->btnAdd->isEnabled());

    _pRegWidget->_pUi->cmbDevice->setCurrentIndex(indexForDevice(unavailableDeviceId));
    QVERIFY(!_pRegWidget->_pUi->btnAdd->isEnabled());

    _pRegWidget->_pUi->cmbDevice->setCurrentIndex(indexForDevice(Device::cFirstDeviceId));
    QVERIFY(_pRegWidget->_pUi->btnAdd->isEnabled());
}

void TestAddRegisterWidget::deviceComboRefreshesLiveOnDeviceAdded()
{
    QCOMPARE(_pRegWidget->_pUi->cmbDevice->count(), 1);

    /* Mirrors addSimAdapter()'s setup, but without deleting/reconstructing _pRegWidget:
     * the point of this test is that the combo now picks up the new device on its own. */
    _settingsModel.setAdapterDataPointSchema("sim", buildSimRegisterSchema());
    QJsonObject describeResult;
    describeResult["name"] = QStringLiteral("Simulator");
    _settingsModel.updateAdapterFromDescribe(QStringLiteral("sim"), describeResult);
    _pMockSimAdapterManager = new MockAdapterManager(&_settingsModel, QStringLiteral("sim"));
    _pMockHub->addManager(QStringLiteral("sim"), _pMockSimAdapterManager);

    const deviceId_t simDeviceId = _settingsModel.addNewDevice();
    _settingsModel.deviceSettings(simDeviceId)->setAdapterId(QStringLiteral("sim"));

    QCOMPARE(_pRegWidget->_pUi->cmbDevice->count(), 2);
    QCOMPARE(_pRegWidget->_pUi->cmbDevice->itemText(1), QString("Device %1").arg(simDeviceId));
}

void TestAddRegisterWidget::deviceComboDisablesAddWhenLastDeviceRemoved()
{
    _settingsModel.removeDevice(Device::cFirstDeviceId);

    QCOMPARE(_pRegWidget->_pUi->cmbDevice->count(), 0);
    QVERIFY(!_pRegWidget->_pUi->btnAdd->isEnabled());
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
