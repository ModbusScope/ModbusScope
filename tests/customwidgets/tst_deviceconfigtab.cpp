#include "tst_deviceconfigtab.h"

#include "../models/devicelisthelpers.h"
#include "customwidgets/deviceconfigtab.h"
#include "models/adapterconfigdraft.h"
#include "models/settingsmodel.h"

#include <QComboBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QLineEdit>
#include <QSignalSpy>
#include <QSpinBox>
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

//! Like makeAdapterDescribe() but the device schema also has a \c connectionId that references
//! the \c connections collection.
QJsonObject makeAdapterDescribeWithConnectionRef(const QString& adapterName)
{
    QJsonObject describe = makeAdapterDescribe(adapterName);

    QJsonObject xRef;
    xRef["collection"] = "connections";
    xRef["value"] = "id";
    xRef["label"] = "name";

    QJsonObject connProp;
    connProp["type"] = "integer";
    connProp["title"] = "Connection";
    connProp["x-ref"] = xRef;

    QJsonObject schema = describe["schema"].toObject();
    QJsonObject topProps = schema["properties"].toObject();
    QJsonObject devices = topProps["devices"].toObject();
    QJsonObject items = devices["items"].toObject();
    QJsonObject itemProps = items["properties"].toObject();
    itemProps["connectionId"] = connProp;
    items["properties"] = itemProps;
    devices["items"] = items;
    topProps["devices"] = devices;
    schema["properties"] = topProps;
    describe["schema"] = schema;
    return describe;
}

//! Like makeAdapterDescribe() but the device schema also has an integer \c id, as real adapters declare.
QJsonObject makeAdapterDescribeWithIdField(const QString& adapterName)
{
    QJsonObject describe = makeAdapterDescribe(adapterName);

    QJsonObject idProp;
    idProp["type"] = "integer";
    idProp["title"] = "Device ID";
    idProp["minimum"] = 1;

    QJsonObject schema = describe["schema"].toObject();
    QJsonObject topProps = schema["properties"].toObject();
    QJsonObject devices = topProps["devices"].toObject();
    QJsonObject items = devices["items"].toObject();
    QJsonObject itemProps = items["properties"].toObject();
    itemProps["id"] = idProp;
    items["properties"] = itemProps;
    devices["items"] = items;
    topProps["devices"] = devices;
    schema["properties"] = topProps;
    describe["schema"] = schema;
    return describe;
}

