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
#include <QSignalSpy>
#include <QSpinBox>
#include <QTest>
#include <QToolButton>

#include <climits>

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

//! Build a describe result like makeAdapterDescribe(), but with a "defaults" section
//! declaring a single untouched-default device with the given ID.
QJsonObject makeAdapterDescribeWithDefaultDevice(const QString& adapterName, int deviceId)
{
    QJsonObject describe = makeAdapterDescribe(adapterName);

    QJsonObject defaultDevice;
    defaultDevice["id"] = deviceId;
    QJsonObject defaults;
    defaults["devices"] = QJsonArray{ defaultDevice };
    defaults["connections"] = QJsonArray();
    defaults["general"] = QJsonObject();
    describe["defaults"] = defaults;

    return describe;
}

//! Build a describe result like makeAdapterDescribe(), but with the devices schema's
//! maxItems set to the given limit.
QJsonObject makeAdapterDescribeWithMaxItems(const QString& adapterName, int maxItems)
{
    QJsonObject describe = makeAdapterDescribe(adapterName);

    QJsonObject schema = describe["schema"].toObject();
    QJsonObject topProps = schema["properties"].toObject();
    QJsonObject devicesSchema = topProps["devices"].toObject();
    devicesSchema["maxItems"] = maxItems;
    topProps["devices"] = devicesSchema;
    schema["properties"] = topProps;
    describe["schema"] = schema;

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

    w.acceptValues();

    const AdapterData* adapter = model.adapterData("adapterA");
    QCOMPARE(adapter->hasStoredConfig(), true);
    QCOMPARE(adapter->currentConfig().value("devices").toArray().at(0).toObject().value("id").toInt(), 1);
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

void TestAdapterDeviceSettings::twoAdaptersWithSameDefaultDeviceIdShowsSingleTab()
{
    SettingsModel model;

    model.updateAdapterFromDescribe("adapterA", makeAdapterDescribeWithDefaultDevice("adapterA", 1));
    model.updateAdapterFromDescribe("adapterB", makeAdapterDescribeWithDefaultDevice("adapterB", 1));

    QVERIFY(!model.adapterData("adapterA")->hasStoredConfig());
    QVERIFY(!model.adapterData("adapterB")->hasStoredConfig());

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 1);

    auto* tab = qobject_cast<DeviceConfigTab*>(tabs->tabContent(0));
    QVERIFY(tab != nullptr);
    QCOMPARE(tab->values().value("id").toInt(-1), 1);
    QCOMPARE(tab->adapterId(), QStringLiteral("adapterA"));

    QCOMPARE(model.deviceList().size(), 1);
    QVERIFY(model.hasDevice(1));
}

void TestAdapterDeviceSettings::existingDeviceAdapterIdMatchesConfigOnOpen()
{
    SettingsModel model;

    // Use id=2: SettingsModel pre-populates device 1 (cFirstDeviceId), so use a
    // different id to genuinely exercise the "not yet in model" path.
    QJsonObject dev;
    dev["id"] = 2;
    setupAdapter(model, "adapterB", QJsonArray{ dev });

    // Device is not yet registered in the model — AdapterDeviceSettings must add it
    // and assign it the adapter its config came from, not Device's constructor default.
    QVERIFY(!model.hasDevice(2));

    AdapterDeviceSettings w(&model);

    QVERIFY(model.hasDevice(2));
    QCOMPARE(model.deviceSettings(2)->adapterId(), QStringLiteral("adapterB"));
}

void TestAdapterDeviceSettings::sharedDefaultDeviceIdReconciledOnDescribeWithoutOpeningDialog()
{
    SettingsModel model;

    // No AdapterDeviceSettings is constructed here: reconciliation must happen as soon as
    // adapters describe, not only when the user opens Settings.
    model.updateAdapterFromDescribe("adapterA", makeAdapterDescribeWithDefaultDevice("adapterA", 1));
    model.updateAdapterFromDescribe("adapterB", makeAdapterDescribeWithDefaultDevice("adapterB", 1));

    QCOMPARE(model.deviceList().size(), 1);
    QVERIFY(model.hasDevice(1));
    QCOMPARE(model.deviceSettings(1)->adapterId(), QStringLiteral("adapterA"));
}

