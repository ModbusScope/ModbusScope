#include "tst_adapterdevicesettings.h"

#include "customwidgets/addabletabwidget.h"
#include "customwidgets/deviceconfigtab.h"
#include "dialogs/adapterdevicesettings.h"
#include "models/settingsmodel.h"

#include <QComboBox>
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
    QVERIFY(tabs->tabText(0).startsWith("Device"));
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
    // The adapter default id (5) must be overridden with a unique SettingsModel id.
    const int assignedId = spin->value();
    QVERIFY(assignedId != 5);
    QVERIFY(model.hasDevice(static_cast<deviceId_t>(assignedId)));
    QCOMPARE(model.deviceSettings(static_cast<deviceId_t>(assignedId))->adapterId(), QStringLiteral("adapterA"));
}

void TestAdapterDeviceSettings::addTabIncrementsDeviceId()
{
    SettingsModel model;
    setupAdapter(model, "adapterA", QJsonArray());

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 0);

    emit tabs->addTabRequested();
    emit tabs->addTabRequested();

    QCOMPARE(tabs->count(), 2);

    auto* tab0 = qobject_cast<DeviceConfigTab*>(tabs->tabContent(0));
    auto* tab1 = qobject_cast<DeviceConfigTab*>(tabs->tabContent(1));
    QVERIFY(tab0 != nullptr);
    QVERIFY(tab1 != nullptr);

    const int id0 = tab0->values().value("id").toInt(-1);
    const int id1 = tab1->values().value("id").toInt(-1);

    QVERIFY(id0 >= 1);
    QVERIFY(id1 > id0);

    // Both ids must be present in the SettingsModel and linked to the adapter.
    QVERIFY(model.hasDevice(static_cast<deviceId_t>(id0)));
    QVERIFY(model.hasDevice(static_cast<deviceId_t>(id1)));
    QCOMPARE(model.deviceSettings(static_cast<deviceId_t>(id0))->adapterId(), QStringLiteral("adapterA"));
    QCOMPARE(model.deviceSettings(static_cast<deviceId_t>(id1))->adapterId(), QStringLiteral("adapterA"));
}

void TestAdapterDeviceSettings::deviceIdPreservedWhenAdapterChanged()
{
    SettingsModel model;
    setupAdapter(model, "adapterA", QJsonArray());
    setupAdapter(model, "adapterB", QJsonArray());

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);

    emit tabs->addTabRequested();

    QCOMPARE(tabs->count(), 1);
    auto* tab = qobject_cast<DeviceConfigTab*>(tabs->tabContent(0));
    QVERIFY(tab != nullptr);

    const int originalId = tab->values().value("id").toInt(-1);
    QVERIFY(originalId >= 1);

    // Trigger DeviceConfigTab::onAdapterChanged by switching the adapter combo
    auto* adapterCombo = tab->findChild<QComboBox*>();
    QVERIFY(adapterCombo != nullptr);
    int adapterBIdx = adapterCombo->findData(QStringLiteral("adapterB"));
    QVERIFY(adapterBIdx >= 0);
    adapterCombo->setCurrentIndex(adapterBIdx);

    // The device ID must be preserved across the adapter switch
    QCOMPARE(tab->values().value("id").toInt(-1), originalId);
    // The device must still exist in the model
    QVERIFY(model.hasDevice(static_cast<deviceId_t>(originalId)));
    // The tab must now report the new adapter
    QCOMPARE(tab->adapterId(), QStringLiteral("adapterB"));
}

void TestAdapterDeviceSettings::deviceNamePersistedAfterAcceptAndReopen()
{
    SettingsModel model;

    QJsonObject dev1;
    dev1["id"] = 1;
    QJsonObject dev2;
    dev2["id"] = 2;
    setupAdapter(model, "adapterA", QJsonArray{ dev1, dev2 });

    // Device 2 is in the adapter config but NOT in the SettingsModel device list,
    // simulating the case where the project file's devices section omitted it.
    QVERIFY(!model.hasDevice(2));

    // First dialog open: user types a name for device 2.
    {
        AdapterDeviceSettings w(&model);

        auto* tabs = w.findChild<AddableTabWidget*>();
        QVERIFY(tabs != nullptr);

        auto* tab2 = qobject_cast<DeviceConfigTab*>(tabs->tabContent(1));
        QVERIFY(tab2 != nullptr);

        auto* nameEdit = tab2->findChild<QLineEdit*>(QString(), Qt::FindDirectChildrenOnly);
        QVERIFY(nameEdit != nullptr);
        nameEdit->setText("Pump 2");

        w.acceptValues();
    }

    // After accepting, device 2 must be registered in the model with the correct name.
    QVERIFY(model.hasDevice(2));
    QCOMPARE(model.deviceSettings(2)->name(), QStringLiteral("Pump 2"));

    // Second dialog open (reopen): device 2 name must not be reset to empty.
    {
        AdapterDeviceSettings w2(&model);

        auto* tabs = w2.findChild<AddableTabWidget*>();
        QVERIFY(tabs != nullptr);

        auto* tab2 = qobject_cast<DeviceConfigTab*>(tabs->tabContent(1));
        QVERIFY(tab2 != nullptr);

        auto* nameEdit = tab2->findChild<QLineEdit*>(QString(), Qt::FindDirectChildrenOnly);
        QVERIFY(nameEdit != nullptr);
        QCOMPARE(nameEdit->text(), QStringLiteral("Pump 2"));
    }
}

