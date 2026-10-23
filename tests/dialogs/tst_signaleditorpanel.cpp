
#include "tst_signaleditorpanel.h"

#include "../models/devicelisthelpers.h"
#include "customwidgets/datapointeditorwidget.h"
#include "customwidgets/devicecombobox.h"
#include "customwidgets/schemaformwidget.h"
#include "dialogs/signaleditorpanel.h"
#include "models/datapointdescriptioncache.h"
#include "models/device.h"
#include "models/graphdatamodel.h"
#include "ui_signaleditorpanel.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QListWidget>
#include <QPointer>
#include <QSignalSpy>
#include <QSpinBox>
#include <QTest>

QJsonObject TestSignalEditorPanel::buildAddressSchema()
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

QJsonObject TestSignalEditorPanel::buildTestRegisterSchema()
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

QJsonObject TestSignalEditorPanel::buildSimRegisterSchema()
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

void TestSignalEditorPanel::init()
{
    DeviceListHelpers::clearDevices(&_settingsModel);
    DeviceListHelpers::seedDevice(&_settingsModel, Device::cFirstDeviceId, QStringLiteral("modbus"));
    /* Drop the sim adapter data a previous multi-adapter test may have registered */
    _settingsModel.removeAdapter(QStringLiteral("sim"));
    _settingsModel.setAdapterDataPointSchema("modbus", buildTestRegisterSchema());

    _pGraphDataModel = new GraphDataModel();
    _pMockDataPointManager = new MockDataPointManager(QStringLiteral("modbus"), &_settingsModel);
    _pMockHub = new MockAdapterHub();
    _pMockHub->addManager(QStringLiteral("modbus"), _pMockDataPointManager);
    _pDataPointService = new DataPointService(&_settingsModel, _pMockHub);
    _pDescriptionCache = new DataPointDescriptionCache(_pDataPointService);
    _pPanel = new SignalEditorPanel(_pGraphDataModel, &_settingsModel, _pDataPointService, _pDescriptionCache);
}

void TestSignalEditorPanel::cleanup()
{
    delete _pPanel;
    _pPanel = nullptr;
    delete _pDescriptionCache;
    _pDescriptionCache = nullptr;
    delete _pDataPointService;
    _pDataPointService = nullptr;
    delete _pMockDataPointManager;
    _pMockDataPointManager = nullptr;
    delete _pMockSimAdapterManager;
    _pMockSimAdapterManager = nullptr;
    delete _pMockHub;
    _pMockHub = nullptr;
    delete _pGraphDataModel;
    _pGraphDataModel = nullptr;
}

//! Replace the panel with a new one, so it picks up the current device list.
void TestSignalEditorPanel::recreatePanel()
{
    delete _pPanel;
    _pPanel = new SignalEditorPanel(_pGraphDataModel, &_settingsModel, _pDataPointService, _pDescriptionCache);
}

//! Add a signal with \a expression to the model and edit it in the panel.
void TestSignalEditorPanel::editNewSignal(const QString& expression)
{
    GraphData graphData;
    graphData.setLabel(QStringLiteral("Signal"));
    graphData.setExpression(expression);
    _pGraphDataModel->add(graphData);

    _pPanel->editSignal(GraphIdx(_pGraphDataModel->size() - 1));
}

//! Answer a describeDataPoint() call (0 for the first) with \a fields and the description "described <reference>".
void TestSignalEditorPanel::describeCall(qsizetype callIndex, const QJsonObject& fields)
{
    const QString description = QStringLiteral("described %1").arg(_pMockDataPointManager->describeCalls.at(callIndex));
    _pMockDataPointManager->injectDescribeResult(callIndex, description, fields);
}

//! The describeDataPoint fields of a holding register on the first device.
QJsonObject TestSignalEditorPanel::holdingRegisterFields(int address)
{
    return QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding register") },
                        { QStringLiteral("address"), address },
                        { QStringLiteral("dataType"), QStringLiteral("16b") },
                        { QStringLiteral("deviceId"), static_cast<int>(Device::cFirstDeviceId) } };
}

/*!
 * \brief Register a second ("sim") adapter with a device bound to it, and rebuild the panel.
 * \return The id of the newly created device.
 */
deviceId_t TestSignalEditorPanel::addSimAdapter()
{
    _settingsModel.setAdapterDataPointSchema("sim", buildSimRegisterSchema());

    QJsonObject describeResult;
    describeResult["name"] = QStringLiteral("Simulator");
    _settingsModel.updateAdapterFromDescribe(QStringLiteral("sim"), describeResult);

    _pMockSimAdapterManager = new MockDataPointManager(QStringLiteral("sim"), &_settingsModel);
    _pMockHub->addManager(QStringLiteral("sim"), _pMockSimAdapterManager);

    const deviceId_t simDeviceId = DeviceListHelpers::seedNextDevice(&_settingsModel, QStringLiteral("sim"));

    recreatePanel();

    return simDeviceId;
}