void TestAdapterDeviceSettings::explicitDeviceAssignmentSurvivesLaterAdapterRedescribe()
{
    SettingsModel model;

    // Both adapters declare device 1 in their raw, never-configured defaults.
    model.updateAdapterFromDescribe("adapterA", makeAdapterDescribeWithDefaultDevice("adapterA", 1));
    model.updateAdapterFromDescribe("adapterB", makeAdapterDescribeWithDefaultDevice("adapterB", 1));

    // adapterA wins the shared ID while neither adapter has an explicit config yet.
    QCOMPARE(model.deviceSettings(1)->adapterId(), QStringLiteral("adapterA"));

    // The user explicitly (re)assigns device 1 to adapterB and saves it: adapterB now has a
    // real, stored config declaring device 1, while adapterA's config is still untouched defaults.
    QJsonObject dev;
    dev["id"] = 1;
    QJsonObject config;
    config["general"] = QJsonObject();
    config["connections"] = QJsonArray();
    config["devices"] = QJsonArray{ dev };
    model.setAdapterCurrentConfig("adapterB", config);
    model.deviceSettings(1)->setAdapterId("adapterB");

    // adapterA reconnects and redescribes; its raw defaults still declare device 1, but the
    // explicit assignment to adapterB must survive rather than be silently reclaimed by adapterA.
    model.updateAdapterFromDescribe("adapterA", makeAdapterDescribeWithDefaultDevice("adapterA", 1));

    QCOMPARE(model.deviceSettings(1)->adapterId(), QStringLiteral("adapterB"));
}

void TestAdapterDeviceSettings::dialogBuildsTabFromReconciledOwnerNotFirstAdapter()
{
    SettingsModel model;

    // adapterA is alphabetically first and declares device 1 in its untouched defaults,
    // but is never explicitly configured.
    model.updateAdapterFromDescribe("adapterA", makeAdapterDescribeWithDefaultDevice("adapterA", 1));

    // adapterB has an explicit stored config that also declares device 1.
    QJsonObject devB;
    devB["id"] = 1;
    setupAdapter(model, "adapterB", QJsonArray{ devB });

    // AdapterDeviceSettings's constructor reconciles device ownership itself before
    // building tabs (see reconcileDevicesWithAdapters() called at construction time),
    // so it must resolve device 1 to adapterB (stored config wins) regardless of
    // adapter iteration order.
    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 1);

    // The single tab must be built from device 1's reconciled owner, adapterB — not from
    // adapterA, which merely happens to come first in adapter iteration order.
    auto* tab = qobject_cast<DeviceConfigTab*>(tabs->tabContent(0));
    QVERIFY(tab != nullptr);
    QCOMPARE(tab->adapterId(), QStringLiteral("adapterB"));

    w.acceptValues();

    // Accepting must not wipe adapterB's stored device out from under it, nor let adapterA
    // silently claim it.
    QCOMPARE(model.adapterData("adapterB")->currentConfig().value("devices").toArray().size(), 1);
    QCOMPARE(model.adapterData("adapterA")->currentConfig().value("devices").toArray().size(), 0);
}

void TestAdapterDeviceSettings::reassigningExistingDeviceOwnerEmitsDeviceListChanged()
{
    SettingsModel model;

    // adapterA describes first and, uncontested, claims device 1.
    model.updateAdapterFromDescribe("adapterA", makeAdapterDescribeWithDefaultDevice("adapterA", 1));
    QCOMPARE(model.deviceSettings(1)->adapterId(), QStringLiteral("adapterA"));

    // adapterB describes and saves an explicit stored config also declaring device 1
    // (setAdapterCurrentConfig() itself does not reconcile, so ownership doesn't move yet).
    model.updateAdapterFromDescribe("adapterB", makeAdapterDescribe("adapterB"));
    QJsonObject devB;
    devB["id"] = 1;
    QJsonObject configB;
    configB["general"] = QJsonObject();
    configB["connections"] = QJsonArray();
    configB["devices"] = QJsonArray{ devB };
    model.setAdapterCurrentConfig("adapterB", configB);
    QCOMPARE(model.deviceSettings(1)->adapterId(), QStringLiteral("adapterA"));

    QSignalSpy spy(&model, &SettingsModel::deviceListChanged);

    // adapterA reconnects and redescribes, re-running reconciliation: adapterB's stored
    // config now wins the tie over adapterA's still-untouched defaults. Device 1 already
    // existed in the model, so addDevice() alone won't emit — reconciliation must emit
    // deviceListChanged() itself when it silently reassigns an already-known device's owner.
    model.updateAdapterFromDescribe("adapterA", makeAdapterDescribeWithDefaultDevice("adapterA", 1));

    QCOMPARE(model.deviceSettings(1)->adapterId(), QStringLiteral("adapterB"));
    QCOMPARE(spy.count(), 1);
}