void TestAdapterDeviceSettings::addTabDoesNotReuseIdFromAdapterConfig()
{
    SettingsModel model;

    // Use id=2: SettingsModel pre-populates device 1 (cFirstDeviceId), so the
    // first free slot found by addNewDevice() would be 2 — colliding with this tab
    // unless the constructor registers it first.
    QJsonObject dev;
    dev["id"] = 2;
    setupAdapter(model, "adapterA", QJsonArray{ dev });

    // Device 2 is in the adapter config but NOT registered in SettingsModel.
    QVERIFY(!model.hasDevice(2));

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 1);

    emit tabs->addTabRequested();

    QCOMPARE(tabs->count(), 2);
    auto* tab = qobject_cast<DeviceConfigTab*>(tabs->tabContent(1));
    QVERIFY(tab != nullptr);

    const int assignedId = tab->values().value("id").toInt(-1);
    QVERIFY(assignedId != 2);
    QCOMPARE(assignedId, 3);
    QVERIFY(model.hasDevice(static_cast<deviceId_t>(assignedId)));
}

void TestAdapterDeviceSettings::addTabWithGapAssignsNextAfterMax()
{
    SettingsModel model;

    // Devices 1 and 3 are present — id 2 is a gap. Adding a new device should
    // assign id 4 (max + 1), not 2 (gap fill), to avoid confusing tab ordering
    // where a new "Device 2" tab appears after an existing "Device 3" tab.
    QJsonObject dev1;
    dev1["id"] = 1;
    QJsonObject dev3;
    dev3["id"] = 3;
    setupAdapter(model, "adapterA", QJsonArray{ dev1, dev3 });

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 2);

    emit tabs->addTabRequested();

    QCOMPARE(tabs->count(), 3);
    auto* tab = qobject_cast<DeviceConfigTab*>(tabs->tabContent(2));
    QVERIFY(tab != nullptr);

    const int assignedId = tab->values().value("id").toInt(-1);
    QCOMPARE(assignedId, 4);
    QVERIFY(model.hasDevice(static_cast<deviceId_t>(assignedId)));
}

void TestAdapterDeviceSettings::closeTabRemovesDeviceFromModel()
{
    SettingsModel model;

    QJsonObject dev1;
    dev1["id"] = 1;
    QJsonObject dev2;
    dev2["id"] = 2;
    setupAdapter(model, "adapterA", QJsonArray{ dev1, dev2 });

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QVERIFY(model.hasDevice(1));
    QVERIFY(model.hasDevice(2));

    tabs->handleCloseTab(0);

    QVERIFY(!model.hasDevice(1));
    QVERIFY(model.hasDevice(2));
}

void TestAdapterDeviceSettings::nameChangeUpdatesModelImmediately()
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

    auto* nameEdit = tab->findChild<QLineEdit*>(QString(), Qt::FindDirectChildrenOnly);
    QVERIFY(nameEdit != nullptr);
    nameEdit->setText("Live Name");

    QCOMPARE(model.deviceSettings(1)->name(), QStringLiteral("Live Name"));
}

void TestAdapterDeviceSettings::adapterChangeUpdatesModelImmediately()
{
    SettingsModel model;
    setupAdapter(model, "adapterA", QJsonArray());
    setupAdapter(model, "adapterB", QJsonArray());

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);

    emit tabs->addTabRequested();
    auto* tab = qobject_cast<DeviceConfigTab*>(tabs->tabContent(0));
    QVERIFY(tab != nullptr);

    const int devId = tab->values().value("id").toInt(-1);
    QVERIFY(devId >= 1);
    QCOMPARE(model.deviceSettings(static_cast<deviceId_t>(devId))->adapterId(), QStringLiteral("adapterA"));

    auto* adapterCombo = tab->findChild<QComboBox*>();
    QVERIFY(adapterCombo != nullptr);
    int adapterBIdx = adapterCombo->findData(QStringLiteral("adapterB"));
    QVERIFY(adapterBIdx >= 0);
    adapterCombo->setCurrentIndex(adapterBIdx);

    QCOMPARE(model.deviceSettings(static_cast<deviceId_t>(devId))->adapterId(), QStringLiteral("adapterB"));
}

void TestAdapterDeviceSettings::multipleAdaptersWithDevices()
{
    SettingsModel model;

    QJsonObject devA;
    devA["id"] = 1;
    QJsonObject devB;
    devB["id"] = 2;
    setupAdapter(model, "adapterA", QJsonArray{ devA });
    setupAdapter(model, "adapterB", QJsonArray{ devB });

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 2);

    w.acceptValues();

    const AdapterData* adapterA = model.adapterData("adapterA");
    const AdapterData* adapterB = model.adapterData("adapterB");
    QCOMPARE(adapterA->currentConfig().value("devices").toArray().size(), 1);
    QCOMPARE(adapterB->currentConfig().value("devices").toArray().size(), 1);
    QCOMPARE(adapterA->currentConfig().value("devices").toArray().at(0).toObject().value("id").toInt(), 1);
    QCOMPARE(adapterB->currentConfig().value("devices").toArray().at(0).toObject().value("id").toInt(), 2);
}

