
#include "tst_addregisterwidget.h"

#include "../models/devicelisthelpers.h"
#include "customwidgets/devicecombobox.h"
#include "customwidgets/schemaformwidget.h"
#include "dialogs/addregisterwidget.h"
#include "models/device.h"
#include "ui_addregisterwidget.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
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
    DeviceListHelpers::clearDevices(&_settingsModel);
    DeviceListHelpers::seedDevice(&_settingsModel, Device::cFirstDeviceId, QStringLiteral("modbus"));
    /* Drop the sim adapter data a previous multi-adapter test may have registered */
    _settingsModel.removeAdapter(QStringLiteral("sim"));
    _settingsModel.setAdapterDataPointSchema("modbus", buildTestRegisterSchema());

    _pMockAdapterManager = new MockAdapterManager(&_settingsModel);
    _pMockHub = new MockAdapterHub();
    _pMockHub->addManager(QStringLiteral("modbus"), _pMockAdapterManager);
    _pDataPointService = new DataPointService(&_settingsModel, _pMockHub);
    _pRegWidget = new AddRegisterWidget(&_settingsModel, _pDataPointService);
}

void TestAddRegisterWidget::cleanup()
{
    delete _pRegWidget;
    _pRegWidget = nullptr;
    delete _pDataPointService;
    _pDataPointService = nullptr;
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

    const deviceId_t simDeviceId = DeviceListHelpers::seedNextDevice(&_settingsModel, QStringLiteral("sim"));

    delete _pRegWidget;
    _pRegWidget = new AddRegisterWidget(&_settingsModel, _pDataPointService);

    return simDeviceId;
}

//! Returns the device selector of the given add form.
DeviceComboBox* TestAddRegisterWidget::deviceCombo(AddRegisterWidget* pWidget)
{
    return pWidget->findChild<DeviceComboBox*>();
}

//! Returns the address form of the given add form.
SchemaFormWidget* TestAddRegisterWidget::addressForm(AddRegisterWidget* pWidget)
{
    return pWidget->findChild<SchemaFormWidget*>();
}

/*!
 * \brief Fill the modbus address form with \a values as if the user typed them.
 *
 * The form sends a new build request for the edited values.
 */
void TestAddRegisterWidget::setAddress(const QJsonObject& values)
{
    SchemaFormWidget* pForm = addressForm(_pRegWidget);
    pForm->setSchema(buildAddressSchema(), values);
    emit pForm->valuesChanged();
}

void TestAddRegisterWidget::registerDefault()
{
    QTest::keyClicks(_pRegWidget->_pUi->lineName, "Register 1");

    setAddress(QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding register") },
                            { QStringLiteral("address"), 100 },
                            { QStringLiteral("dataType"), QStringLiteral("16b") } });

    const MockAdapterManager::BuildCall call = _pMockAdapterManager->buildCalls.last();

    GraphData graphData;
    addRegister(graphData, QStringLiteral("${h100}"));

    QCOMPARE(graphData.label(), QStringLiteral("Register 1"));
    QCOMPARE(graphData.expression(), QStringLiteral("${h100}"));
    QVERIFY(graphData.isActive());

    QCOMPARE(call.fields["objectType"].toString(), QStringLiteral("holding register"));
    QCOMPARE(call.fields["address"].toInt(), 100);
    QCOMPARE(call.dataType, QStringLiteral("16b"));
    QCOMPARE(call.deviceId, Device::cFirstDeviceId);
}

void TestAddRegisterWidget::registerType()
{
    setAddress(QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding register") },
                            { QStringLiteral("address"), 0 },
                            { QStringLiteral("dataType"), QStringLiteral("32b") } });
    QCOMPARE(_pMockAdapterManager->buildCalls.last().dataType, QStringLiteral("32b"));

    GraphData graphData;
    addRegister(graphData, QStringLiteral("${h0:32b}"));

    QCOMPARE(graphData.expression(), QStringLiteral("${h0:32b}"));
}

