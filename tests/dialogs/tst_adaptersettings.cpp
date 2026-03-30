#include "tst_adaptersettings.h"

#include "customwidgets/addabletabwidget.h"
#include "customwidgets/schemaformwidget.h"
#include "dialogs/adaptersettings.h"
#include "models/settingsmodel.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QTest>

namespace {

//! Build a describe result with a single top-level property \a key of type
//! "array" whose items have the given \a itemProps, or of type "object" with
//! \a itemProps as properties directly.
QJsonObject makeDescribeResult(const QString& key,
                               const QString& propertyType,
                               const QJsonObject& itemProps = QJsonObject())
{
    QJsonObject propSchema;
    if (propertyType == "array")
    {
        QJsonObject itemSchema;
        itemSchema["type"] = "object";
        itemSchema["properties"] = itemProps;
        propSchema["type"] = "array";
        propSchema["items"] = itemSchema;
    }
    else
    {
        propSchema["type"] = "object";
        propSchema["properties"] = itemProps;
    }

    QJsonObject topProps;
    topProps[key] = propSchema;

    QJsonObject schema;
    schema["type"] = "object";
    schema["properties"] = topProps;

    QJsonObject describe;
    describe["name"] = "testAdapter";
    describe["version"] = "1.0.0";
    describe["configVersion"] = 1;
    describe["schema"] = schema;
    describe["defaults"] = QJsonObject();
    describe["capabilities"] = QJsonObject();
    return describe;
}

} // namespace

void TestAdapterSettings::isRenderableArrayWithItems()
{
    QJsonObject itemSchema;
    itemSchema["type"] = "object";
    itemSchema["properties"] = QJsonObject{ { "port", QJsonObject{ { "type", "integer" } } } };

    QJsonObject propSchema;
    propSchema["type"] = "array";
    propSchema["items"] = itemSchema;

    QVERIFY(AdapterSettings::isRenderableProperty(propSchema));
}

void TestAdapterSettings::isRenderableArrayWithoutItems()
{
    QJsonObject propSchema;
    propSchema["type"] = "array";
    propSchema["items"] = QJsonObject();

    QVERIFY(!AdapterSettings::isRenderableProperty(propSchema));
}

void TestAdapterSettings::isRenderableObjectWithProperties()
{
    QJsonObject propSchema;
    propSchema["type"] = "object";
    propSchema["properties"] = QJsonObject{ { "timeout", QJsonObject{ { "type", "integer" } } } };

    QVERIFY(AdapterSettings::isRenderableProperty(propSchema));
}

void TestAdapterSettings::isRenderableObjectWithoutProperties()
{
    QJsonObject propSchema;
    propSchema["type"] = "object";
    propSchema["properties"] = QJsonObject();

    QVERIFY(!AdapterSettings::isRenderableProperty(propSchema));
}

void TestAdapterSettings::isRenderableScalarReturnsFalse()
{
    QJsonObject propSchema;
    propSchema["type"] = "integer";

    QVERIFY(!AdapterSettings::isRenderableProperty(propSchema));
}

void TestAdapterSettings::arrayPropertyCreatesTabWidget()
{
    SettingsModel model;

    QJsonObject hostProp;
    hostProp["type"] = "string";
    QJsonObject itemProps;
    itemProps["host"] = hostProp;

    model.updateAdapterFromDescribe("testAdapter", makeDescribeResult("connections", "array", itemProps));

    QJsonObject conn0;
    conn0["host"] = "192.168.0.1";
    QJsonObject conn1;
    conn1["host"] = "10.0.0.1";
    QJsonObject config;
    config["connections"] = QJsonArray{ conn0, conn1 };
    model.setAdapterCurrentConfig("testAdapter", config);

    AdapterSettings w(&model, "testAdapter", "connections");

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 2);
}

void TestAdapterSettings::objectPropertyCreatesSingleForm()
{
    SettingsModel model;

    QJsonObject settingProp;
    settingProp["type"] = "string";
    QJsonObject props;
    props["setting"] = settingProp;

    model.updateAdapterFromDescribe("testAdapter", makeDescribeResult("general", "object", props));

    AdapterSettings w(&model, "testAdapter", "general");

    QVERIFY(w.findChild<AddableTabWidget*>() == nullptr);
    QVERIFY(w.findChild<SchemaFormWidget*>() != nullptr);
}

void TestAdapterSettings::addTabUsesPropertyDefaults()
{
    SettingsModel model;

    QJsonObject portProp;
    portProp["type"] = "integer";
    portProp["title"] = "Port";
    QJsonObject itemProps;
    itemProps["port"] = portProp;

    QJsonObject describe = makeDescribeResult("connections", "array", itemProps);

    QJsonObject defaultConn;
    defaultConn["port"] = 502;
    QJsonObject defaults;
    defaults["connections"] = QJsonArray{ defaultConn };
    describe["defaults"] = defaults;

    model.updateAdapterFromDescribe("testAdapter", describe);

    QJsonObject config;
    config["connections"] = QJsonArray();
    model.setAdapterCurrentConfig("testAdapter", config);

    AdapterSettings w(&model, "testAdapter", "connections");

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 0);

    emit tabs->addTabRequested();

    QCOMPARE(tabs->count(), 1);
    auto* form = qobject_cast<SchemaFormWidget*>(tabs->tabContent(0));
    QVERIFY(form != nullptr);
    QCOMPARE(form->values().value("port").toInt(), 502);
}

void TestAdapterSettings::acceptValuesStoresConfigInAdapterData()
{
    SettingsModel model;

    QJsonObject hostProp;
    hostProp["type"] = "string";
    QJsonObject itemProps;
    itemProps["host"] = hostProp;

    model.updateAdapterFromDescribe("testAdapter", makeDescribeResult("connections", "array", itemProps));

    QJsonObject conn0;
    conn0["host"] = "original";
    QJsonObject config;
    config["connections"] = QJsonArray{ conn0 };
    model.setAdapterCurrentConfig("testAdapter", config);

    AdapterSettings w(&model, "testAdapter", "connections");

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    auto* form = qobject_cast<SchemaFormWidget*>(tabs->tabContent(0));
    QVERIFY(form != nullptr);
    auto* edit = form->findChild<QLineEdit*>();
    QVERIFY(edit != nullptr);
    edit->setText("192.168.1.1");

    w.acceptValues();

    const AdapterData* adapter = model.adapterData("testAdapter");
    QCOMPARE(adapter->hasStoredConfig(), true);
    QCOMPARE(adapter->currentConfig().value("connections").toArray().at(0).toObject().value("host").toString(),
             QStringLiteral("192.168.1.1"));
}

QTEST_MAIN(TestAdapterSettings)