void TestAdapterDeviceSettings::addTabAfterIdEditDoesNotDuplicate()
{
    SettingsModel model;

    QJsonObject dev1;
    dev1["id"] = 1;
    setupAdapter(model, "adapterA", QJsonArray{ dev1 });

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 1);

    // Add device → gets ID 2
    emit tabs->addTabRequested();
    QCOMPARE(tabs->count(), 2);

    auto* tab2 = qobject_cast<DeviceConfigTab*>(tabs->tabContent(1));
    QVERIFY(tab2 != nullptr);
    QCOMPARE(tab2->values().value("id").toInt(-1), 2);

    // User manually edits device 2's ID spinbox to 3
    auto* spin = tab2->findChild<QSpinBox*>();
    QVERIFY(spin != nullptr);
    spin->setValue(3);
    QCOMPARE(tab2->values().value("id").toInt(-1), 3);

    // Add another device — must get ID 4, not 3 (which is now shown in an open tab)
    emit tabs->addTabRequested();
    QCOMPARE(tabs->count(), 3);

    auto* tab3 = qobject_cast<DeviceConfigTab*>(tabs->tabContent(2));
    QVERIFY(tab3 != nullptr);
    const int assignedId = tab3->values().value("id").toInt(-1);
    QCOMPARE(assignedId, 4);
}

void TestAdapterDeviceSettings::acceptValuesClearsDevicesForEmptiedAdapter()
{
    SettingsModel model;

    QJsonObject devA;
    devA["id"] = 1;
    QJsonObject devB;
    devB["id"] = 3;
    setupAdapter(model, "adapterA", QJsonArray{ devA });
    setupAdapter(model, "adapterB", QJsonArray{ devB });

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 2);

    // Close adapter B's only device tab (tab index 1 — adapters sorted alphabetically)
    tabs->handleCloseTab(1);
    QCOMPARE(tabs->count(), 1);

    w.acceptValues();

    // Adapter B's config must have an empty devices array, not the stale {device 3}
    const AdapterData* adapterB = model.adapterData("adapterB");
    QVERIFY(adapterB->hasStoredConfig());
    QCOMPARE(adapterB->currentConfig().value("devices").toArray().size(), 0);

    // Re-open: device 3 must NOT reappear
    AdapterDeviceSettings w2(&model);
    auto* tabs2 = w2.findChild<AddableTabWidget*>();
    QVERIFY(tabs2 != nullptr);
    QCOMPARE(tabs2->count(), 1);
}

void TestAdapterDeviceSettings::cancelAndReopenDoesNotLeakDeviceIds()
{
    SettingsModel model;

    QJsonObject dev1;
    dev1["id"] = 1;
    setupAdapter(model, "adapterA", QJsonArray{ dev1 });

    // First session: add a device then destroy without accepting (simulate cancel)
    {
        AdapterDeviceSettings w(&model);

        auto* tabs = w.findChild<AddableTabWidget*>();
        QVERIFY(tabs != nullptr);
        QCOMPARE(tabs->count(), 1);

        emit tabs->addTabRequested(); // addNewDevice() → ID 2; leaks into model on cancel
        QCOMPARE(tabs->count(), 2);
        // w destroyed without acceptValues() — leaked device 2 remains in model
    }

    // Second session: config still has only device 1
    {
        AdapterDeviceSettings w2(&model);

        auto* tabs = w2.findChild<AddableTabWidget*>();
        QVERIFY(tabs != nullptr);
        QCOMPARE(tabs->count(), 1);

        emit tabs->addTabRequested();
        QCOMPARE(tabs->count(), 2);

        auto* newTab = qobject_cast<DeviceConfigTab*>(tabs->tabContent(1));
        QVERIFY(newTab != nullptr);
        const int assignedId = newTab->values().value("id").toInt(-1);
        QCOMPARE(assignedId, 2); // must be 2, not 3 or higher
    }
}

#if 0
void TestAdapterDeviceSettings::editingDeviceIdInSchemaFormUpdatesCorrectModelDevice()
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

    // Change the id spinbox from 1 to 5
    auto* spin = tab->findChild<QSpinBox*>();
    QVERIFY(spin != nullptr);
    spin->setValue(5);

    // After spinbox change, deviceId() must return the updated value
    QCOMPARE(tab->deviceId(), 5);

    QVERIFY(!model.hasDevice(5));
    auto* nameEdit = tab->findChild<QLineEdit*>(QString(), Qt::FindDirectChildrenOnly);
    QVERIFY(nameEdit != nullptr);
    nameEdit->setText("Sensor");
    QVERIFY(model.hasDevice(5));
    QCOMPARE(model.deviceSettings(5)->name(), QStringLiteral("Sensor"));
}

#endif

QTEST_MAIN(TestAdapterDeviceSettings)