void TestAddRegisterWidget::registerObjectType()
{
    setAddress(QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("input register") },
                            { QStringLiteral("address"), 0 } });
    QCOMPARE(_pMockAdapterManager->buildCalls.last().fields["objectType"].toString(), QStringLiteral("input register"));

    GraphData graphData;
    addRegister(graphData, QStringLiteral("${i0}"));

    QCOMPARE(graphData.expression(), QStringLiteral("${i0}"));
}

void TestAddRegisterWidget::registerDevice()
{
    const deviceId_t secondDeviceId = DeviceListHelpers::seedNextDevice(&_settingsModel, QStringLiteral("modbus"));
    delete _pRegWidget;
    _pRegWidget = new AddRegisterWidget(&_settingsModel, _pDataPointService);

    deviceCombo(_pRegWidget)->setSelectedDeviceId(secondDeviceId);

    GraphData graphData;
    addRegister(graphData, QStringLiteral("${h0@2}"));

    QCOMPARE(graphData.expression(), QStringLiteral("${h0@2}"));
    QCOMPARE(_pMockAdapterManager->buildCalls.last().deviceId, secondDeviceId);
}

void TestAddRegisterWidget::registerValueAxis()
{
    QTest::mouseClick(_pRegWidget->_pUi->radioSecondary, Qt::LeftButton);

    GraphData graphData;
    addRegister(graphData, QStringLiteral("${h0}"));

    QCOMPARE(graphData.valueAxis(), GraphData::VALUE_AXIS_SECONDARY);
}

//! The form builds the default reference on open; Add waits for it.
void TestAddRegisterWidget::addDisabledUntilFirstBuildReply()
{
    QCOMPARE(_pMockAdapterManager->buildCalls.size(), 1);
    QVERIFY(!_pRegWidget->_pUi->btnAdd->isEnabled());

    _pMockAdapterManager->injectBuildExpressionResult(QStringLiteral("${h0}"));

    QVERIFY(_pRegWidget->_pUi->btnAdd->isEnabled());
}

//! A field change disables Add until the new reference is built.
void TestAddRegisterWidget::addDisabledWhileBuildPending()
{
    _pMockAdapterManager->injectBuildExpressionResult(QStringLiteral("${h0}"));
    QVERIFY(_pRegWidget->_pUi->btnAdd->isEnabled());

    setAddress(QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding register") },
                            { QStringLiteral("address"), 5 } });

    QVERIFY(!_pRegWidget->_pUi->btnAdd->isEnabled());
    QCOMPARE(_pMockAdapterManager->buildCalls.size(), 2);
}

/*!
 * \brief Regression test: an adapter error for buildExpression used to stop the adapter. The form
 * must stay usable, so the user can correct the address and try again.
 */
void TestAddRegisterWidget::buildExpressionFailureKeepsFormUsable()
{
    QSignalSpy spy(_pRegWidget, &AddRegisterWidget::graphDataConfigured);

    _pMockAdapterManager->injectBuildExpressionFailure(QStringLiteral("Address out of range"));
    QTRY_VERIFY(!_pRegWidget->_pUi->lblError->isHidden());
    QVERIFY(!_pRegWidget->_pUi->btnAdd->isEnabled());

    GraphData graphData;
    setAddress(QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding register") },
                            { QStringLiteral("address"), 5 } });
    addRegister(graphData, QStringLiteral("${h5}"));

    QCOMPARE(graphData.expression(), QStringLiteral("${h5}"));
}

//! The name field starts empty with a placeholder; an empty name is left for the model to fill in.
void TestAddRegisterWidget::emptyNameAddsSignalWithoutLabel()
{
    QCOMPARE(_pRegWidget->_pUi->lineName->text(), QString());
    QVERIFY(!_pRegWidget->_pUi->lineName->placeholderText().isEmpty());

    GraphData graphData;
    addRegister(graphData, QStringLiteral("${h0}"));

    QCOMPARE(graphData.label(), QString());
}