//! Returns the device selector of the given panel.
DeviceComboBox* TestSignalEditorPanel::deviceCombo(SignalEditorPanel* pPanel)
{
    return pPanel->findChild<DeviceComboBox*>();
}

//! Returns the address form of the given panel.
SchemaFormWidget* TestSignalEditorPanel::addressForm(SignalEditorPanel* pPanel)
{
    return pPanel->findChild<SchemaFormWidget*>();
}

/*!
 * \brief Fill the modbus address form with \a values as if the user typed them.
 *
 * The form sends a new build request for the edited values.
 */
void TestSignalEditorPanel::setAddress(const QJsonObject& values)
{
    SchemaFormWidget* pForm = addressForm(_pPanel);
    pForm->setSchema(buildAddressSchema(), values);
    emit pForm->valuesChanged();
}

void TestSignalEditorPanel::registerDefault()
{
    QTest::keyClicks(_pPanel->_pUi->lineName, "Register 1");

    setAddress(QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding register") },
                            { QStringLiteral("address"), 100 },
                            { QStringLiteral("dataType"), QStringLiteral("16b") } });

    const MockDataPointManager::BuildCall call = _pMockDataPointManager->buildCalls.last();

    addSignal(QStringLiteral("${h100}"));

    QCOMPARE(_pGraphDataModel->size(), 1);
    QCOMPARE(_pGraphDataModel->label(GraphIdx(0)), QStringLiteral("Register 1"));
    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QStringLiteral("${h100}"));
    QVERIFY(_pGraphDataModel->isActive(GraphIdx(0)));

    QCOMPARE(call.fields["objectType"].toString(), QStringLiteral("holding register"));
    QCOMPARE(call.fields["address"].toInt(), 100);
    QCOMPARE(call.dataType, QStringLiteral("16b"));
    QCOMPARE(call.deviceId, Device::cFirstDeviceId);
}

void TestSignalEditorPanel::registerType()
{
    setAddress(QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding register") },
                            { QStringLiteral("address"), 0 },
                            { QStringLiteral("dataType"), QStringLiteral("32b") } });
    QCOMPARE(_pMockDataPointManager->buildCalls.last().dataType, QStringLiteral("32b"));

    addSignal(QStringLiteral("${h0:32b}"));

    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QStringLiteral("${h0:32b}"));
}

void TestSignalEditorPanel::registerObjectType()
{
    setAddress(QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("input register") },
                            { QStringLiteral("address"), 0 } });
    QCOMPARE(_pMockDataPointManager->buildCalls.last().fields["objectType"].toString(),
             QStringLiteral("input register"));

    addSignal(QStringLiteral("${i0}"));

    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QStringLiteral("${i0}"));
}

void TestSignalEditorPanel::registerDevice()
{
    const deviceId_t secondDeviceId = DeviceListHelpers::seedNextDevice(&_settingsModel, QStringLiteral("modbus"));
    recreatePanel();

    deviceCombo(_pPanel)->setSelectedDeviceId(secondDeviceId);

    addSignal(QStringLiteral("${h0@2}"));

    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QStringLiteral("${h0@2}"));
    QCOMPARE(_pMockDataPointManager->buildCalls.last().deviceId, secondDeviceId);
}

void TestSignalEditorPanel::registerValueAxis()
{
    QTest::mouseClick(_pPanel->_pUi->radioSecondary, Qt::LeftButton);

    addSignal(QStringLiteral("${h0}"));

    QCOMPARE(_pGraphDataModel->valueAxis(GraphIdx(0)), GraphData::VALUE_AXIS_SECONDARY);
}

//! The form builds the default reference on open; Add waits for it.
void TestSignalEditorPanel::addDisabledUntilFirstBuildReply()
{
    QCOMPARE(_pMockDataPointManager->buildCalls.size(), 1);
    QVERIFY(!_pPanel->_pUi->btnAdd->isEnabled());

    _pMockDataPointManager->injectBuildResult(QStringLiteral("${h0}"));

    QVERIFY(_pPanel->_pUi->btnAdd->isEnabled());
}

//! A field change disables Add until the new reference is built.
void TestSignalEditorPanel::addDisabledWhileBuildPending()
{
    _pMockDataPointManager->injectBuildResult(QStringLiteral("${h0}"));
    QVERIFY(_pPanel->_pUi->btnAdd->isEnabled());

    setAddress(QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding register") },
                            { QStringLiteral("address"), 5 } });

    QVERIFY(!_pPanel->_pUi->btnAdd->isEnabled());
    QCOMPARE(_pMockDataPointManager->buildCalls.size(), 2);
}

