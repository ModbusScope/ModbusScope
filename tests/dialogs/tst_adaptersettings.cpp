#include "tst_adaptersettings.h"

#include "customwidgets/addabletabwidget.h"
#include "customwidgets/schemaformwidget.h"
#include "dialogs/adaptersettings.h"
#include "models/settingsmodel.h"

#include <QComboBox>
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

void TestAdapterSettings::addTabUsesNextIndexForId()
{
    SettingsModel model;

    QJsonObject idProp;
    idProp["type"] = "integer";
    idProp["title"] = "ID";
    QJsonObject portProp;
    portProp["type"] = "integer";
    portProp["title"] = "Port";
    QJsonObject itemProps;
    itemProps["id"] = idProp;
    itemProps["port"] = portProp;

    QJsonObject describe = makeDescribeResult("connections", "array", itemProps);

    QJsonObject defaultConn;
    defaultConn["id"] = 1;
    defaultConn["port"] = 502;
    QJsonObject defaults;
    defaults["connections"] = QJsonArray{ defaultConn };
    describe["defaults"] = defaults;

    model.updateAdapterFromDescribe("testAdapter", describe);

    QJsonObject conn0;
    conn0["id"] = 1;
    conn0["port"] = 502;
    QJsonObject conn1;
    conn1["id"] = 2;
    conn1["port"] = 503;
    QJsonObject config;
    config["connections"] = QJsonArray{ conn0, conn1 };
    model.setAdapterCurrentConfig("testAdapter", config);

    AdapterSettings w(&model, "testAdapter", "connections");

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 2);

    emit tabs->addTabRequested();

    QCOMPARE(tabs->count(), 3);
    auto* form = qobject_cast<SchemaFormWidget*>(tabs->tabContent(2));
    QVERIFY(form != nullptr);
    QCOMPARE(form->values().value("id").toInt(), 3);
}

void TestAdapterSettings::addTabAssignsMaxIdPlusOneForNonContiguousIds()
{
    SettingsModel model;

    QJsonObject idProp;
    idProp["type"] = "integer";
    idProp["title"] = "ID";
    QJsonObject portProp;
    portProp["type"] = "integer";
    portProp["title"] = "Port";
    QJsonObject itemProps;
    itemProps["id"] = idProp;
    itemProps["port"] = portProp;

    QJsonObject describe = makeDescribeResult("connections", "array", itemProps);

    QJsonObject defaultConn;
    defaultConn["id"] = 1;
    defaultConn["port"] = 502;
    QJsonObject defaults;
    defaults["connections"] = QJsonArray{ defaultConn };
    describe["defaults"] = defaults;

    model.updateAdapterFromDescribe("testAdapter", describe);

    QJsonObject conn0;
    conn0["id"] = 1;
    conn0["port"] = 502;
    QJsonObject conn1;
    conn1["id"] = 3;
    conn1["port"] = 503;
    QJsonObject config;
    config["connections"] = QJsonArray{ conn0, conn1 };
    model.setAdapterCurrentConfig("testAdapter", config);

    AdapterSettings w(&model, "testAdapter", "connections");

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 2);

    emit tabs->addTabRequested();

    QCOMPARE(tabs->count(), 3);
    auto* form = qobject_cast<SchemaFormWidget*>(tabs->tabContent(2));
    QVERIFY(form != nullptr);
    QCOMPARE(form->values().value("id").toInt(), 4);
}

void TestAdapterSettings::addTabNameMatchesIdForNonContiguousIds()
{
    SettingsModel model;

    QJsonObject idProp;
    idProp["type"] = "integer";
    idProp["title"] = "ID";
    QJsonObject nameProp;
    nameProp["type"] = "string";
    nameProp["title"] = "Connection Name";
    QJsonObject itemProps;
    itemProps["id"] = idProp;
    itemProps["name"] = nameProp;

    QJsonObject describe = makeDescribeResult("connections", "array", itemProps);

    QJsonObject defaultConn;
    defaultConn["id"] = 1;
    defaultConn["name"] = "Connection 1";
    QJsonObject defaults;
    defaults["connections"] = QJsonArray{ defaultConn };
    describe["defaults"] = defaults;

    model.updateAdapterFromDescribe("testAdapter", describe);

    QJsonObject conn0;
    conn0["id"] = 1;
    conn0["name"] = "Connection 1";
    QJsonObject conn1;
    conn1["id"] = 3;
    conn1["name"] = "Connection 3";
    QJsonObject config;
    config["connections"] = QJsonArray{ conn0, conn1 };
    model.setAdapterCurrentConfig("testAdapter", config);

    AdapterSettings w(&model, "testAdapter", "connections");

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 2);

    emit tabs->addTabRequested();

    QCOMPARE(tabs->count(), 3);
    auto* form = qobject_cast<SchemaFormWidget*>(tabs->tabContent(2));
    QVERIFY(form != nullptr);

    // ID must be maxId+1=4, and the default name and tab title must match.
    QCOMPARE(form->values().value("id").toInt(), 4);
    QCOMPARE(form->values().value("name").toString(), QStringLiteral("Connection 4"));
    QCOMPARE(tabs->tabText(2), QStringLiteral("Connection 4"));
}