//! The name field is cleared after a signal was added.
void TestAddRegisterWidget::nameClearedAfterAdd()
{
    QTest::keyClicks(_pRegWidget->_pUi->lineName, "Register 1");

    GraphData graphData;
    addRegister(graphData, QStringLiteral("${h0}"));

    QCOMPARE(graphData.label(), QStringLiteral("Register 1"));
    QCOMPARE(_pRegWidget->_pUi->lineName->text(), QString());
}

//! After Add the form returns to the adapter defaults and builds them again.
void TestAddRegisterWidget::formResetAfterAdd()
{
    setAddress(QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding register") },
                            { QStringLiteral("address"), 100 } });

    GraphData graphData;
    addRegister(graphData, QStringLiteral("${h100}"));

    QCOMPARE(addressForm(_pRegWidget)->values().value("address").toInt(), 0);
    QCOMPARE(_pMockAdapterManager->buildCalls.last().fields["address"].toInt(), 0);
    QVERIFY(!_pRegWidget->_pUi->btnAdd->isEnabled());
}

void TestAddRegisterWidget::deviceComboVisibleWithSingleDevice()
{
    QVERIFY(!deviceCombo(_pRegWidget)->isHidden());
}

void TestAddRegisterWidget::deviceComboListsDevices()
{
    const deviceId_t simDeviceId = addSimAdapter();
    DeviceComboBox* pCombo = deviceCombo(_pRegWidget);

    QVERIFY(!pCombo->isHidden());
    QCOMPARE(pCombo->count(), 2);

    /* Devices are listed in ascending id order; label defaults to "Device N" */
    QCOMPARE(pCombo->itemText(0), QStringLiteral("Device 1"));
    QCOMPARE(pCombo->itemData(0).value<deviceId_t>(), Device::cFirstDeviceId);
    QCOMPARE(pCombo->itemText(1), QString("Device %1").arg(simDeviceId));
    QCOMPARE(pCombo->itemData(1).value<deviceId_t>(), simDeviceId);
}

void TestAddRegisterWidget::switchDeviceRebuildsSchema()
{
    const deviceId_t simDeviceId = addSimAdapter();

    deviceCombo(_pRegWidget)->setSelectedDeviceId(simDeviceId);

    QCOMPARE(addressForm(_pRegWidget)->values().value("channel").toInt(), 3);
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
    deviceCombo(pWidget)->setSelectedDeviceId(simDeviceId);
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
    deviceCombo(pWidget)->setSelectedDeviceId(simDeviceId);
    menu.show();

    /* Switch, while the popup is open, from sim (1 field) to modbus (3 fields). Newly created
     * field widgets used to stay hidden until the next event loop turn, which made QFormLayout
     * treat them as zero-sized and left the popup too small to show the rebuilt form - see
     * SchemaFormWidget::addFieldRow(). Check the state right after the switch, with no event
     * loop turn in between, so a regression here fails immediately instead of only intermittently. */
    deviceCombo(pWidget)->setSelectedDeviceId(Device::cFirstDeviceId);

    QLayout* addressFormLayout = addressForm(pWidget)->layout();
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
    const qsizetype modbusCallsBefore = _pMockAdapterManager->buildCalls.size();

    deviceCombo(_pRegWidget)->setSelectedDeviceId(simDeviceId);

    QCOMPARE(_pMockSimAdapterManager->buildCalls.size(), 1);
    QCOMPARE(_pMockAdapterManager->buildCalls.size(), modbusCallsBefore);
    QCOMPARE(_pMockSimAdapterManager->buildCalls[0].deviceId, simDeviceId);

    QSignalSpy spy(_pRegWidget, &AddRegisterWidget::graphDataConfigured);
    _pMockSimAdapterManager->injectBuildExpressionResult(QStringLiteral("${c3}"));
    clickAdd();

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<GraphData>().expression(), QStringLiteral("${c3}"));
}