/*!
 * \brief Regression test: an adapter error for buildExpression used to stop the adapter. The form
 * must stay usable, so the user can correct the address and try again.
 */
void TestSignalEditorPanel::buildExpressionFailureKeepsFormUsable()
{
    _pMockDataPointManager->injectBuildFailure(QStringLiteral("Address out of range"));
    QTRY_VERIFY(!_pPanel->_pUi->lblStatus->isHidden());
    QVERIFY(!_pPanel->_pUi->btnAdd->isEnabled());

    setAddress(QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding register") },
                            { QStringLiteral("address"), 5 } });
    addSignal(QStringLiteral("${h5}"));

    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QStringLiteral("${h5}"));
}

//! The name field starts empty with a placeholder; the model names a signal added without a name.
void TestSignalEditorPanel::emptyNameAddsSignalWithDefaultLabel()
{
    QCOMPARE(_pPanel->_pUi->lineName->text(), QString());
    QVERIFY(!_pPanel->_pUi->lineName->placeholderText().isEmpty());

    addSignal(QStringLiteral("${h0}"));
    clickAdd();

    QCOMPARE(_pGraphDataModel->label(GraphIdx(0)), QStringLiteral("New signal"));
    QCOMPARE(_pGraphDataModel->label(GraphIdx(1)), QStringLiteral("New signal 2"));
}

//! The name field is cleared after a signal was added.
void TestSignalEditorPanel::nameClearedAfterAdd()
{
    QTest::keyClicks(_pPanel->_pUi->lineName, "Register 1");

    addSignal(QStringLiteral("${h0}"));

    QCOMPARE(_pGraphDataModel->label(GraphIdx(0)), QStringLiteral("Register 1"));
    QCOMPARE(_pPanel->_pUi->lineName->text(), QString());
}

//! After Add, device, address fields and axis stay, so the next signal only needs the changed fields.
void TestSignalEditorPanel::addKeepsDeviceFieldsAndAxis()
{
    QTest::mouseClick(_pPanel->_pUi->radioSecondary, Qt::LeftButton);
    setAddress(QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding register") },
                            { QStringLiteral("address"), 100 } });
    const qsizetype buildCallsBefore = _pMockDataPointManager->buildCalls.size();

    addSignal(QStringLiteral("${h100}"));

    QCOMPARE(addressForm(_pPanel)->values().value("address").toInt(), 100);
    QCOMPARE(deviceCombo(_pPanel)->selectedDeviceId(), Device::cFirstDeviceId);
    QVERIFY(_pPanel->_pUi->radioSecondary->isChecked());
    QCOMPARE(_pMockDataPointManager->buildCalls.size(), buildCallsBefore);
    QVERIFY(_pPanel->_pUi->btnAdd->isEnabled());
}

//! Several signals are added one after another without leaving the panel.
void TestSignalEditorPanel::consecutiveAddsWithoutReopening()
{
    for (int address = 0; address < 10; address++)
    {
        setAddress(QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding register") },
                                { QStringLiteral("address"), address } });
        addSignal(QString("${h%1}").arg(address));
    }

    QCOMPARE(_pGraphDataModel->size(), 10);
    for (int address = 0; address < 10; address++)
    {
        QCOMPARE(_pGraphDataModel->expression(GraphIdx(address)), QString("${h%1}").arg(address));
    }
}

void TestSignalEditorPanel::startAddClearsNameAndFocusesIt()
{
    _pPanel->show();
    QVERIFY(QTest::qWaitForWindowExposed(_pPanel));
    QTest::keyClicks(_pPanel->_pUi->lineName, "draft name");
    _pPanel->_pUi->radioPrimary->setFocus();

    _pPanel->startAdd();

    QCOMPARE(_pPanel->_pUi->lineName->text(), QString());
    /* The window's focus widget does not depend on the window being active */
    QCOMPARE(_pPanel->focusWidget(), _pPanel->_pUi->lineName);
}

//! Enter in the name field adds the signal, as clicking Add signal does.
void TestSignalEditorPanel::enterAddsSignal()
{
    _pMockDataPointManager->injectBuildResult(QStringLiteral("${h0}"));
    QTest::keyClicks(_pPanel->_pUi->lineName, "By enter");

    QTest::keyClick(_pPanel->_pUi->lineName, Qt::Key_Return);

    QCOMPARE(_pGraphDataModel->size(), 1);
    QCOMPARE(_pGraphDataModel->label(GraphIdx(0)), QStringLiteral("By enter"));
}

