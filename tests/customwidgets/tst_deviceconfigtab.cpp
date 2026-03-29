#include "tst_deviceconfigtab.h"

#include "customwidgets/deviceconfigtab.h"
#include "models/settingsmodel.h"

#include <QComboBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QTest>

namespace {

//! Build a minimal adapter describe result with a device schema containing one
//! string property \c "name".
QJsonObject makeAdapterDescribe(const QString& adapterName)
{
    QJsonObject nameProp;
    nameProp["type"] = "string";
    nameProp["title"] = "Name";

    QJsonObject itemProps;
    itemProps["name"] = nameProp;

    QJsonObject itemSchema;
    itemSchema["type"] = "object";
    itemSchema["properties"] = itemProps;

    QJsonObject devicesSchema;
    devicesSchema["type"] = "array";
    devicesSchema["items"] = itemSchema;

    QJsonObject topProps;
    topProps["devices"] = devicesSchema;

    QJsonObject schema;
    schema["type"] = "object";
    schema["properties"] = topProps;

    QJsonObject describe;
    describe["name"] = adapterName;
    describe["version"] = "1.0.0";
    describe["configVersion"] = 1;
    describe["schema"] = schema;
    describe["defaults"] = QJsonObject();
    describe["capabilities"] = QJsonObject();
    return describe;
}

} // namespace

void TestDeviceConfigTab::setupTwoAdapters(SettingsModel& model)
{
    model.updateAdapterFromDescribe("adapterA", makeAdapterDescribe("adapterA"));
    model.updateAdapterFromDescribe("adapterB", makeAdapterDescribe("adapterB"));
}

void TestDeviceConfigTab::comboContainsAllAdapterIds()
{
    SettingsModel model;
    setupTwoAdapters(model);

    DeviceConfigTab tab(&model, "adapterA", QJsonObject());

    auto* combo = tab.findChild<QComboBox*>(QString(), Qt::FindDirectChildrenOnly);
    QVERIFY(combo != nullptr);
    QCOMPARE(combo->count(), 2);

    QStringList items;
    for (int i = 0; i < combo->count(); ++i)
    {
        items.append(combo->itemData(i).toString());
    }
    QVERIFY(items.contains("adapterA"));
    QVERIFY(items.contains("adapterB"));
}

void TestDeviceConfigTab::constructorSelectsCorrectAdapter()
{
    SettingsModel model;
    setupTwoAdapters(model);

    DeviceConfigTab tab(&model, "adapterB", QJsonObject());

    QCOMPARE(tab.adapterId(), QStringLiteral("adapterB"));

    auto* combo = tab.findChild<QComboBox*>(QString(), Qt::FindDirectChildrenOnly);
    QVERIFY(combo != nullptr);
    QCOMPARE(combo->currentData().toString(), QStringLiteral("adapterB"));
}

void TestDeviceConfigTab::valuesReturnsDeviceFieldValues()
{
    SettingsModel model;
    setupTwoAdapters(model);

    QJsonObject deviceValues;
    deviceValues["name"] = "Pump";

    DeviceConfigTab tab(&model, "adapterA", deviceValues);

    QCOMPARE(tab.values().value("name").toString(), QStringLiteral("Pump"));
}

void TestDeviceConfigTab::adapterIdMatchesComboInitially()
{
    SettingsModel model;
    setupTwoAdapters(model);

    DeviceConfigTab tab(&model, "adapterA", QJsonObject());

    auto* combo = tab.findChild<QComboBox*>(QString(), Qt::FindDirectChildrenOnly);
    QVERIFY(combo != nullptr);
    QCOMPARE(tab.adapterId(), combo->currentData().toString());
}

void TestDeviceConfigTab::deviceNameInitializesFromDeviceModel()
{
    SettingsModel model;
    setupTwoAdapters(model);
    model.addDevice(5);
    model.deviceSettings(5)->setName("Pump Station");

    QJsonObject deviceValues;
    deviceValues["id"] = 5;

    DeviceConfigTab tab(&model, "adapterA", deviceValues);
    QCOMPARE(tab.deviceName(), QStringLiteral("Pump Station"));
}

void TestDeviceConfigTab::deviceNameEmptyForUnregisteredDevice()
{
    SettingsModel model;
    setupTwoAdapters(model);

    QJsonObject deviceValues;
    deviceValues["id"] = 99;

    DeviceConfigTab tab(&model, "adapterA", deviceValues);
    QVERIFY(tab.deviceName().isEmpty());
}

void TestDeviceConfigTab::adapterChangeUsesDefaults()
{
    SettingsModel model;
    model.updateAdapterFromDescribe("adapterA", makeAdapterDescribe("adapterA"));

    QJsonObject describeB = makeAdapterDescribe("adapterB");
    QJsonObject defaultDevice;
    defaultDevice["name"] = "defaultName";
    QJsonObject defaults;
    defaults["devices"] = QJsonArray{ defaultDevice };
    describeB["defaults"] = defaults;
    model.updateAdapterFromDescribe("adapterB", describeB);

    DeviceConfigTab tab(&model, "adapterA", QJsonObject());

    auto* combo = tab.findChild<QComboBox*>(QString(), Qt::FindDirectChildrenOnly);
    QVERIFY(combo != nullptr);

    combo->setCurrentIndex(combo->findData(QStringLiteral("adapterB")));

    QCOMPARE(tab.values().value("name").toString(), QStringLiteral("defaultName"));
}

QTEST_MAIN(TestDeviceConfigTab)
