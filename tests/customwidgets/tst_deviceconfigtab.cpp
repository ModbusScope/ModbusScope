#include "tst_deviceconfigtab.h"

#include "customwidgets/deviceconfigtab.h"
#include "models/settingsmodel.h"

#include <QComboBox>
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

    QCOMPARE(tab.values()["name"].toString(), QStringLiteral("Pump"));
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

QTEST_MAIN(TestDeviceConfigTab)