void TestAddRegisterWidget::btnAddDisabledWhenSelectedDeviceAdapterUnavailable()
{
    /* Bind a second device to an adapter id that is never registered in the mock hub */
    const deviceId_t unavailableDeviceId =
      DeviceListHelpers::seedNextDevice(&_settingsModel, QStringLiteral("unavailable"));
    delete _pRegWidget;
    _pRegWidget = new AddRegisterWidget(&_settingsModel, _pDataPointService);

    /* Device 1 is initially selected and it belongs to the modbus adapter */
    _pMockAdapterManager->injectBuildExpressionResult(QStringLiteral("${h0}"));
    QVERIFY(_pRegWidget->_pUi->btnAdd->isEnabled());

    deviceCombo(_pRegWidget)->setSelectedDeviceId(unavailableDeviceId);
    QVERIFY(!_pRegWidget->_pUi->btnAdd->isEnabled());
    QTRY_VERIFY(!_pRegWidget->_pUi->lblError->isHidden());

    deviceCombo(_pRegWidget)->setSelectedDeviceId(Device::cFirstDeviceId);
    _pMockAdapterManager->injectBuildExpressionResult(QStringLiteral("${h0}"));
    QVERIFY(_pRegWidget->_pUi->btnAdd->isEnabled());
}

void TestAddRegisterWidget::deviceComboRefreshesLiveOnDeviceAdded()
{
    QCOMPARE(deviceCombo(_pRegWidget)->count(), 1);

    /* Mirrors addSimAdapter()'s setup, but without deleting/reconstructing _pRegWidget:
     * the point of this test is that the combo now picks up the new device on its own. */
    _settingsModel.setAdapterDataPointSchema("sim", buildSimRegisterSchema());
    QJsonObject describeResult;
    describeResult["name"] = QStringLiteral("Simulator");
    _settingsModel.updateAdapterFromDescribe(QStringLiteral("sim"), describeResult);
    _pMockSimAdapterManager = new MockAdapterManager(&_settingsModel, QStringLiteral("sim"));
    _pMockHub->addManager(QStringLiteral("sim"), _pMockSimAdapterManager);

    const deviceId_t simDeviceId = DeviceListHelpers::seedNextDevice(&_settingsModel, QStringLiteral("sim"));

    QCOMPARE(deviceCombo(_pRegWidget)->count(), 2);
    QCOMPARE(deviceCombo(_pRegWidget)->itemText(1), QString("Device %1").arg(simDeviceId));
}

void TestAddRegisterWidget::deviceAddedToEmptyListAppliesConfiguredAdapter()
{
    _settingsModel.setAdapterDataPointSchema("sim", buildSimRegisterSchema());
    QJsonObject describeResult;
    describeResult["name"] = QStringLiteral("Simulator");
    _settingsModel.updateAdapterFromDescribe(QStringLiteral("sim"), describeResult);
    _pMockSimAdapterManager = new MockAdapterManager(&_settingsModel, QStringLiteral("sim"));
    _pMockHub->addManager(QStringLiteral("sim"), _pMockSimAdapterManager);

    /* Start from an empty device list, so the device added below becomes the selected one
     * and the combo refresh has to build its address form from the sim adapter. */
    DeviceListHelpers::clearDevices(&_settingsModel);
    delete _pRegWidget;
    _pRegWidget = new AddRegisterWidget(&_settingsModel, _pDataPointService);
    QCOMPARE(deviceCombo(_pRegWidget)->count(), 0);

    const deviceId_t simDeviceId = Device::cFirstDeviceId;
    DeviceListHelpers::seedDevice(&_settingsModel, simDeviceId, QStringLiteral("sim"));

    /* The adapter must already be assigned when deviceListChanged() arrives: assigning it
     * afterwards left the widget showing Device's default ("modbus") adapter instead. */
    QCOMPARE(deviceCombo(_pRegWidget)->count(), 1);
    QVERIFY(hasLabelWithText(QStringLiteral("Protocol: Simulator")));
    QCOMPARE(addressForm(_pRegWidget)->values().value("channel").toInt(), 3);

    _pMockSimAdapterManager->injectBuildExpressionResult(QStringLiteral("${c3}"));
    QVERIFY(_pRegWidget->_pUi->btnAdd->isEnabled());
}

