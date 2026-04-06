
#include "tst_adapterdata.h"

#include "models/adapterdata.h"
#include "models/settingsmodel.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QTest>

void TestAdapterData::init()
{
}

void TestAdapterData::cleanup()
{
}

void TestAdapterData::defaultConstruction()
{
    AdapterData data;

    QVERIFY(data.name().isEmpty());
    QVERIFY(data.version().isEmpty());
    QCOMPARE(data.configVersion(), 0);
    QVERIFY(data.schema().isEmpty());
    QVERIFY(data.defaults().isEmpty());
    QVERIFY(data.capabilities().isEmpty());
    QVERIFY(data.currentConfig().isEmpty());
    QCOMPARE(data.hasStoredConfig(), false);
}

void TestAdapterData::updateFromDescribe()
{
    AdapterData data;

    QJsonObject caps;
    caps["supportsHotReload"] = false;
    caps["requiresRestartOn"] = QJsonArray{ "connections", "devices" };

    QJsonObject defaults;
    defaults["version"] = 1;
    defaults["connections"] = QJsonArray();

    QJsonObject schema;
    schema["type"] = "object";

    QJsonObject describeResult;
    describeResult["name"] = "modbusAdapter";
    describeResult["version"] = "1.0.0";
    describeResult["configVersion"] = 2;
    describeResult["schema"] = schema;
    describeResult["defaults"] = defaults;
    describeResult["capabilities"] = caps;

    data.updateFromDescribe(describeResult);

    QCOMPARE(data.name(), QStringLiteral("modbusAdapter"));
    QCOMPARE(data.version(), QStringLiteral("1.0.0"));
    QCOMPARE(data.configVersion(), 2);
    QCOMPARE(data.schema().value("type").toString(), QStringLiteral("object"));
    QCOMPARE(data.defaults().value("version").toInt(), 1);
    QCOMPARE(data.capabilities().value("supportsHotReload").toBool(), false);
}

void TestAdapterData::updateFromDescribeMissingFields()
{
    AdapterData data;

    /* Only name present, everything else missing */
    QJsonObject describeResult;
    describeResult["name"] = "minimalAdapter";

    data.updateFromDescribe(describeResult);

    QCOMPARE(data.name(), QStringLiteral("minimalAdapter"));
    QVERIFY(data.version().isEmpty());
    QCOMPARE(data.configVersion(), 0);
    QVERIFY(data.schema().isEmpty());
    QVERIFY(data.defaults().isEmpty());
    QVERIFY(data.capabilities().isEmpty());
}

void TestAdapterData::effectiveConfigReturnsDefaults()
{
    AdapterData data;

    QJsonObject defaults;
    defaults["version"] = 1;
    defaults["setting"] = "default_value";

    QJsonObject describeResult;
    describeResult["name"] = "testAdapter";
    describeResult["defaults"] = defaults;

    data.updateFromDescribe(describeResult);

    /* No stored config → effectiveConfig should return defaults */
    QCOMPARE(data.hasStoredConfig(), false);
    QJsonObject config = data.effectiveConfig();
    QCOMPARE(config["setting"].toString(), QStringLiteral("default_value"));
}

void TestAdapterData::effectiveConfigReturnsStoredConfig()
{
    AdapterData data;

    QJsonObject defaults;
    defaults["setting"] = "default_value";

    QJsonObject describeResult;
    describeResult["name"] = "testAdapter";
    describeResult["defaults"] = defaults;

    data.updateFromDescribe(describeResult);

    /* Set stored config */
    QJsonObject storedConfig;
    storedConfig["setting"] = "stored_value";
    data.setCurrentConfig(storedConfig);
    data.setHasStoredConfig(true);

    QJsonObject config = data.effectiveConfig();
    QCOMPARE(config["setting"].toString(), QStringLiteral("stored_value"));
}

void TestAdapterData::settingsModelAdapterDataCreatesEntry()
{
    SettingsModel model;

    /* First access creates a default entry */
    const AdapterData* data = model.adapterData("modbus");
    QVERIFY(data != nullptr);
    // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage) -- QVERIFY aborts if null
    QVERIFY(data->name().isEmpty());

    /* updateAdapterFromDescribe updates the same entry in place */
    QJsonObject desc;
    desc["name"] = QStringLiteral("modbusAdapter");
    desc["version"] = QString();
    desc["configVersion"] = 0;
    desc["schema"] = QJsonObject();
    desc["defaults"] = QJsonObject();
    desc["capabilities"] = QJsonObject();
    model.updateAdapterFromDescribe("modbus", desc);

    const AdapterData* data2 = model.adapterData("modbus");
    QCOMPARE(data2->name(), QStringLiteral("modbusAdapter"));
}