void TestAdapterSettings::addTabNameMatchesIdForConsecutiveAddsWithNonContiguousStart()
{
    SettingsModel model;

    QJsonObject idProp;
    idProp["type"] = "integer";
    idProp["title"] = "ID";
    QJsonObject nameProp;
    nameProp["type"] = "string";
    nameProp["title"] = "Connection Name";
    QJsonObject itemProps;
    itemProps["id"] = idProp;
    itemProps["name"] = nameProp;

    QJsonObject describe = makeDescribeResult("connections", "array", itemProps);

    QJsonObject defaultConn;
    defaultConn["id"] = 1;
    defaultConn["name"] = "Connection 1";
    QJsonObject defaults;
    defaults["connections"] = QJsonArray{ defaultConn };
    describe["defaults"] = defaults;

    model.updateAdapterFromDescribe("testAdapter", describe);

    QJsonObject conn0;
    conn0["id"] = 1;
    conn0["name"] = "Connection 1";
    QJsonObject conn1;
    conn1["id"] = 3;
    conn1["name"] = "Connection 3";
    QJsonObject config;
    config["connections"] = QJsonArray{ conn0, conn1 };
    model.setAdapterCurrentConfig("testAdapter", config);

    AdapterSettings w(&model, "testAdapter", "connections");

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 2);

    // First add: maxId=3, so new id=4 and name="Connection 4"
    emit tabs->addTabRequested();

    QCOMPARE(tabs->count(), 3);
    auto* form0 = qobject_cast<SchemaFormWidget*>(tabs->tabContent(2));
    QVERIFY(form0 != nullptr);
    QCOMPARE(form0->values().value("id").toInt(), 4);
    QCOMPARE(form0->values().value("name").toString(), QStringLiteral("Connection 4"));
    QCOMPARE(tabs->tabText(2), QStringLiteral("Connection 4"));

    // Second add: maxId=4, so new id=5 and name="Connection 5"
    emit tabs->addTabRequested();

    QCOMPARE(tabs->count(), 4);
    auto* form1 = qobject_cast<SchemaFormWidget*>(tabs->tabContent(3));
    QVERIFY(form1 != nullptr);
    QCOMPARE(form1->values().value("id").toInt(), 5);
    QCOMPARE(form1->values().value("name").toString(), QStringLiteral("Connection 5"));
    QCOMPARE(tabs->tabText(3), QStringLiteral("Connection 5"));
}

void TestAdapterSettings::addTabInitializesNameToConnectionId()
{
    SettingsModel model;

    QJsonObject idProp;
    idProp["type"] = "integer";
    idProp["title"] = "ID";
    QJsonObject nameProp;
    nameProp["type"] = "string";
    nameProp["title"] = "Connection Name";
    QJsonObject itemProps;
    itemProps["id"] = idProp;
    itemProps["name"] = nameProp;

    QJsonObject describe = makeDescribeResult("connections", "array", itemProps);

    QJsonObject defaultConn;
    defaultConn["id"] = 1;
    defaultConn["name"] = "Connection 1";
    QJsonObject defaults;
    defaults["connections"] = QJsonArray{ defaultConn };
    describe["defaults"] = defaults;

    model.updateAdapterFromDescribe("testAdapter", describe);

    QJsonObject conn0;
    conn0["id"] = 1;
    conn0["name"] = "Connection 1";
    QJsonObject conn1;
    conn1["id"] = 2;
    conn1["name"] = "Connection 2";
    QJsonObject config;
    config["connections"] = QJsonArray{ conn0, conn1 };
    model.setAdapterCurrentConfig("testAdapter", config);

    AdapterSettings w(&model, "testAdapter", "connections");

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 2);

    emit tabs->addTabRequested();

    QCOMPARE(tabs->count(), 3);
    auto* form = qobject_cast<SchemaFormWidget*>(tabs->tabContent(2));
    QVERIFY(form != nullptr);
    QCOMPARE(form->values().value("name").toString(), QStringLiteral("Connection 3"));
}