//! Enter in an address field adds the signal; the spin box reporting its value again keeps the reference.
void TestSignalEditorPanel::enterInAddressFieldAddsSignal()
{
    _pMockDataPointManager->injectBuildResult(QStringLiteral("${h0}"));

    auto* pAddressSpin = _pPanel->_pUi->addressContainer->findChild<QSpinBox*>();
    QVERIFY(pAddressSpin != nullptr);
    QTest::keyClick(pAddressSpin, Qt::Key_Return);

    QCOMPARE(_pMockDataPointManager->buildCalls.size(), 1);
    QCOMPARE(_pGraphDataModel->size(), 1);
    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QStringLiteral("${h0}"));
}

void TestSignalEditorPanel::enterIgnoredWhileBuildPending()
{
    QTest::keyClick(_pPanel->_pUi->lineName, Qt::Key_Return);

    QCOMPARE(_pGraphDataModel->size(), 0);
}

void TestSignalEditorPanel::statusNamesAddedSignal()
{
    QVERIFY(_pPanel->_pUi->lblStatus->isHidden());
    QTest::keyClicks(_pPanel->_pUi->lineName, "Pressure");

    addSignal(QStringLiteral("${h0}"));

    QVERIFY(!_pPanel->_pUi->lblStatus->isHidden());
    QCOMPARE(_pPanel->_pUi->lblStatus->text(), QStringLiteral("Added \"Pressure\""));
}

//! A build error after an add replaces the "Added" message; clearing the error leaves the line empty.
void TestSignalEditorPanel::buildErrorReplacesAddedStatus()
{
    addSignal(QStringLiteral("${h0}"));
    setAddress(QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding register") },
                            { QStringLiteral("address"), 5 } });

    _pMockDataPointManager->injectBuildFailure(QStringLiteral("Invalid address"));
    QTRY_VERIFY(_pPanel->_pUi->lblStatus->text().startsWith(QStringLiteral("Invalid address")));

    setAddress(QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding register") },
                            { QStringLiteral("address"), 6 } });

    QVERIFY(_pPanel->_pUi->lblStatus->isHidden());
}

//! Changing a field for the next signal clears the "Added" message.
void TestSignalEditorPanel::fieldChangeClearsAddedStatus()
{
    addSignal(QStringLiteral("${h0}"));
    QVERIFY(!_pPanel->_pUi->lblStatus->isHidden());

    setAddress(QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding register") },
                            { QStringLiteral("address"), 1 } });

    QVERIFY(_pPanel->_pUi->lblStatus->isHidden());
}

//! An empty build result is reported instead of silently ignored.
void TestSignalEditorPanel::emptyBuildResultShowsReason()
{
    _pMockDataPointManager->injectBuildResult(QString());

    QVERIFY(!_pPanel->_pUi->lblStatus->isHidden());
    QVERIFY(!_pPanel->_pUi->lblStatus->text().isEmpty());
    QVERIFY(!_pPanel->_pUi->btnAdd->isEnabled());
}

//! A failure without a reason still tells the user the address was not accepted.
void TestSignalEditorPanel::buildFailureWithoutReasonShowsFallback()
{
    _pMockDataPointManager->injectBuildFailure(QString());

    QTRY_VERIFY(!_pPanel->_pUi->lblStatus->isHidden());
    QVERIFY(!_pPanel->_pUi->lblStatus->text().isEmpty());
}

//! The status line shows adapter text as plain text.
void TestSignalEditorPanel::statusIsPlainText()
{
    QCOMPARE(_pPanel->_pUi->lblStatus->textFormat(), Qt::PlainText);
}

//! A field change after a failure hides the failure reason.
void TestSignalEditorPanel::fieldChangeHidesBuildError()
{
    _pMockDataPointManager->injectBuildFailure(QStringLiteral("Invalid address"));
    QTRY_VERIFY(!_pPanel->_pUi->lblStatus->isHidden());

    setAddress(QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding register") },
                            { QStringLiteral("address"), 5 } });

    QVERIFY(_pPanel->_pUi->lblStatus->isHidden());
    QCOMPARE(_pPanel->_pUi->lblStatus->text(), QString());
}

//! Selecting another device hides the failure reason.
void TestSignalEditorPanel::deviceSwitchHidesBuildError()
{
    const deviceId_t simDeviceId = addSimAdapter();

    _pMockDataPointManager->injectBuildFailure(QStringLiteral("Invalid address"));
    QTRY_VERIFY(!_pPanel->_pUi->lblStatus->isHidden());

    deviceCombo(_pPanel)->setSelectedDeviceId(simDeviceId);

    QVERIFY(_pPanel->_pUi->lblStatus->isHidden());
}

