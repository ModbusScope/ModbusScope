#include "tst_adapterdevicesettings.h"

#include "customwidgets/addabletabwidget.h"
#include "customwidgets/deviceconfigtab.h"
#include "dialogs/adapterdevicesettings.h"
#include "models/settingsmodel.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QTest>

namespace {

//! Build a minimal adapter describe result with a devices schema containing
//! one integer property \c "id".
QJsonObject makeAdapterDescribe(const QString& adapterName)
{
    QJsonObject idProp;
    idProp["type"] = "integer";
    idProp["title"] = "Device ID";
    idProp["minimum"] = 1;

    QJsonObject itemProps;
    itemProps["id"] = idProp;

    QJsonObject itemSchema;
    itemSchema["type"] = "object";
    itemSchema["properties"] = itemProps;

    QJsonObject devicesSchema;
    devicesSchema["type"] = "array";
    devicesSchema["items"] = itemSchema;

    QJsonObject connectionsSchema;
    connectionsSchema["type"] = "array";
    connectionsSchema["items"] = QJsonObject();

    QJsonObject generalSchema;
    generalSchema["type"] = "object";
    generalSchema["properties"] = QJsonObject();

    QJsonObject topProps;
    topProps["general"] = generalSchema;
    topProps["connections"] = connectionsSchema;
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

void TestAdapterDeviceSettings::setupAdapter(SettingsModel& model, const QString& adapterId, const QJsonArray& devices)
{
    model.updateAdapterFromDescribe(adapterId, makeAdapterDescribe(adapterId));

    QJsonObject config;
    config["general"] = QJsonObject();
    config["connections"] = QJsonArray();
    config["devices"] = devices;
    model.setAdapterCurrentConfig(adapterId, config);
}

void TestAdapterDeviceSettings::noAdapterShowsLabel()
{
    SettingsModel model;
    AdapterDeviceSettings w(&model);

    QVERIFY(w.findChild<QLabel*>() != nullptr);
    QVERIFY(w.findChild<AddableTabWidget*>() == nullptr);
}

void TestAdapterDeviceSettings::devicesPopulateTabsFromConfig()
{
    SettingsModel model;

    QJsonObject dev0;
    dev0["id"] = 1;
    QJsonObject dev1;
    dev1["id"] = 2;

    setupAdapter(model, "adapterA", QJsonArray{ dev0, dev1 });

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 2);
}

void TestAdapterDeviceSettings::deviceModelNameUsedAsTabTitle()
{
    SettingsModel model;

    QJsonObject dev;
    dev["id"] = 1;

    setupAdapter(model, "adapterA", QJsonArray{ dev });

    model.addDevice(1);
    model.deviceSettings(1)->setName("Pump");

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->tabText(0), QStringLiteral("Pump"));
}

void TestAdapterDeviceSettings::missingNameFallsBackToDeviceN()
{
    SettingsModel model;

    // Device with no "id" field — id defaults to -1, no Device lookup possible
    setupAdapter(model, "adapterA", QJsonArray{ QJsonObject() });

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    if (tabs == nullptr)
    {
        return;
    }
    QVERIFY(tabs->tabText(0).startsWith("Device "));
}

void TestAdapterDeviceSettings::acceptValuesSavesToAdapterConfig()
{
    SettingsModel model;

    QJsonObject dev;
    dev["id"] = 1;

    setupAdapter(model, "adapterA", QJsonArray{ dev });

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);

    auto* tab = qobject_cast<DeviceConfigTab*>(tabs->tabContent(0));
    QVERIFY(tab != nullptr);

    auto* spin = tab->findChild<QSpinBox*>();
    QVERIFY(spin != nullptr);
    spin->setValue(2);

    w.acceptValues();

    const AdapterData* adapter = model.adapterData("adapterA");
    QCOMPARE(adapter->hasStoredConfig(), true);
    QCOMPARE(adapter->currentConfig().value("devices").toArray().at(0).toObject().value("id").toInt(), 2);
}

void TestAdapterDeviceSettings::acceptValuesSavesDeviceNameToModel()
{
    SettingsModel model;

    QJsonObject dev;
    dev["id"] = 1;
    setupAdapter(model, "adapterA", QJsonArray{ dev });

    model.addDevice(1);
    model.deviceSettings(1)->setName("Old Name");

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);

    auto* tab = qobject_cast<DeviceConfigTab*>(tabs->tabContent(0));
    QVERIFY(tab != nullptr);

    auto* nameEdit = tab->findChild<QLineEdit*>(QString(), Qt::FindDirectChildrenOnly);
    QVERIFY(nameEdit != nullptr);
    nameEdit->setText("New Name");

    w.acceptValues();

    QCOMPARE(model.deviceSettings(1)->name(), QStringLiteral("New Name"));
}

void TestAdapterDeviceSettings::addTabUsesDeviceDefaults()
{
    SettingsModel model;

    QJsonObject describe = makeAdapterDescribe("adapterA");
    QJsonObject defaultDevice;
    defaultDevice["id"] = 5;
    QJsonObject defaults;
    defaults["devices"] = QJsonArray{ defaultDevice };
    defaults["connections"] = QJsonArray();
    defaults["general"] = QJsonObject();
    describe["defaults"] = defaults;

    model.updateAdapterFromDescribe("adapterA", describe);

    QJsonObject config;
    config["general"] = QJsonObject();
    config["connections"] = QJsonArray();
    config["devices"] = QJsonArray();
    model.setAdapterCurrentConfig("adapterA", config);

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 0);

    emit tabs->addTabRequested();

    QCOMPARE(tabs->count(), 1);
    auto* tab = qobject_cast<DeviceConfigTab*>(tabs->tabContent(0));
    QVERIFY(tab != nullptr);

    auto* spin = tab->findChild<QSpinBox*>();
    QVERIFY(spin != nullptr);
    QCOMPARE(spin->value(), 5);
}

QTEST_MAIN(TestAdapterDeviceSettings)
