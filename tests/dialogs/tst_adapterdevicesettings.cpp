#include "tst_adapterdevicesettings.h"

#include "customwidgets/addabletabwidget.h"
#include "customwidgets/deviceconfigtab.h"
#include "dialogs/adapterdevicesettings.h"
#include "models/settingsmodel.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QTest>

namespace {

//! Build a minimal adapter describe result with a devices schema containing
//! one string property \c "name".
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

void TestAdapterDeviceSettings::setupAdapter(SettingsModel& model,
                                             const QString& adapterId,
                                             const QJsonArray& devices)
{
    model.adapterData(adapterId)->updateFromDescribe(makeAdapterDescribe(adapterId));

    QJsonObject config;
    config["general"] = QJsonObject();
    config["connections"] = QJsonArray();
    config["devices"] = devices;
    model.adapterData(adapterId)->setCurrentConfig(config);
    model.adapterData(adapterId)->setHasStoredConfig(true);
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
    dev0["name"] = "Pump";
    QJsonObject dev1;
    dev1["name"] = "Motor";

    setupAdapter(model, "adapterA", QJsonArray{ dev0, dev1 });

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 2);
}

void TestAdapterDeviceSettings::deviceNameUsedAsTabTitle()
{
    SettingsModel model;

    QJsonObject dev;
    dev["name"] = "Pump";

    setupAdapter(model, "adapterA", QJsonArray{ dev });

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->tabText(0), QStringLiteral("Pump"));
}

void TestAdapterDeviceSettings::missingNameFallsBackToDeviceN()
{
    SettingsModel model;

    // Device with no "name" field
    setupAdapter(model, "adapterA", QJsonArray{ QJsonObject() });

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QVERIFY(tabs->tabText(0).startsWith("Device "));
}

void TestAdapterDeviceSettings::acceptValuesSavesToAdapterConfig()
{
    SettingsModel model;

    QJsonObject dev;
    dev["name"] = "OriginalName";

    setupAdapter(model, "adapterA", QJsonArray{ dev });

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);

    auto* tab = qobject_cast<DeviceConfigTab*>(tabs->tabContent(0));
    QVERIFY(tab != nullptr);

    auto* edit = tab->findChild<QLineEdit*>();
    QVERIFY(edit != nullptr);
    edit->setText("UpdatedName");

    w.acceptValues();

    AdapterData* adapter = model.adapterData("adapterA");
    QCOMPARE(adapter->hasStoredConfig(), true);
    QCOMPARE(adapter->currentConfig()["devices"].toArray().at(0).toObject()["name"].toString(),
             QStringLiteral("UpdatedName"));
}

QTEST_MAIN(TestAdapterDeviceSettings)