void TestSignalEditorPanel::noDeviceDisablesAddWithHint()
{
    DeviceListHelpers::clearDevices(&_settingsModel);
    recreatePanel();

    QVERIFY(!_pPanel->_pUi->btnAdd->isEnabled());
    QVERIFY(!_pPanel->_pUi->lblStatus->isHidden());
    QVERIFY(_pPanel->_pUi->lblStatus->text().contains(QStringLiteral("device")));
}

void TestSignalEditorPanel::buildExpressionRoutedToSelectedDevice()
{
    const deviceId_t simDeviceId = addSimAdapter();
    const qsizetype modbusCallsBefore = _pMockDataPointManager->buildCalls.size();

    deviceCombo(_pPanel)->setSelectedDeviceId(simDeviceId);

    QCOMPARE(_pMockSimAdapterManager->buildCalls.size(), 1);
    QCOMPARE(_pMockDataPointManager->buildCalls.size(), modbusCallsBefore);
    QCOMPARE(_pMockSimAdapterManager->buildCalls[0].deviceId, simDeviceId);

    _pMockSimAdapterManager->injectBuildResult(QStringLiteral("${c3}"));
    clickAdd();

    QCOMPARE(_pGraphDataModel->size(), 1);
    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QStringLiteral("${c3}"));
}

void TestSignalEditorPanel::btnAddDisabledWhenSelectedDeviceAdapterUnavailable()
{
    /* Bind a second device to an adapter id that is never registered in the mock hub */
    const deviceId_t unavailableDeviceId =
      DeviceListHelpers::seedNextDevice(&_settingsModel, QStringLiteral("unavailable"));
    recreatePanel();

    /* Device 1 is initially selected and it belongs to the modbus adapter */
    _pMockDataPointManager->injectBuildResult(QStringLiteral("${h0}"));
    QVERIFY(_pPanel->_pUi->btnAdd->isEnabled());

    deviceCombo(_pPanel)->setSelectedDeviceId(unavailableDeviceId);
    QVERIFY(!_pPanel->_pUi->btnAdd->isEnabled());
    QTRY_VERIFY(!_pPanel->_pUi->lblStatus->isHidden());

    deviceCombo(_pPanel)->setSelectedDeviceId(Device::cFirstDeviceId);
    _pMockDataPointManager->injectBuildResult(QStringLiteral("${h0}"));
    QVERIFY(_pPanel->_pUi->btnAdd->isEnabled());
}

void TestSignalEditorPanel::deviceAddedToEmptyListAppliesConfiguredAdapter()
{
    _settingsModel.setAdapterDataPointSchema("sim", buildSimRegisterSchema());
    QJsonObject describeResult;
    describeResult["name"] = QStringLiteral("Simulator");
    _settingsModel.updateAdapterFromDescribe(QStringLiteral("sim"), describeResult);
    _pMockSimAdapterManager = new MockDataPointManager(QStringLiteral("sim"), &_settingsModel);
    _pMockHub->addManager(QStringLiteral("sim"), _pMockSimAdapterManager);

    /* Start from an empty device list, so the device added below becomes the selected one
     * and the combo refresh has to build its address form from the sim adapter. */
    DeviceListHelpers::clearDevices(&_settingsModel);
    recreatePanel();
    QCOMPARE(deviceCombo(_pPanel)->count(), 0);

    const deviceId_t simDeviceId = Device::cFirstDeviceId;
    DeviceListHelpers::seedDevice(&_settingsModel, simDeviceId, QStringLiteral("sim"));

    /* The adapter must already be assigned when deviceListChanged() arrives: assigning it
     * afterwards left the widget showing Device's default ("modbus") adapter instead. */
    QCOMPARE(deviceCombo(_pPanel)->count(), 1);
    QVERIFY(hasLabelWithText(QStringLiteral("Protocol: Simulator")));
    QCOMPARE(addressForm(_pPanel)->values().value("channel").toInt(), 3);

    _pMockSimAdapterManager->injectBuildResult(QStringLiteral("${c3}"));
    QVERIFY(_pPanel->_pUi->btnAdd->isEnabled());
    QVERIFY(_pPanel->_pUi->lblStatus->isHidden());
}

void TestSignalEditorPanel::deviceComboDisablesAddWhenLastDeviceRemoved()
{
    _pMockDataPointManager->injectBuildResult(QStringLiteral("${h0}"));
    QVERIFY(_pPanel->_pUi->btnAdd->isEnabled());

    DeviceListHelpers::dropDevice(&_settingsModel, Device::cFirstDeviceId);

    QCOMPARE(deviceCombo(_pPanel)->count(), 0);
    QVERIFY(!_pPanel->_pUi->btnAdd->isEnabled());
    QVERIFY(!_pPanel->_pUi->lblStatus->isHidden());
}