void TestAddRegisterWidget::deviceComboDisablesAddWhenLastDeviceRemoved()
{
    _pMockAdapterManager->injectBuildExpressionResult(QStringLiteral("${h0}"));
    QVERIFY(_pRegWidget->_pUi->btnAdd->isEnabled());

    DeviceListHelpers::dropDevice(&_settingsModel, Device::cFirstDeviceId);

    QCOMPARE(deviceCombo(_pRegWidget)->count(), 0);
    QVERIFY(!_pRegWidget->_pUi->btnAdd->isEnabled());
}

void TestAddRegisterWidget::addRegisterClosesContainingMenu()
{
    /* See switchDeviceWhileMenuOpenResizesPopup for why ownership is handed off up front. */
    AddRegisterWidget* pWidget = _pRegWidget;
    _pRegWidget = nullptr;

    QMenu menu;
    auto* action = new QWidgetAction(&menu);
    action->setDefaultWidget(pWidget);
    menu.addAction(action);

    menu.show();
    QVERIFY(menu.isVisible());

    _pMockAdapterManager->injectBuildExpressionResult(QStringLiteral("${h0}"));
    QTest::mouseClick(pWidget->_pUi->btnAdd, Qt::LeftButton);

    /* The menu is closed on the next event loop turn */
    QTRY_VERIFY(!menu.isVisible());
}

void TestAddRegisterWidget::emptyExpressionResponseLeavesContainingMenuOpen()
{
    /* See switchDeviceWhileMenuOpenResizesPopup for why ownership is handed off up front. */
    AddRegisterWidget* pWidget = _pRegWidget;
    _pRegWidget = nullptr;

    QMenu menu;
    auto* action = new QWidgetAction(&menu);
    action->setDefaultWidget(pWidget);
    menu.addAction(action);

    menu.show();

    _pMockAdapterManager->injectBuildExpressionResult(QString());
    QTest::mouseClick(pWidget->_pUi->btnAdd, Qt::LeftButton);

    QVERIFY(!pWidget->_pUi->btnAdd->isEnabled());
    QVERIFY(menu.isVisible());

    menu.hide();
}

//! A failed build shows its reason and leaves the containing popup open.
void TestAddRegisterWidget::buildFailureShowsReasonInOpenMenu()
{
    /* See switchDeviceWhileMenuOpenResizesPopup for why ownership is handed off up front. */
    AddRegisterWidget* pWidget = _pRegWidget;
    _pRegWidget = nullptr;

    QMenu menu;
    auto* action = new QWidgetAction(&menu);
    action->setDefaultWidget(pWidget);
    menu.addAction(action);

    menu.show();
    QVERIFY(pWidget->_pUi->lblError->isHidden());

    _pMockAdapterManager->injectBuildExpressionFailure(QStringLiteral("Invalid address"));

    QTRY_VERIFY(pWidget->_pUi->lblError->text().startsWith(QStringLiteral("Invalid address")));
    QVERIFY(!pWidget->_pUi->lblError->isHidden());
    QVERIFY(menu.isVisible());

    menu.hide();
}

//! A long failure reason wraps within the form instead of widening the popup.
void TestAddRegisterWidget::longBuildErrorWrapsInOpenMenu()
{
    /* See switchDeviceWhileMenuOpenResizesPopup for why ownership is handed off up front. */
    AddRegisterWidget* pWidget = _pRegWidget;
    _pRegWidget = nullptr;

    QMenu menu;
    auto* action = new QWidgetAction(&menu);
    action->setDefaultWidget(pWidget);
    menu.addAction(action);

    menu.show();
    const int widthBefore = menu.width();

    const QString longError = QStringLiteral("The address is out of range for this object type. ").repeated(6);
    _pMockAdapterManager->injectBuildExpressionFailure(longError);
    QTRY_VERIFY(pWidget->_pUi->lblError->text().startsWith(longError));

    QCOMPARE(menu.width(), widthBefore);
    QVERIFY(pWidget->_pUi->lblError->height() >=
            pWidget->_pUi->lblError->heightForWidth(pWidget->_pUi->lblError->width()));

    menu.hide();
}

