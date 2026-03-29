#include "tst_adapterconnectionsettings.h"

#include "customwidgets/addabletabwidget.h"
#include "customwidgets/schemaformwidget.h"
#include "dialogs/adapterconnectionsettings.h"
#include "models/settingsmodel.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QTest>

namespace {

//! Build a describe result whose connections section has the given \a connectionsType
//! ("array" or "object") and whose general section has \a generalProperties.
QJsonObject makeDescribeResult(const QString& connectionsType,
                               const QJsonObject& generalProperties = QJsonObject(),
                               const QJsonObject& connectionItemProps = QJsonObject())
{
    QJsonObject generalSchema;
    generalSchema["type"] = "object";
    generalSchema["properties"] = generalProperties;

    QJsonObject connectionsSchema;
    if (connectionsType == "array")
    {
        QJsonObject itemSchema;
        itemSchema["type"] = "object";
        itemSchema["properties"] = connectionItemProps;

        connectionsSchema["type"] = "array";
        connectionsSchema["items"] = itemSchema;
    }
    else
    {
        connectionsSchema["type"] = "object";
        connectionsSchema["properties"] = connectionItemProps;
    }

    QJsonObject devicesSchema;
    devicesSchema["type"] = "array";
    devicesSchema["items"] = QJsonObject();

    QJsonObject topProps;
    topProps["general"] = generalSchema;
    topProps["connections"] = connectionsSchema;
    topProps["devices"] = devicesSchema;

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

void TestAdapterConnectionSettings::noAdapterShowsLabel()
{
    SettingsModel model;
    AdapterConnectionSettings w(&model);

    QVERIFY(w.findChild<QLabel*>() != nullptr);
    QVERIFY(w.findChild<AddableTabWidget*>() == nullptr);
}

void TestAdapterConnectionSettings::emptySchemaShowsLabel()
{
    SettingsModel model;
    // Creates entry with empty schema
    model.adapterData("testAdapter");

    AdapterConnectionSettings w(&model);

    QVERIFY(w.findChild<QLabel*>() != nullptr);
    QVERIFY(w.findChild<AddableTabWidget*>() == nullptr);
}

void TestAdapterConnectionSettings::arrayConnectionsCreatesTabWidget()
{
    SettingsModel model;

    QJsonObject hostProp;
    hostProp["type"] = "string";
    QJsonObject itemProps;
    itemProps["host"] = hostProp;

    model.updateAdapterFromDescribe("testAdapter", makeDescribeResult("array", QJsonObject(), itemProps));

    // Store a config with 2 connections
    QJsonObject conn0;
    conn0["host"] = "192.168.0.1";
    QJsonObject conn1;
    conn1["host"] = "10.0.0.1";
    QJsonObject config;
    config["connections"] = QJsonArray{ conn0, conn1 };
    config["devices"] = QJsonArray();
    config["general"] = QJsonObject();
    model.setAdapterCurrentConfig("testAdapter", config);

    AdapterConnectionSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 2);
}

void TestAdapterConnectionSettings::objectConnectionsCreatesSingleForm()
{
    SettingsModel model;
    model.updateAdapterFromDescribe("testAdapter", makeDescribeResult("object"));

    AdapterConnectionSettings w(&model);

    QVERIFY(w.findChild<AddableTabWidget*>() == nullptr);
    QVERIFY(w.findChild<SchemaFormWidget*>() != nullptr);
}

void TestAdapterConnectionSettings::generalSectionHiddenWhenPropertiesEmpty()
{
    SettingsModel model;
    // general.properties is empty by default in makeDescribeResult
    model.updateAdapterFromDescribe("testAdapter", makeDescribeResult("array"));

    AdapterConnectionSettings w(&model);

    // With no general section, only the connections SchemaFormWidgets inside the
    // tab widget should exist. The tab widget itself has no SchemaFormWidget sibling.
    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);

    // SchemaFormWidgets should only be inside the tab widget, not siblings of it
    auto forms = w.findChildren<SchemaFormWidget*>(QString(), Qt::FindDirectChildrenOnly);
    QCOMPARE(forms.size(), 0);
}

void TestAdapterConnectionSettings::generalSectionShownWhenNonEmpty()
{
    SettingsModel model;

    QJsonObject settingProp;
    settingProp["type"] = "string";
    QJsonObject generalProperties;
    generalProperties["setting"] = settingProp;

    model.updateAdapterFromDescribe("testAdapter", makeDescribeResult("array", generalProperties));

    AdapterConnectionSettings w(&model);

    // The general SchemaFormWidget is a direct child of the connection settings widget
    auto forms = w.findChildren<SchemaFormWidget*>(QString(), Qt::FindDirectChildrenOnly);
    QCOMPARE(forms.size(), 1);
}

void TestAdapterConnectionSettings::acceptValuesStoresConfigInAdapterData()
{
    SettingsModel model;

    QJsonObject hostProp;
    hostProp["type"] = "string";
    QJsonObject itemProps;
    itemProps["host"] = hostProp;

    model.updateAdapterFromDescribe("testAdapter", makeDescribeResult("array", QJsonObject(), itemProps));

    QJsonObject conn0;
    conn0["host"] = "original";
    QJsonObject config;
    config["connections"] = QJsonArray{ conn0 };
    config["devices"] = QJsonArray();
    config["general"] = QJsonObject();
    model.setAdapterCurrentConfig("testAdapter", config);

    AdapterConnectionSettings w(&model);

    // Modify the host field in the first connection tab
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

void TestAdapterConnectionSettings::addTabUsesConnectionDefaults()
{
    SettingsModel model;

    QJsonObject portProp;
    portProp["type"] = "integer";
    portProp["title"] = "Port";
    QJsonObject itemProps;
    itemProps["port"] = portProp;

    QJsonObject describe = makeDescribeResult("array", QJsonObject(), itemProps);

    QJsonObject defaultConn;
    defaultConn["port"] = 502;
    QJsonObject defaults;
    defaults["connections"] = QJsonArray{ defaultConn };
    defaults["devices"] = QJsonArray();
    defaults["general"] = QJsonObject();
    describe["defaults"] = defaults;

    model.updateAdapterFromDescribe("testAdapter", describe);

    QJsonObject config;
    config["connections"] = QJsonArray();
    config["devices"] = QJsonArray();
    config["general"] = QJsonObject();
    model.setAdapterCurrentConfig("testAdapter", config);

    AdapterConnectionSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 0);

    emit tabs->addTabRequested();

    QCOMPARE(tabs->count(), 1);
    auto* form = qobject_cast<SchemaFormWidget*>(tabs->tabContent(0));
    QVERIFY(form != nullptr);
    QCOMPARE(form->values().value("port").toInt(), 502);
}

QTEST_MAIN(TestAdapterConnectionSettings)