//! Edit mode asks for the description and keeps the editor disabled until it arrives.
void TestSignalEditorPanel::editModeLoadsReferenceWhenDescribed()
{
    editNewSignal(QStringLiteral("${h4}"));

    QCOMPARE(_pPanel->_pUi->lblTitle->text(), QStringLiteral("Edit signal"));
    QVERIFY(_pPanel->_pUi->btnAdd->isHidden());
    QCOMPARE(_pPanel->_pUi->lineName->text(), QStringLiteral("Signal"));
    QCOMPARE(_pMockDataPointManager->describeCalls, QStringList{ QStringLiteral("${h4}") });
    QVERIFY(!_pPanel->_pEditor->isEnabled());

    describeCall(0, holdingRegisterFields(4));

    QVERIFY(_pPanel->_pEditor->isEnabled());
    QCOMPARE(addressForm(_pPanel)->values().value("address").toInt(), 4);
    QCOMPARE(_pPanel->_pEditor->expression(), QStringLiteral("${h4}"));
    QVERIFY(_pPanel->_pUi->lblExpression->isHidden());
    QVERIFY(_pPanel->_pUi->listDataPoints->isHidden());
}

void TestSignalEditorPanel::editAddressOfSingleReference()
{
    editNewSignal(QStringLiteral("${40001}"));
    describeCall(0, holdingRegisterFields(0));

    setAddress(holdingRegisterFields(5));
    QCOMPARE(_pMockDataPointManager->buildCalls.last().fields["address"].toInt(), 5);
    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QStringLiteral("${40001}"));

    _pMockDataPointManager->injectBuildResult(QStringLiteral("${h5}"));

    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QStringLiteral("${h5}"));
    QCOMPARE(_pGraphDataModel->size(), 1);

    /* The panel's own write keeps the form usable */
    QVERIFY(_pPanel->_pEditor->isEnabled());
    QCOMPARE(addressForm(_pPanel)->values().value("address").toInt(), 5);
}

void TestSignalEditorPanel::editKeepsArithmeticAroundReference()
{
    editNewSignal(QStringLiteral("${40001}/2"));
    describeCall(0, holdingRegisterFields(0));

    QVERIFY(!_pPanel->_pUi->lblExpression->isHidden());
    QCOMPARE(_pPanel->_pUi->lblExpression->text(),
             QStringLiteral("<span style=\"white-space: pre-wrap\"><b>${40001}</b>/2</span>"));

    setAddress(holdingRegisterFields(5));
    _pMockDataPointManager->injectBuildResult(QStringLiteral("${h5}"));

    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QStringLiteral("${h5}/2"));
    QCOMPARE(_pPanel->_pUi->lblExpression->text(),
             QStringLiteral("<span style=\"white-space: pre-wrap\"><b>${h5}</b>/2</span>"));
}

//! Only the selected occurrence of a data point used twice is replaced, and it stays selected.
void TestSignalEditorPanel::editSecondOfTwoIdenticalReferences()
{
    editNewSignal(QStringLiteral("${h0} - ${h0}"));
    describeCall(0, holdingRegisterFields(0));

    QVERIFY(!_pPanel->_pUi->listDataPoints->isHidden());
    QCOMPARE(_pPanel->_pUi->listDataPoints->count(), 2);

    _pPanel->_pUi->listDataPoints->setCurrentRow(1);
    setAddress(holdingRegisterFields(5));
    _pMockDataPointManager->injectBuildResult(QStringLiteral("${h5}"));

    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QStringLiteral("${h0} - ${h5}"));
    QCOMPARE(_pPanel->_pUi->listDataPoints->currentRow(), 1);

    setAddress(holdingRegisterFields(6));
    _pMockDataPointManager->injectBuildResult(QStringLiteral("${h6}"));

    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QStringLiteral("${h0} - ${h6}"));
}