void TestAdapterData::settingsModelAdapterIds()
{
    SettingsModel model;

    QVERIFY(model.adapterIds().isEmpty());

    model.adapterData("modbus");
    model.adapterData("opcua");

    QStringList ids = model.adapterIds();
    QCOMPARE(ids.size(), 2);
    QVERIFY(ids.contains("modbus"));
    QVERIFY(ids.contains("opcua"));
}

void TestAdapterData::settingsModelRemoveAdapter()
{
    SettingsModel model;

    model.adapterData("modbus");
    QCOMPARE(model.adapterIds().size(), 1);

    model.removeAdapter("modbus");
    QVERIFY(model.adapterIds().isEmpty());
}

void TestAdapterData::registerSchemaDefaultEmpty()
{
    AdapterData data;
    QVERIFY(data.registerSchema().isEmpty());
}

void TestAdapterData::setAndGetRegisterSchema()
{
    AdapterData data;

    QJsonObject addressSchema;
    addressSchema["type"] = QStringLiteral("object");

    QJsonArray dataTypes;
    QJsonObject type16b;
    type16b["id"] = QStringLiteral("16b");
    type16b["label"] = QStringLiteral("Unsigned 16-bit");
    dataTypes.append(type16b);

    QJsonObject schema;
    schema["addressSchema"] = addressSchema;
    schema["dataTypes"] = dataTypes;
    schema["defaultDataType"] = QStringLiteral("16b");

    data.setRegisterSchema(schema);

    const QJsonObject stored = data.registerSchema();
    QCOMPARE(stored["defaultDataType"].toString(), QStringLiteral("16b"));
    QCOMPARE(stored["dataTypes"].toArray().size(), 1);
}

void TestAdapterData::settingsModelSetAdapterRegisterSchema()
{
    SettingsModel model;

    QJsonObject schema;
    schema["defaultDataType"] = QStringLiteral("16b");

    model.setAdapterRegisterSchema("modbus", schema);

    const AdapterData* data = model.adapterData("modbus");
    const QJsonObject stored = data->registerSchema();
    QCOMPARE(stored["defaultDataType"].toString(), QStringLiteral("16b"));
}

void TestAdapterData::deviceAdapterIdDefaultsToModbus()
{
    SettingsModel model;

    QCOMPARE(model.deviceSettings(Device::cFirstDeviceId)->adapterId(), QStringLiteral("modbus"));
}

void TestAdapterData::deviceSetAndGetAdapterId()
{
    Device device;
    QCOMPARE(device.adapterId(), QStringLiteral("modbus"));

    device.setAdapterId("custom");
    QCOMPARE(device.adapterId(), QStringLiteral("custom"));
}

void TestAdapterData::deviceListForAdapterFiltersCorrectly()
{
    SettingsModel model;

    /* Default device 1 is modbus; add two more */
    const deviceId_t idTwo = model.addNewDevice();
    const deviceId_t idThree = model.addNewDevice();

    model.deviceSettings(Device::cFirstDeviceId)->setAdapterId("modbus");
    model.deviceSettings(idTwo)->setAdapterId("custom");
    model.deviceSettings(idThree)->setAdapterId("modbus");

    const QList<deviceId_t> modbusList = model.deviceListForAdapter("modbus");
    QCOMPARE(modbusList.size(), 2);
    QVERIFY(modbusList.contains(Device::cFirstDeviceId));
    QVERIFY(modbusList.contains(idThree));

    const QList<deviceId_t> customList = model.deviceListForAdapter("custom");
    QCOMPARE(customList.size(), 1);
    QVERIFY(customList.contains(idTwo));
}

void TestAdapterData::deviceListForAdapterUnknownReturnsEmpty()
{
    SettingsModel model;

    QVERIFY(model.deviceListForAdapter("opcua").isEmpty());
}

QTEST_GUILESS_MAIN(TestAdapterData)