void TestAdapterDeviceSettings::addTabAfterDefaultDeviceKeepsOrder()
{
    SettingsModel model;

    // Single-adapter sanity check: handleAddTab() appends directly and never needs
    // reordering here, so this doesn't exercise sortPagesByDeviceId() itself (that's
    // covered by reopenAfterAdapterSwitchPreservesDeviceOrder below). This just guards
    // the common case: mirrors the real "modbus" adapter, with no stored config yet and
    // defaults that already contain one device (id=1), matching SettingsModel's
    // built-in device 1.
    QJsonObject describe = makeAdapterDescribe("modbus");
    QJsonObject defaultDevice;
    defaultDevice["id"] = 1;
    QJsonObject defaults;
    defaults["devices"] = QJsonArray{ defaultDevice };
    defaults["connections"] = QJsonArray();
    defaults["general"] = QJsonObject();
    describe["defaults"] = defaults;
    model.updateAdapterFromDescribe("modbus", describe);

    QVERIFY(!model.adapterData("modbus")->hasStoredConfig());

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 1);

    auto* firstTab = qobject_cast<DeviceConfigTab*>(tabs->tabContent(0));
    QVERIFY(firstTab != nullptr);
    QCOMPARE(firstTab->values().value("id").toInt(-1), 1);

    emit tabs->addTabRequested();

    QCOMPARE(tabs->count(), 2);

    // The original first device's tab must still be at index 0, unchanged.
    auto* tab0After = qobject_cast<DeviceConfigTab*>(tabs->tabContent(0));
    auto* tab1After = qobject_cast<DeviceConfigTab*>(tabs->tabContent(1));
    QVERIFY(tab0After != nullptr);
    QVERIFY(tab1After != nullptr);

    QCOMPARE(tab0After, firstTab); // same widget instance, still first
    QCOMPARE(tab0After->values().value("id").toInt(-1), 1);
    QCOMPARE(tab1After->values().value("id").toInt(-1), 2);
}

void TestAdapterDeviceSettings::reopenAfterAdapterSwitchPreservesDeviceOrder()
{
    SettingsModel model;

    // SettingsModel::adapterIds() returns adapters alphabetically, not in creation or
    // usage order, so pick adapter IDs where that alphabetical order differs from the
    // order devices are actually added in.
    QJsonObject dev1;
    dev1["id"] = 1;
    setupAdapter(model, "adapterA", QJsonArray{ dev1 });
    setupAdapter(model, "adapterZ", QJsonArray());

    // Session 1: switch device 1 (originally on adapterA) over to adapterZ,
    // then add a new device (always goes to adapterIds().first() == adapterA),
    // then accept (as OK would do).
    {
        AdapterDeviceSettings w(&model);
        auto* tabs = w.findChild<AddableTabWidget*>();
        QVERIFY(tabs != nullptr);
        QCOMPARE(tabs->count(), 1);

        auto* tab0 = qobject_cast<DeviceConfigTab*>(tabs->tabContent(0));
        QVERIFY(tab0 != nullptr);
        QCOMPARE(tab0->deviceId(), 1);

        auto* combo = tab0->findChild<QComboBox*>();
        QVERIFY(combo != nullptr);
        int zIdx = combo->findData(QStringLiteral("adapterZ"));
        QVERIFY(zIdx >= 0);
        combo->setCurrentIndex(zIdx);
        QCOMPARE(tab0->adapterId(), QStringLiteral("adapterZ"));

        emit tabs->addTabRequested(); // device 2, goes to adapterA (first() alphabetically)
        QCOMPARE(tabs->count(), 2);
        auto* tab1 = qobject_cast<DeviceConfigTab*>(tabs->tabContent(1));
        QVERIFY(tab1 != nullptr);
        QCOMPARE(tab1->deviceId(), 2);
        QCOMPARE(tab1->adapterId(), QStringLiteral("adapterA"));

        w.acceptValues();
    }

    // Session 2: reopen. Device 1 was added first and should still appear
    // before device 2 in the tab order.
    {
        AdapterDeviceSettings w2(&model);
        auto* tabs = w2.findChild<AddableTabWidget*>();
        QVERIFY(tabs != nullptr);
        QCOMPARE(tabs->count(), 2);

        auto* tab0 = qobject_cast<DeviceConfigTab*>(tabs->tabContent(0));
        auto* tab1 = qobject_cast<DeviceConfigTab*>(tabs->tabContent(1));
        QVERIFY(tab0 != nullptr);
        QVERIFY(tab1 != nullptr);

        QCOMPARE(tab0->deviceId(), 1); // device 1 (added first) must stay first
        QCOMPARE(tab1->deviceId(), 2);
    }
}