void TestSignalEditorPanel::selectingDataPointLoadsIt()
{
    editNewSignal(QStringLiteral("${h3}*10 + ${h7}"));
    QCOMPARE(_pMockDataPointManager->describeCalls.size(), 2);
    describeCall(0, holdingRegisterFields(3));
    describeCall(1, holdingRegisterFields(7));

    QCOMPARE(_pPanel->_pUi->lblDataPoints->text(), QStringLiteral("This signal combines 2 data points."));
    QCOMPARE(_pPanel->_pUi->listDataPoints->item(1)->text(), QStringLiteral("${h7}  described ${h7}"));
    QCOMPARE(addressForm(_pPanel)->values().value("address").toInt(), 3);

    _pPanel->_pUi->listDataPoints->setCurrentRow(1);

    QCOMPARE(_pPanel->_pEditor->expression(), QStringLiteral("${h7}"));
    QCOMPARE(addressForm(_pPanel)->values().value("address").toInt(), 7);
    QCOMPARE(_pPanel->_pUi->lblExpression->text(),
             QStringLiteral("<span style=\"white-space: pre-wrap\">${h3}*10 + <b>${h7}</b></span>"));
}

void TestSignalEditorPanel::syntaxErrorShowsOnlyErrorAndEditExpression()
{
    GraphData graphData;
    graphData.setExpression(QStringLiteral("${h0} +"));
    _pGraphDataModel->add(graphData);
    _pGraphDataModel->setExpressionState(GraphIdx(0), GraphData::ExpressionState::SYNTAX_ERROR,
                                         QStringLiteral("Unexpected end of expression"));

    _pPanel->editSignal(GraphIdx(0));

    QVERIFY(_pPanel->_pUi->addressContainer->isHidden());
    QVERIFY(_pPanel->_pUi->listDataPoints->isHidden());
    QVERIFY(_pPanel->_pUi->lblExpression->isHidden());
    QVERIFY(!_pPanel->_pUi->btnEditExpression->isHidden());
    QCOMPARE(_pPanel->_pUi->lblStatus->text(), QStringLiteral("Unexpected end of expression"));
    QVERIFY(_pMockDataPointManager->describeCalls.isEmpty());
}

void TestSignalEditorPanel::editExpressionRequestsDialogForRow()
{
    editNewSignal(QStringLiteral("${h0}"));
    editNewSignal(QStringLiteral("${h1}"));
    QSignalSpy spy(_pPanel, &SignalEditorPanel::editExpressionRequested);

    QTest::mouseClick(_pPanel->_pUi->btnEditExpression, Qt::LeftButton);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().value<GraphIdx>(), GraphIdx(1));
}

void TestSignalEditorPanel::nameAndAxisEditRow()
{
    editNewSignal(QStringLiteral("${h0}"));

    _pPanel->_pUi->lineName->selectAll();
    QTest::keyClicks(_pPanel->_pUi->lineName, "Pressure");
    QTest::mouseClick(_pPanel->_pUi->radioSecondary, Qt::LeftButton);

    QCOMPARE(_pGraphDataModel->label(GraphIdx(0)), QStringLiteral("Pressure"));
    QCOMPARE(_pGraphDataModel->valueAxis(GraphIdx(0)), GraphData::VALUE_AXIS_SECONDARY);
    QCOMPARE(_pGraphDataModel->size(), 1);
}

void TestSignalEditorPanel::emptyNameRestoresLabel()
{
    editNewSignal(QStringLiteral("${h0}"));

    _pPanel->_pUi->lineName->selectAll();
    QTest::keyClick(_pPanel->_pUi->lineName, Qt::Key_Backspace);
    QCOMPARE(_pGraphDataModel->label(GraphIdx(0)), QStringLiteral("Signal"));

    emit _pPanel->_pUi->lineName->editingFinished();

    QCOMPARE(_pPanel->_pUi->lineName->text(), QStringLiteral("Signal"));
}

void TestSignalEditorPanel::enterInEditModeAddsNothing()
{
    editNewSignal(QStringLiteral("${h0}"));
    describeCall(0, holdingRegisterFields(0));

    QTest::keyClick(_pPanel->_pUi->lineName, Qt::Key_Return);

    QCOMPARE(_pGraphDataModel->size(), 1);
}

//! A device on another adapter resets the form to that adapter's defaults and replaces the reference.
void TestSignalEditorPanel::deviceSwitchReplacesReferenceWithDefault()
{
    const deviceId_t simDeviceId = addSimAdapter();
    editNewSignal(QStringLiteral("${h0}*2"));
    describeCall(0, holdingRegisterFields(0));

    deviceCombo(_pPanel)->setSelectedDeviceId(simDeviceId);
    QCOMPARE(_pMockSimAdapterManager->buildCalls.last().fields.value("channel").toInt(), 3);

    _pMockSimAdapterManager->injectBuildResult(QStringLiteral("${c3@2}"));

    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QStringLiteral("${c3@2}*2"));
}

