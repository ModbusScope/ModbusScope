
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
    objectTypeSchema["enum"] = QJsonArray{ QStringLiteral("coil"), QStringLiteral("discrete-input"),
                                           QStringLiteral("input-register"), QStringLiteral("holding-register") };
    objectTypeSchema["x-enumLabels"] =
      QJsonArray{ QStringLiteral("Coil"), QStringLiteral("Discrete Input"), QStringLiteral("Input Register"),
                  QStringLiteral("Holding Register") };

    QJsonObject addressField;
    addressField["type"] = QStringLiteral("integer");
    addressField["title"] = QStringLiteral("Address");
    addressField["minimum"] = 0;
    addressField["maximum"] = 65535;

    QJsonObject properties;
    properties["objectType"] = objectTypeSchema;
    properties["address"] = addressField;

    QJsonObject schema;
    schema["type"] = QStringLiteral("object");
    schema["properties"] = properties;
    schema["required"] = QJsonArray{ QStringLiteral("objectType"), QStringLiteral("address") };
    return schema;
}

QJsonObject TestAddRegisterWidget::buildTestRegisterSchema()
{
    QJsonArray dataTypes;
    dataTypes.append(QJsonObject{ { QStringLiteral("id"), QStringLiteral("16b") },
                                  { QStringLiteral("label"), QStringLiteral("Unsigned 16-bit") } });
    dataTypes.append(QJsonObject{ { QStringLiteral("id"), QStringLiteral("s16b") },
                                  { QStringLiteral("label"), QStringLiteral("Signed 16-bit") } });
    dataTypes.append(QJsonObject{ { QStringLiteral("id"), QStringLiteral("32b") },
                                  { QStringLiteral("label"), QStringLiteral("Unsigned 32-bit") } });
    dataTypes.append(QJsonObject{ { QStringLiteral("id"), QStringLiteral("s32b") },
                                  { QStringLiteral("label"), QStringLiteral("Signed 32-bit") } });
    dataTypes.append(QJsonObject{ { QStringLiteral("id"), QStringLiteral("f32b") },
                                  { QStringLiteral("label"), QStringLiteral("32-bit float") } });

    QJsonObject schema;
    schema["addressSchema"] = buildAddressSchema();
    schema["dataTypes"] = dataTypes;
    schema["defaultDataType"] = QStringLiteral("16b");
    return schema;
}

void TestAddRegisterWidget::init()
{
    _settingsModel.removeAllDevice();
    _settingsModel.setAdapterRegisterSchema("modbus", buildTestRegisterSchema());
    _settingsModel.deviceSettings(Device::cFirstDeviceId)->setAdapterId("modbus");

    _pMockAdapterManager = new MockAdapterManager(&_settingsModel);
    _pRegWidget = new AddRegisterWidget(&_settingsModel, QStringLiteral("modbus"), _pMockAdapterManager);
}

void TestAddRegisterWidget::cleanup()
{
    delete _pRegWidget;
    _pRegWidget = nullptr;
    delete _pMockAdapterManager;
    _pMockAdapterManager = nullptr;
}

void TestAddRegisterWidget::registerDefault()
{
    _pRegWidget->_pUi->lineName->selectAll();
    QTest::keyClicks(_pRegWidget->_pUi->lineName, "Register 1");

    _pRegWidget->_pAddressForm->setSchema(
      buildAddressSchema(), QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding-register") },
                                         { QStringLiteral("address"), 100 } });

    GraphData graphData;
    addRegister(graphData, QStringLiteral("${h100}"));

    QCOMPARE(graphData.label(), QStringLiteral("Register 1"));
    QCOMPARE(graphData.expression(), QStringLiteral("${h100}"));
    QVERIFY(graphData.isActive());

    QCOMPARE(_pMockAdapterManager->buildCalls.size(), 1);
    QCOMPARE(_pMockAdapterManager->buildCalls[0].fields["objectType"].toString(), QStringLiteral("holding-register"));
    QCOMPARE(_pMockAdapterManager->buildCalls[0].fields["address"].toInt(), 100);
    QCOMPARE(_pMockAdapterManager->buildCalls[0].dataType, QStringLiteral("16b"));
    QCOMPARE(_pMockAdapterManager->buildCalls[0].deviceId, Device::cFirstDeviceId);
}

void TestAddRegisterWidget::registerType()
{
    _pRegWidget->_pAddressForm->setSchema(
      buildAddressSchema(), QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding-register") },
                                         { QStringLiteral("address"), 0 } });

    /* Select "32b" (index 2 in the combo: 16b, s16b, 32b, ...) */
    _pRegWidget->_pUi->cmbType->setCurrentIndex(2);

    GraphData graphData;
    addRegister(graphData, QStringLiteral("${h0:32b}"));

    QCOMPARE(graphData.expression(), QStringLiteral("${h0:32b}"));
    QCOMPARE(_pMockAdapterManager->buildCalls[0].dataType, QStringLiteral("32b"));
}

void TestAddRegisterWidget::registerObjectType()
{
    _pRegWidget->_pAddressForm->setSchema(
      buildAddressSchema(), QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("input-register") },
                                         { QStringLiteral("address"), 0 } });

    GraphData graphData;
    addRegister(graphData, QStringLiteral("${i0}"));

    QCOMPARE(graphData.expression(), QStringLiteral("${i0}"));
    QCOMPARE(_pMockAdapterManager->buildCalls[0].fields["objectType"].toString(), QStringLiteral("input-register"));
}

void TestAddRegisterWidget::registerDevice()
{
    delete _pRegWidget;
    _pRegWidget = nullptr;

    const deviceId_t devId2 = _settingsModel.addNewDevice();
    _settingsModel.deviceSettings(devId2)->setAdapterId("modbus");

    _pRegWidget = new AddRegisterWidget(&_settingsModel, QStringLiteral("modbus"), _pMockAdapterManager);

    _pRegWidget->_pAddressForm->setSchema(
      buildAddressSchema(), QJsonObject{ { QStringLiteral("objectType"), QStringLiteral("holding-register") },
                                         { QStringLiteral("address"), 0 } });

    /* Select device 2 (index 1) */
    _pRegWidget->_pUi->cmbDevice->setCurrentIndex(1);

    GraphData graphData;
    addRegister(graphData, QStringLiteral("${h0@2}"));

    QCOMPARE(graphData.expression(), QStringLiteral("${h0@2}"));
    QCOMPARE(_pMockAdapterManager->buildCalls[0].deviceId, devId2);
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