//! An empty build result is reported instead of silently ignored.
void TestAddRegisterWidget::emptyBuildResultShowsReason()
{
    _pMockAdapterManager->injectBuildExpressionResult(QString());

    QVERIFY(!_pRegWidget->_pUi->lblError->isHidden());
    QVERIFY(!_pRegWidget->_pUi->lblError->text().isEmpty());
    QVERIFY(!_pRegWidget->_pUi->btnAdd->isEnabled());
}

//! A failure without a reason still tells the user the address was not accepted.
void TestAddRegisterWidget::buildFailureWithoutReasonShowsFallback()
{
    _pMockAdapterManager->injectBuildExpressionFailure(QString());

    QTRY_VERIFY(!_pRegWidget->_pUi->lblError->isHidden());
    QVERIFY(!_pRegWidget->_pUi->lblError->text().isEmpty());
}

//! The label shows adapter text as plain text.
void TestAddRegisterWidget::buildErrorIsPlainText()
{
    QCOMPARE(_pRegWidget->_pUi->lblError->textFormat(), Qt::PlainText);
}

//! A field change after a failure hides the failure reason.
void TestAddRegisterWidget::fieldChangeHidesBuildError()
{
    _pMockAdapterManager->injectBuildExpressionFailure(QStringLiteral("Invalid address"));
    QTRY_VERIFY(!_pRegWidget->_pUi->lblError->isHidden());

    setAddress(QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding register") },
                            { QStringLiteral("address"), 5 } });

    QVERIFY(_pRegWidget->_pUi->lblError->isHidden());
    QCOMPARE(_pRegWidget->_pUi->lblError->text(), QString());
}

//! Selecting another device hides the failure reason.
void TestAddRegisterWidget::deviceSwitchHidesBuildError()
{
    const deviceId_t simDeviceId = addSimAdapter();

    _pMockAdapterManager->injectBuildExpressionFailure(QStringLiteral("Invalid address"));
    QTRY_VERIFY(!_pRegWidget->_pUi->lblError->isHidden());

    deviceCombo(_pRegWidget)->setSelectedDeviceId(simDeviceId);

    QVERIFY(_pRegWidget->_pUi->lblError->isHidden());
}

void TestAddRegisterWidget::clickAdd()
{
    QTest::mouseClick(_pRegWidget->_pUi->btnAdd, Qt::LeftButton);
}

/*!
 * \brief Answer the pending build request with \a expression, click Add and return the configured signal.
 */
void TestAddRegisterWidget::addRegister(GraphData& graphData, const QString& expression)
{
    QSignalSpy spyGraphDataConfigured(_pRegWidget, &AddRegisterWidget::graphDataConfigured);

    /* Simulate the adapter returning the expression string */
    _pMockAdapterManager->injectBuildExpressionResult(expression);

    clickAdd();

    QCOMPARE(spyGraphDataConfigured.count(), 1);

    QList<QVariant> arguments = spyGraphDataConfigured.takeFirst();
    QCOMPARE(arguments.count(), 1);

    QVERIFY(arguments.first().canConvert<GraphData>());

    graphData = arguments.first().value<GraphData>();
}

//! Returns whether the add form shows a label with exactly \a text.
bool TestAddRegisterWidget::hasLabelWithText(const QString& text) const
{
    const QList<QLabel*> labels = _pRegWidget->findChildren<QLabel*>();
    for (const QLabel* pLabel : labels)
    {
        if (pLabel->text() == text)
        {
            return true;
        }
    }
    return false;
}

QTEST_MAIN(TestAddRegisterWidget)