void TestAdapterDeviceSettings::invalidIdTabSortsAfterValidIntMaxIdTab()
{
    SettingsModel model;

    // adapterA is alphabetically first, so its tab is built before adapterB's. Its device
    // has no "id" field, giving it the invalid-ID sentinel used by sortPagesByDeviceId().
    setupAdapter(model, "adapterA", QJsonArray{ QJsonObject() });

    // adapterB's device has the largest valid ID an int can hold. A sentinel that collides
    // with INT_MAX (rather than one strictly wider than int) would tie with the invalid tab
    // above and let std::stable_sort keep the invalid tab first, violating the documented
    // invalid-ID-last rule.
    QJsonObject devB;
    devB["id"] = INT_MAX;
    setupAdapter(model, "adapterB", QJsonArray{ devB });

    AdapterDeviceSettings w(&model);
    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 2);

    auto* tab0 = qobject_cast<DeviceConfigTab*>(tabs->tabContent(0));
    auto* tab1 = qobject_cast<DeviceConfigTab*>(tabs->tabContent(1));
    QVERIFY(tab0 != nullptr);
    QVERIFY(tab1 != nullptr);

    QCOMPARE(tab0->deviceId(), INT_MAX); // valid ID, even at INT_MAX, must sort first
    QCOMPARE(tab1->deviceId(), -1);      // invalid ID must sort last
}

void TestAdapterDeviceSettings::invalidIdTabValuesOmitFabricatedId()
{
    SettingsModel model;

    // Device has no "id" field. The "id" spinbox is schema-bound with a minimum of 1, so a
    // naive read of its live value would clamp the missing id to 1 instead of preserving the
    // invalid-ID sentinel, silently fabricating a plausible-looking id.
    setupAdapter(model, "adapterA", QJsonArray{ QJsonObject() });

    AdapterDeviceSettings w(&model);
    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);

    auto* tab = qobject_cast<DeviceConfigTab*>(tabs->tabContent(0));
    QVERIFY(tab != nullptr);

    QCOMPARE(tab->values().value("id").toInt(-2), -1);
}

void TestAdapterDeviceSettings::addTabDefaultsToModbusEvenWhenNotFirstAlphabetically()
{
    SettingsModel model;

    // "aaaadapter" is alphabetically first, so validAdapterIds().first() would pick it over
    // "modbus" unless handleAddTab() explicitly prefers modbus.
    setupAdapter(model, "aaaadapter", QJsonArray());
    setupAdapter(model, "modbus", QJsonArray());

    AdapterDeviceSettings w(&model);
    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 0);

    emit tabs->addTabRequested();

    QCOMPARE(tabs->count(), 1);
    auto* tab = qobject_cast<DeviceConfigTab*>(tabs->tabContent(0));
    QVERIFY(tab != nullptr);
    QCOMPARE(tab->adapterId(), QStringLiteral("modbus"));
}

void TestAdapterDeviceSettings::initialDeviceReconciliationPrefersModbusOverDiscoveryOrder()
{
    SettingsModel model;

    // SettingsModel pre-populates device 1 on "modbus" (Device's constructor default).
    QCOMPARE(model.deviceSettings(1)->adapterId(), QStringLiteral("modbus"));

    // "aaaadapter" also declares default device 1 in its untouched defaults and describes
    // first (e.g. it responded before modbus during adapter discovery). Without a modbus
    // tie-break, reconcileDevicesWithAdapters() would reassign device 1 to "aaaadapter"
    // purely because it sorts alphabetically ahead of "modbus".
    model.updateAdapterFromDescribe("aaaadapter", makeAdapterDescribeWithDefaultDevice("aaaadapter", 1));
    model.updateAdapterFromDescribe("modbus", makeAdapterDescribeWithDefaultDevice("modbus", 1));

    QCOMPARE(model.deviceSettings(1)->adapterId(), QStringLiteral("modbus"));
}

void TestAdapterDeviceSettings::addButtonRemainsAvailableWhenAdapterOverLimit()
{
    SettingsModel model;

    QJsonObject dev0;
    dev0["id"] = 1;
    QJsonObject dev1;
    dev1["id"] = 2;

    model.updateAdapterFromDescribe("adapterA", makeAdapterDescribeWithMaxItems("adapterA", 1));
    QJsonObject config;
    config["general"] = QJsonObject();
    config["connections"] = QJsonArray();
    config["devices"] = QJsonArray{ dev0, dev1 };
    model.setAdapterCurrentConfig("adapterA", config);

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    // Both devices are loaded even though adapterA only allows 1 — no silent block.
    QCOMPARE(tabs->count(), 2);

    auto* addButton = qobject_cast<QToolButton*>(tabs->cornerWidget(Qt::TopLeftCorner));
    QVERIFY(addButton != nullptr);
    // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage) -- QVERIFY aborts if null
    QVERIFY(!addButton->isHidden());
}