//! Return the first combo under \a tab that has an item with \a text, or nullptr.
QComboBox* findCombo(const DeviceConfigTab& tab, const QString& text)
{
    const QList<QComboBox*> combos = tab.findChildren<QComboBox*>();
    for (QComboBox* combo : combos)
    {
        if (combo->findText(text) >= 0)
        {
            return combo;
        }
    }
    return nullptr;
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

    DeviceConfigTab tab(&model, "adapterA", QString(), QJsonObject());

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

void TestDeviceConfigTab::comboShowsProtocolNames()
{
    SettingsModel model;
    QJsonObject describe = makeAdapterDescribe("modbusAdapter");
    describe["protocolName"] = "modbus";
    model.updateAdapterFromDescribe("adapterA", describe);

    DeviceConfigTab tab(&model, "adapterA", QString(), QJsonObject());

    auto* combo = tab.findChild<QComboBox*>(QString(), Qt::FindDirectChildrenOnly);
    QVERIFY(combo != nullptr);
    QCOMPARE(combo->count(), 1);
    QCOMPARE(combo->itemText(0), QStringLiteral("modbus"));
    QCOMPARE(combo->itemData(0).toString(), QStringLiteral("adapterA"));
}

void TestDeviceConfigTab::comboDisambiguatesDuplicateProtocolNames()
{
    SettingsModel model;
    QJsonObject describe = makeAdapterDescribe("modbusAdapter");
    describe["protocolName"] = "modbus";
    model.updateAdapterFromDescribe("adapterA", describe);
    model.updateAdapterFromDescribe("adapterB", describe);

    DeviceConfigTab tab(&model, "adapterB", QString(), QJsonObject());

    auto* combo = tab.findChild<QComboBox*>(QString(), Qt::FindDirectChildrenOnly);
    QVERIFY(combo != nullptr);
    QCOMPARE(combo->count(), 2);
    QCOMPARE(combo->itemText(0), QStringLiteral("modbus (adapterA)"));
    QCOMPARE(combo->itemData(0).toString(), QStringLiteral("adapterA"));
    QCOMPARE(combo->itemText(1), QStringLiteral("modbus (adapterB)"));
    QCOMPARE(combo->itemData(1).toString(), QStringLiteral("adapterB"));
    QCOMPARE(tab.adapterId(), QStringLiteral("adapterB"));
}

void TestDeviceConfigTab::constructorSelectsCorrectAdapter()
{
    SettingsModel model;
    setupTwoAdapters(model);

    DeviceConfigTab tab(&model, "adapterB", QString(), QJsonObject());

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

    DeviceConfigTab tab(&model, "adapterA", QString(), deviceValues);

    QCOMPARE(tab.values().value("name").toString(), QStringLiteral("Pump"));
}

void TestDeviceConfigTab::adapterIdMatchesComboInitially()
{
    SettingsModel model;
    setupTwoAdapters(model);

    DeviceConfigTab tab(&model, "adapterA", QString(), QJsonObject());

    auto* combo = tab.findChild<QComboBox*>(QString(), Qt::FindDirectChildrenOnly);
    QVERIFY(combo != nullptr);
    QCOMPARE(tab.adapterId(), combo->currentData().toString());
}

void TestDeviceConfigTab::deviceNameInitializesFromConstructorArgument()
{
    SettingsModel model;
    setupTwoAdapters(model);

    // The tab holds the working copy, so its owner supplies the name rather than the tab
    // reading it back from SettingsModel. A device with a name it has not been given shows none.
    DeviceListHelpers::seedDevice(&model, 5, QString(), QStringLiteral("Pump Station"));

    QJsonObject deviceValues;
    deviceValues["id"] = 5;

    DeviceConfigTab named(&model, "adapterA", "Booster Pump", deviceValues);
    QCOMPARE(named.deviceName(), QStringLiteral("Booster Pump"));

    DeviceConfigTab unnamed(&model, "adapterA", QString(), deviceValues);
    QVERIFY(unnamed.deviceName().isEmpty());
}

void TestDeviceConfigTab::nameChangeDoesNotWriteModel()
{
    SettingsModel model;
    setupTwoAdapters(model);
    DeviceListHelpers::seedDevice(&model, 5, QString(), QStringLiteral("Pump Station"));

    QJsonObject deviceValues;
    deviceValues["id"] = 5;

    DeviceConfigTab tab(&model, "adapterA", "Pump Station", deviceValues);
    QSignalSpy spy(&tab, &DeviceConfigTab::nameChanged);

    auto* nameEdit = tab.findChild<QLineEdit*>(QString(), Qt::FindDirectChildrenOnly);
    QVERIFY(nameEdit != nullptr);
    nameEdit->setText("Renamed");

    QCOMPARE(spy.count(), 1);
    QCOMPARE(tab.deviceName(), QStringLiteral("Renamed"));

    // The edit lives only in the tab until the owning page accepts it.
    QCOMPARE(model.deviceSettings(5)->name(), QStringLiteral("Pump Station"));
}

void TestDeviceConfigTab::adapterChangeDoesNotWriteModel()
{
    SettingsModel model;
    setupTwoAdapters(model);
    DeviceListHelpers::seedDevice(&model, 5, QStringLiteral("adapterA"));

    QJsonObject deviceValues;
    deviceValues["id"] = 5;

    DeviceConfigTab tab(&model, "adapterA", QString(), deviceValues);
    QSignalSpy spy(&tab, &DeviceConfigTab::adapterChanged);

    auto* combo = tab.findChild<QComboBox*>(QString(), Qt::FindDirectChildrenOnly);
    QVERIFY(combo != nullptr);
    combo->setCurrentIndex(combo->findData(QStringLiteral("adapterB")));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(tab.adapterId(), QStringLiteral("adapterB"));

    // The reassignment lives only in the tab until the owning page accepts it.
    QCOMPARE(model.deviceSettings(5)->adapterId(), QStringLiteral("adapterA"));
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

    DeviceConfigTab tab(&model, "adapterA", QString(), QJsonObject());

    auto* combo = tab.findChild<QComboBox*>(QString(), Qt::FindDirectChildrenOnly);
    QVERIFY(combo != nullptr);

    combo->setCurrentIndex(combo->findData(QStringLiteral("adapterB")));

    QCOMPARE(tab.values().value("name").toString(), QStringLiteral("defaultName"));
}

void TestDeviceConfigTab::idFieldIsReadOnly()
{
    SettingsModel model;
    model.updateAdapterFromDescribe("adapterA", makeAdapterDescribeWithIdField("adapterA"));

    DeviceConfigTab tab(&model, "adapterA", QString(), QJsonObject{ { "id", 5 } });

    const QList<QSpinBox*> spins = tab.findChildren<QSpinBox*>();
    QCOMPARE(spins.size(), 1);
    QCOMPARE(spins.first()->value(), 5);
    QVERIFY(!spins.first()->isEnabled());
}

void TestDeviceConfigTab::idFieldStaysReadOnlyAfterAdapterChange()
{
    SettingsModel model;
    model.updateAdapterFromDescribe("adapterA", makeAdapterDescribeWithIdField("adapterA"));
    model.updateAdapterFromDescribe("adapterB", makeAdapterDescribeWithIdField("adapterB"));

    DeviceConfigTab tab(&model, "adapterA", QString(), QJsonObject{ { "id", 5 } });

    auto* combo = tab.findChild<QComboBox*>(QString(), Qt::FindDirectChildrenOnly);
    QVERIFY(combo != nullptr);
    combo->setCurrentIndex(combo->findData(QStringLiteral("adapterB")));

    // The previous form is released with deleteLater(), so wait until only the rebuilt one remains.
    QTRY_COMPARE(tab.findChildren<QSpinBox*>().size(), 1);
    QSpinBox* spin = tab.findChildren<QSpinBox*>().constFirst();
    QCOMPARE(spin->value(), 5);
    QVERIFY(!spin->isEnabled());
}

void TestDeviceConfigTab::connectionIdRendersAsComboWithConnectionNames()
{
    SettingsModel model;
    model.updateAdapterFromDescribe("adapterA", makeAdapterDescribeWithConnectionRef("adapterA"));

    QJsonArray connections;
    connections.append(QJsonObject{ { "id", 1 }, { "name", "Main" } });
    connections.append(QJsonObject{ { "id", 2 }, { "name", "Backup" } });
    model.setAdapterCurrentConfig("adapterA", QJsonObject{ { "connections", connections } });

    DeviceConfigTab tab(&model, "adapterA", QString(), QJsonObject{ { "connectionId", 2 } });

    // Search by item text so the adapter selector combo is skipped.
    QComboBox* refCombo = findCombo(tab, "Backup");
    QVERIFY(refCombo != nullptr);
    QCOMPARE(refCombo->count(), 2);
    QCOMPARE(refCombo->currentText(), QStringLiteral("Backup"));
    QCOMPARE(tab.values().value("connectionId").toInt(), 2);
}

void TestDeviceConfigTab::connectionComboFollowsModelConfigChange()
{
    SettingsModel model;
    model.updateAdapterFromDescribe("adapterA", makeAdapterDescribeWithConnectionRef("adapterA"));
    model.setAdapterCurrentConfig(
      "adapterA", QJsonObject{ { "connections", QJsonArray{ QJsonObject{ { "id", 1 }, { "name", "Main" } } } } });

    DeviceConfigTab tab(&model, "adapterA", QString(), QJsonObject{ { "connectionId", 1 } });
    QVERIFY(findCombo(tab, "Main") != nullptr);

    QJsonArray connections;
    connections.append(QJsonObject{ { "id", 1 }, { "name", "Renamed" } });
    connections.append(QJsonObject{ { "id", 2 }, { "name", "Backup" } });
    model.setAdapterCurrentConfig("adapterA", QJsonObject{ { "connections", connections } });

    QComboBox* refCombo = findCombo(tab, "Backup");
    QVERIFY(refCombo != nullptr);
    QCOMPARE(refCombo->count(), 2);
    QCOMPARE(refCombo->currentText(), QStringLiteral("Renamed"));
    QCOMPARE(tab.values().value("connectionId").toInt(), 1);
}

void TestDeviceConfigTab::connectionComboFollowsDraftSection()
{
    SettingsModel model;
    model.updateAdapterFromDescribe("adapterA", makeAdapterDescribeWithConnectionRef("adapterA"));
    model.setAdapterCurrentConfig(
      "adapterA", QJsonObject{ { "connections", QJsonArray{ QJsonObject{ { "id", 1 }, { "name", "Main" } } } } });
    AdapterConfigDraft draft(&model);

    DeviceConfigTab tab(&model, "adapterA", QString(), QJsonObject{ { "connectionId", 1 } }, nullptr, &draft);

    QJsonArray pending;
    pending.append(QJsonObject{ { "id", 1 }, { "name", "Main" } });
    pending.append(QJsonObject{ { "id", 2 }, { "name", "Unsaved" } });
    draft.setSection("adapterA", "connections", pending);

    QComboBox* refCombo = findCombo(tab, "Unsaved");
    QVERIFY(refCombo != nullptr);
    QCOMPARE(refCombo->count(), 2);
    QCOMPARE(tab.values().value("connectionId").toInt(), 1);

    // The saved model config is untouched
    QCOMPARE(model.adapterData("adapterA")->effectiveConfig().value("connections").toArray().size(), 1);
}

void TestDeviceConfigTab::draftSectionOfOtherKeyKeepsConnections()
{
    SettingsModel model;
    model.updateAdapterFromDescribe("adapterA", makeAdapterDescribeWithConnectionRef("adapterA"));
    model.setAdapterCurrentConfig(
      "adapterA", QJsonObject{ { "connections", QJsonArray{ QJsonObject{ { "id", 1 }, { "name", "Main" } } } } });
    AdapterConfigDraft draft(&model);
    DeviceConfigTab tab(&model, "adapterA", QString(), QJsonObject{ { "connectionId", 1 } }, nullptr, &draft);

    draft.setSection("adapterA", "connections", QJsonArray{ QJsonObject{ { "id", 1 }, { "name", "Edited" } } });
    draft.setSection("adapterA", "general", QJsonObject{ { "poll", 500 } });

    QVERIFY(findCombo(tab, "Edited") != nullptr);
}

void TestDeviceConfigTab::draftOfOtherAdapterIsIgnored()
{
    SettingsModel model;
    model.updateAdapterFromDescribe("adapterA", makeAdapterDescribeWithConnectionRef("adapterA"));
    model.updateAdapterFromDescribe("adapterB", makeAdapterDescribeWithConnectionRef("adapterB"));
    model.setAdapterCurrentConfig(
      "adapterA", QJsonObject{ { "connections", QJsonArray{ QJsonObject{ { "id", 1 }, { "name", "Main" } } } } });
    AdapterConfigDraft draft(&model);
    DeviceConfigTab tab(&model, "adapterA", QString(), QJsonObject{ { "connectionId", 1 } }, nullptr, &draft);

    draft.setSection("adapterB", "connections", QJsonArray{ QJsonObject{ { "id", 7 }, { "name", "Elsewhere" } } });

    QVERIFY(findCombo(tab, "Elsewhere") == nullptr);
    QVERIFY(findCombo(tab, "Main") != nullptr);
}

void TestDeviceConfigTab::removedConnectionStaysSelectedAsMissing()
{
    SettingsModel model;
    model.updateAdapterFromDescribe("adapterA", makeAdapterDescribeWithConnectionRef("adapterA"));
    QJsonArray connections;
    connections.append(QJsonObject{ { "id", 1 }, { "name", "Main" } });
    connections.append(QJsonObject{ { "id", 2 }, { "name", "Backup" } });
    model.setAdapterCurrentConfig("adapterA", QJsonObject{ { "connections", connections } });
    AdapterConfigDraft draft(&model);
    DeviceConfigTab tab(&model, "adapterA", QString(), QJsonObject{ { "connectionId", 2 } }, nullptr, &draft);

    draft.setSection("adapterA", "connections", QJsonArray{ QJsonObject{ { "id", 1 }, { "name", "Main" } } });

    QComboBox* refCombo = findCombo(tab, "Main");
    QVERIFY(refCombo != nullptr);
    QVERIFY(refCombo->currentText().contains("missing"));
    QCOMPARE(tab.values().value("connectionId").toInt(), 2);
}

QTEST_MAIN(TestDeviceConfigTab)