void TestSignalEditorPanel::removedRowReturnsToAddMode()
{
    editNewSignal(QStringLiteral("${h0}"));

    _pGraphDataModel->removeSignals({ GraphIdx(0) });

    QCOMPARE(_pPanel->_pUi->lblTitle->text(), QStringLiteral("New signal"));
    QVERIFY(!_pPanel->_pUi->btnAdd->isHidden());
    QVERIFY(_pPanel->_pUi->btnEditExpression->isHidden());
}

//! An expression changed elsewhere, for example in Compose expression, is loaded again.
void TestSignalEditorPanel::outsideExpressionChangeReloadsPanel()
{
    editNewSignal(QStringLiteral("${h0}"));
    describeCall(0, holdingRegisterFields(0));

    _pGraphDataModel->setExpression(GraphIdx(0), QStringLiteral("${h7} + ${h8}"));

    QCOMPARE(_pPanel->_pUi->listDataPoints->count(), 2);
    QCOMPARE(_pPanel->_pEditor->expression(), QStringLiteral("${h7}"));
    QVERIFY(!_pPanel->_pEditor->isEnabled());
    QVERIFY(_pMockDataPointManager->describeCalls.contains(QStringLiteral("${h7}")));
}

void TestSignalEditorPanel::showAddModeClearsNameAndShowsAdd()
{
    editNewSignal(QStringLiteral("${h0}"));
    QCOMPARE(_pPanel->_pUi->lineName->text(), QStringLiteral("Signal"));

    _pPanel->showAddMode();

    QCOMPARE(_pPanel->_pUi->lineName->text(), QString());
    QCOMPARE(_pPanel->_pUi->lblTitle->text(), QStringLiteral("New signal"));
    QVERIFY(!_pPanel->_pUi->btnAdd->isHidden());
    QVERIFY(_pPanel->_pEditor->isEnabled());
    QVERIFY(_pPanel->_pUi->listDataPoints->isHidden());
}

//! The description of the reference the panel just wrote does not rebuild the form being edited.
void TestSignalEditorPanel::descriptionAfterOwnWriteKeepsForm()
{
    editNewSignal(QStringLiteral("${h0}"));
    describeCall(0, holdingRegisterFields(0));

    setAddress(holdingRegisterFields(5));
    _pMockDataPointManager->injectBuildResult(QStringLiteral("${h5}"));
    const qsizetype describeIdx = _pMockDataPointManager->describeCalls.indexOf(QStringLiteral("${h5}"));
    QVERIFY(describeIdx >= 0);
    auto* pAddressSpin = _pPanel->_pUi->addressContainer->findChild<QSpinBox*>();
    QVERIFY(pAddressSpin != nullptr);
    const QPointer<QSpinBox> spinBefore(pAddressSpin);

    describeCall(static_cast<int>(describeIdx), holdingRegisterFields(5));

    QVERIFY(!spinBefore.isNull());
    QCOMPARE(_pPanel->_pUi->addressContainer->findChild<QSpinBox*>(), spinBefore.data());
    QVERIFY(_pPanel->_pEditor->isEnabled());
}

void TestSignalEditorPanel::expressionLabelIsEscaped()
{
    editNewSignal(QStringLiteral("${h0} < 2 & 1"));

    QCOMPARE(_pPanel->_pUi->lblExpression->text(),
             QStringLiteral("<span style=\"white-space: pre-wrap\"><b>${h0}</b> &lt; 2 &amp; 1</span>"));
}

void TestSignalEditorPanel::expressionWithoutDataPointShowsHint()
{
    editNewSignal(QStringLiteral("5"));

    QVERIFY(_pPanel->_pUi->addressContainer->isHidden());
    QVERIFY(!_pPanel->_pUi->btnEditExpression->isHidden());
    QCOMPARE(_pPanel->_pUi->lblStatus->text(), QStringLiteral("This signal reads no data point."));
}

void TestSignalEditorPanel::clickAdd()
{
    QTest::mouseClick(_pPanel->_pUi->btnAdd, Qt::LeftButton);
}

/*!
 * \brief Answer the pending build request with \a expression and click Add signal.
 */
void TestSignalEditorPanel::addSignal(const QString& expression)
{
    const qint32 sizeBefore = _pGraphDataModel->size();

    /* Simulate the adapter returning the expression string */
    _pMockDataPointManager->injectBuildResult(expression);

    clickAdd();

    QCOMPARE(_pGraphDataModel->size(), sizeBefore + 1);
}

//! Returns whether the panel shows a label with exactly \a text.
bool TestSignalEditorPanel::hasLabelWithText(const QString& text) const
{
    const QList<QLabel*> labels = _pPanel->findChildren<QLabel*>();
    for (const QLabel* pLabel : labels)
    {
        if (pLabel->text() == text)
        {
            return true;
        }
    }
    return false;
}

QTEST_MAIN(TestSignalEditorPanel)