void TestAdapterDeviceSettings::warningLabelShowsForOverLimitAdapter()
{
    SettingsModel model;

    QJsonObject dev0;
    dev0["id"] = 1;
    QJsonObject dev1;
    dev1["id"] = 2;

    model.updateAdapterFromDescribe("adapterA", makeAdapterDescribeWithMaxItems("adapterA", 1));
    QJsonObject config;
    config["general"] = QJsonObject();
    config["connections"] = QJsonArray();
    config["devices"] = QJsonArray{ dev0, dev1 };
    model.setAdapterCurrentConfig("adapterA", config);

    AdapterDeviceSettings w(&model);

    auto* warningLabel = w.findChild<QLabel*>("deviceLimitWarningLabel");
    QVERIFY(warningLabel != nullptr);
    QVERIFY(!warningLabel->isHidden());
    QVERIFY(warningLabel->text().contains("adapterA"));
    QVERIFY(warningLabel->text().contains("1"));
    QVERIFY(warningLabel->text().contains("2"));
}

void TestAdapterDeviceSettings::addingDeviceForModbusNotBlockedByOtherAdapterOverLimit()
{
    SettingsModel model;

    QJsonObject dev0;
    dev0["id"] = 1;
    QJsonObject dev1;
    dev1["id"] = 2;

    model.updateAdapterFromDescribe("dummy", makeAdapterDescribeWithMaxItems("dummy", 1));
    QJsonObject dummyConfig;
    dummyConfig["general"] = QJsonObject();
    dummyConfig["connections"] = QJsonArray();
    dummyConfig["devices"] = QJsonArray{ dev0, dev1 };
    model.setAdapterCurrentConfig("dummy", dummyConfig);

    setupAdapter(model, "modbus", QJsonArray());

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 2);

    // "dummy" is already over its limit of 1, but that must not stop a device from being
    // added for "modbus" (the default target adapter), which has no configured limit.
    emit tabs->addTabRequested();

    QCOMPARE(tabs->count(), 3);
    auto* newTab = qobject_cast<DeviceConfigTab*>(tabs->tabContent(2));
    QVERIFY(newTab != nullptr);
    QCOMPARE(newTab->adapterId(), QStringLiteral("modbus"));

    auto* addButton = qobject_cast<QToolButton*>(tabs->cornerWidget(Qt::TopLeftCorner));
    QVERIFY(addButton != nullptr);
    // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage) -- QVERIFY aborts if null
    QVERIFY(!addButton->isHidden());
}

/*!
 * \brief Regression test for the interaction between ProjectFileHandler's
 * applyAdapterSettings()/applyDeviceSettings() and this dialog's constructor-time pruning: when
 * an adapter's stored config and SettingsModel's generic device list already agree on the full
 * set of devices (as they now do after a project load, even beyond the adapter's maxItems), the
 * constructor must not silently remove any of them.
 */
void TestAdapterDeviceSettings::doesNotDropDevicesWhenAdapterConfigMatchesDeviceList()
{
    SettingsModel model;

    model.updateAdapterFromDescribe("modbus", makeAdapterDescribeWithMaxItems("modbus", 2));

    QJsonArray deviceArray;
    for (int i = 1; i <= 5; ++i)
    {
        QJsonObject dev;
        dev["id"] = i;
        deviceArray.append(dev);

        model.addDevice(static_cast<deviceId_t>(i));
        model.deviceSettings(static_cast<deviceId_t>(i))->setAdapterId("modbus");
    }

    QJsonObject config;
    config["general"] = QJsonObject();
    config["connections"] = QJsonArray();
    config["devices"] = deviceArray;
    model.setAdapterCurrentConfig("modbus", config);

    AdapterDeviceSettings w(&model);

    auto* tabs = w.findChild<AddableTabWidget*>();
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 5);
    QCOMPARE(model.deviceList().size(), 5);

    auto* warningLabel = w.findChild<QLabel*>("deviceLimitWarningLabel");
    QVERIFY(warningLabel != nullptr);
    QVERIFY(!warningLabel->isHidden());
}

QTEST_MAIN(TestAdapterDeviceSettings)