void TestAdapterSettings::tabNameUsesNameFieldOnLoad()
{
    SettingsModel model;

    QJsonObject nameProp;
    nameProp["type"] = "string";
    nameProp["title"] = "Connection Name";
    QJsonObject itemProps;
    itemProps["name"] = nameProp;

    model.updateAdapterFromDescribe("testAdapter", makeDescribeResult("connections", "array", itemProps));

    QJsonObject conn0;
    conn0["name"] = "My Conn";
    QJsonObject config;
    config["connections"] = QJsonArray{ conn0 };
    model.setAdapterCurrentConfig("testAdapter", config);

    AdapterSettings w(&model, "testAdapter", "connections");

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->tabText(0), QStringLiteral("My Conn"));
}

void TestAdapterSettings::tabNameUpdatesWhenNameFieldChanges()
{
    SettingsModel model;

    QJsonObject nameProp;
    nameProp["type"] = "string";
    nameProp["title"] = "Connection Name";
    QJsonObject itemProps;
    itemProps["name"] = nameProp;

    model.updateAdapterFromDescribe("testAdapter", makeDescribeResult("connections", "array", itemProps));

    QJsonObject conn0;
    conn0["name"] = "Original";
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

    edit->setText("Renamed");

    QCOMPARE(tabs->tabText(0), QStringLiteral("Renamed"));
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

void TestAdapterSettings::switchingConnectionTypeAfterReloadUsesDefaults()
{
    SettingsModel model;

    QJsonObject typeProp;
    typeProp["type"] = "string";
    typeProp["enum"] = QJsonArray{ "tcp", "serial" };
    QJsonObject itemProps;
    itemProps["type"] = typeProp;

    QJsonObject portProp;
    portProp["type"] = "integer";
    QJsonObject thenProps;
    thenProps["port"] = portProp;
    QJsonObject thenObj;
    thenObj["properties"] = thenProps;

    QJsonObject portNameProp;
    portNameProp["type"] = "string";
    QJsonObject elseProps;
    elseProps["portName"] = portNameProp;
    QJsonObject elseObj;
    elseObj["properties"] = elseProps;

    QJsonObject ifTypeConst;
    ifTypeConst["const"] = "tcp";
    QJsonObject ifProps;
    ifProps["type"] = ifTypeConst;
    QJsonObject ifObj;
    ifObj["properties"] = ifProps;

    QJsonObject itemSchema;
    itemSchema["type"] = "object";
    itemSchema["properties"] = itemProps;
    itemSchema["if"] = ifObj;
    itemSchema["then"] = thenObj;
    itemSchema["else"] = elseObj;

    QJsonObject propSchema;
    propSchema["type"] = "array";
    propSchema["items"] = itemSchema;

    QJsonObject topProps;
    topProps["connections"] = propSchema;
    QJsonObject schema;
    schema["type"] = "object";
    schema["properties"] = topProps;

    QJsonObject defaultConn;
    defaultConn["type"] = "tcp";
    defaultConn["port"] = 502;
    defaultConn["portName"] = "COM1";
    QJsonObject defaults;
    defaults["connections"] = QJsonArray{ defaultConn };

    QJsonObject describe;
    describe["name"] = "testAdapter";
    describe["version"] = "1.0.0";
    describe["configVersion"] = 1;
    describe["schema"] = schema;
    describe["defaults"] = defaults;
    describe["capabilities"] = QJsonObject();

    model.updateAdapterFromDescribe("testAdapter", describe);

    // Simulate a connection that already went through one save-with-stripping:
    // "portName" (the inactive "else" branch when type is "tcp") was dropped.
    QJsonObject conn0;
    conn0["type"] = "tcp";
    conn0["port"] = 1234;
    QJsonObject config;
    config["connections"] = QJsonArray{ conn0 };
    model.setAdapterCurrentConfig("testAdapter", config);

    AdapterSettings w(&model, "testAdapter", "connections");

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 1);

    auto* form = qobject_cast<SchemaFormWidget*>(tabs->tabContent(0));
    QVERIFY(form != nullptr);

    // The stored value must survive the backfill, not just the default.
    QCOMPARE(form->values().value("port").toInt(), 1234);

    auto* combo = form->findChild<QComboBox*>();
    QVERIFY(combo != nullptr);
    combo->setCurrentIndex(combo->findData("serial"));

    auto* portNameEdit = form->findChild<QLineEdit*>();
    QVERIFY(portNameEdit != nullptr);
    QCOMPARE(portNameEdit->text(), QStringLiteral("COM1"));

    // Switching back, the stored "port" must still be the original value, not the default.
    combo->setCurrentIndex(combo->findData("tcp"));
    QCOMPARE(form->values().value("port").toInt(), 1234);
}

QTEST_MAIN(TestAdapterSettings)
