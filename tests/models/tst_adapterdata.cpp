
#include "tst_adapterdata.h"

#include "models/adapterdata.h"
#include "models/settingsmodel.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QTest>

#include <climits>

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
    QVERIFY(data.license().isEmpty());
    QVERIFY(data.currentConfig().isEmpty());
    QCOMPARE(data.hasStoredConfig(), false);
}

void TestAdapterData::updateFromDescribe()
{
    AdapterData data;

    QJsonObject caps;
    caps["mbcCompatible"] = true;

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
    QCOMPARE(data.capabilities().value("mbcCompatible").toBool(), true);
    QVERIFY(data.isMbcCompatible());
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
    QVERIFY(data.license().isEmpty());
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

void TestAdapterData::effectiveConfigFillsMissingKeysFromDefaults()
{
    AdapterData data;

    QJsonObject defaults;
    defaults["general"] = QJsonObject();
    defaults["connections"] = QJsonArray{ QStringLiteral("default_connection") };

    QJsonObject describeResult;
    describeResult["name"] = "testAdapter";
    describeResult["defaults"] = defaults;
    data.updateFromDescribe(describeResult);

    /* Stored config from an old project file: missing the "general" key */
    QJsonObject storedConfig;
    storedConfig["connections"] = QJsonArray{ QStringLiteral("stored_connection") };
    data.setCurrentConfig(storedConfig);
    data.setHasStoredConfig(true);

    QJsonObject config = data.effectiveConfig();

    /* "general" must be filled in from defaults */
    QVERIFY(config.value("general").isObject());
    /* "connections" must come from the stored config, not from defaults */
    QCOMPARE(config.value("connections").toArray(), QJsonArray{ QStringLiteral("stored_connection") });
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

void TestAdapterData::dataPointSchemaDefaultEmpty()
{
    AdapterData data;
    QVERIFY(data.dataPointSchema().isEmpty());
}

void TestAdapterData::setAndGetDataPointSchema()
{
    AdapterData data;

    QJsonObject addressSchema;
    addressSchema["type"] = QStringLiteral("object");

    QJsonObject defaults;
    defaults["dataType"] = QStringLiteral("16b");

    QJsonObject schema;
    schema["addressSchema"] = addressSchema;
    schema["defaults"] = defaults;

    data.setDataPointSchema(schema);

    const QJsonObject stored = data.dataPointSchema();
    QCOMPARE(stored["defaults"].toObject().value("dataType").toString(), QStringLiteral("16b"));
    QVERIFY(stored.contains(QStringLiteral("addressSchema")));
}

void TestAdapterData::settingsModelSetAdapterDataPointSchema()
{
    SettingsModel model;

    QJsonObject defaults;
    defaults["dataType"] = QStringLiteral("16b");

    QJsonObject addressSchema;
    addressSchema["type"] = QStringLiteral("object");

    QJsonObject schema;
    schema["addressSchema"] = addressSchema;
    schema["defaults"] = defaults;

    model.setAdapterDataPointSchema("modbus", schema);

    const AdapterData* data = model.adapterData("modbus");
    const QJsonObject stored = data->dataPointSchema();
    QCOMPARE(stored["defaults"].toObject().value("dataType").toString(), QStringLiteral("16b"));
    QCOMPARE(stored["addressSchema"].toObject().value("type").toString(), QStringLiteral("object"));
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

/*!
 * \brief maxDevicesFromSchema returns INT_MAX when the schema has no devices.maxItems.
 */
void TestAdapterData::maxDevicesFromSchemaReturnsIntMaxWhenAbsent()
{
    AdapterData data;

    QJsonObject schema;
    schema["type"] = "object";
    QJsonObject describeResult;
    describeResult["schema"] = schema;
    data.updateFromDescribe(describeResult);

    QCOMPARE(data.maxDevicesFromSchema(), INT_MAX);
}

/*!
 * \brief maxDevicesFromSchema returns the maxItems value from schema.properties.devices.
 */
void TestAdapterData::maxDevicesFromSchemaReturnsValue()
{
    AdapterData data;

    QJsonObject devicesSchema;
    devicesSchema["type"] = "array";
    devicesSchema["maxItems"] = 3;

    QJsonObject properties;
    properties["devices"] = devicesSchema;

    QJsonObject schema;
    schema["type"] = "object";
    schema["properties"] = properties;

    QJsonObject describeResult;
    describeResult["schema"] = schema;
    data.updateFromDescribe(describeResult);

    QCOMPARE(data.maxDevicesFromSchema(), 3);
}

/*!
 * \brief maxDevicesFromSchema returns INT_MAX when maxItems is negative, since a negative
 * limit is invalid schema data rather than a real cap.
 */
void TestAdapterData::maxDevicesFromSchemaReturnsIntMaxWhenNegative()
{
    AdapterData data;

    QJsonObject devicesSchema;
    devicesSchema["type"] = "array";
    devicesSchema["maxItems"] = -1;

    QJsonObject properties;
    properties["devices"] = devicesSchema;

    QJsonObject schema;
    schema["type"] = "object";
    schema["properties"] = properties;

    QJsonObject describeResult;
    describeResult["schema"] = schema;
    data.updateFromDescribe(describeResult);

    QCOMPARE(data.maxDevicesFromSchema(), INT_MAX);
}

/*!
 * \brief maxDevicesFromSchema returns 0 when maxItems is explicitly 0, since that is a
 * legitimate limit ("no devices allowed") rather than invalid schema data.
 */
void TestAdapterData::maxDevicesFromSchemaReturnsZeroWhenExplicit()
{
    AdapterData data;

    QJsonObject devicesSchema;
    devicesSchema["type"] = "array";
    devicesSchema["maxItems"] = 0;

    QJsonObject properties;
    properties["devices"] = devicesSchema;

    QJsonObject schema;
    schema["type"] = "object";
    schema["properties"] = properties;

    QJsonObject describeResult;
    describeResult["schema"] = schema;
    data.updateFromDescribe(describeResult);

    QCOMPARE(data.maxDevicesFromSchema(), 0);
}

/*!
 * \brief maxDevicesFromCapabilities returns INT_MAX when capabilities has no maxDevices.
 */
void TestAdapterData::maxDevicesFromCapabilitiesReturnsIntMaxWhenAbsent()
{
    AdapterData data;

    QJsonObject describeResult;
    describeResult["capabilities"] = QJsonObject();
    data.updateFromDescribe(describeResult);

    QCOMPARE(data.maxDevicesFromCapabilities(), INT_MAX);
}

/*!
 * \brief maxDevicesFromCapabilities returns the maxDevices value from capabilities.
 */
void TestAdapterData::maxDevicesFromCapabilitiesReturnsValue()
{
    AdapterData data;

    QJsonObject caps;
    caps["maxDevices"] = 2;

    QJsonObject describeResult;
    describeResult["capabilities"] = caps;
    data.updateFromDescribe(describeResult);

    QCOMPARE(data.maxDevicesFromCapabilities(), 2);
}

/*!
 * \brief maxDevicesFromCapabilities returns INT_MAX when maxDevices is negative, since a
 * negative limit is invalid capability data rather than a real cap.
 */
void TestAdapterData::maxDevicesFromCapabilitiesReturnsIntMaxWhenNegative()
{
    AdapterData data;

    QJsonObject caps;
    caps["maxDevices"] = -1;

    QJsonObject describeResult;
    describeResult["capabilities"] = caps;
    data.updateFromDescribe(describeResult);

    QCOMPARE(data.maxDevicesFromCapabilities(), INT_MAX);
}

/*!
 * \brief maxDevicesFromCapabilities returns 0 when maxDevices is explicitly 0, since that is a
 * legitimate limit ("no devices allowed") rather than invalid capability data.
 */
void TestAdapterData::maxDevicesFromCapabilitiesReturnsZeroWhenExplicit()
{
    AdapterData data;

    QJsonObject caps;
    caps["maxDevices"] = 0;

    QJsonObject describeResult;
    describeResult["capabilities"] = caps;
    data.updateFromDescribe(describeResult);

    QCOMPARE(data.maxDevicesFromCapabilities(), 0);
}

/*!
 * \brief maxDevices returns the smaller of maxDevicesFromSchema() and
 * maxDevicesFromCapabilities(), reproducing the real Modbus adapter's describe response where
 * the schema's maxItems (a fixed structural cap) is larger than the license-aware
 * capabilities.maxDevices actually enforced by adapter.configure.
 */
void TestAdapterData::maxDevicesReturnsSmallerOfSchemaAndCapabilities()
{
    AdapterData data;

    QJsonObject devicesSchema;
    devicesSchema["type"] = "array";
    devicesSchema["maxItems"] = 99;
    QJsonObject properties;
    properties["devices"] = devicesSchema;
    QJsonObject schema;
    schema["type"] = "object";
    schema["properties"] = properties;

    QJsonObject caps;
    caps["maxDevices"] = 2;

    QJsonObject describeResult;
    describeResult["schema"] = schema;
    describeResult["capabilities"] = caps;
    data.updateFromDescribe(describeResult);

    QCOMPARE(data.maxDevices(), 2);
}

/*!
 * \brief maxRegisters returns INT_MAX when capabilities has no maxRegisters, which is how a
 * licensed adapter reports that it enforces no data point limit at all.
 */
void TestAdapterData::maxRegistersReturnsIntMaxWhenAbsent()
{
    AdapterData data;

    QJsonObject describeResult;
    describeResult["capabilities"] = QJsonObject();
    data.updateFromDescribe(describeResult);

    QCOMPARE(data.maxRegisters(), INT_MAX);
}

/*!
 * \brief maxRegisters returns the maxRegisters value from capabilities.
 */
void TestAdapterData::maxRegistersReturnsValue()
{
    AdapterData data;

    QJsonObject caps;
    caps["maxRegisters"] = 5;

    QJsonObject describeResult;
    describeResult["capabilities"] = caps;
    data.updateFromDescribe(describeResult);

    QCOMPARE(data.maxRegisters(), 5);
}

/*!
 * \brief maxRegisters returns INT_MAX when maxRegisters is negative, since a negative limit is
 * invalid capability data rather than a real cap.
 */
void TestAdapterData::maxRegistersReturnsIntMaxWhenNegative()
{
    AdapterData data;

    QJsonObject caps;
    caps["maxRegisters"] = -1;

    QJsonObject describeResult;
    describeResult["capabilities"] = caps;
    data.updateFromDescribe(describeResult);

    QCOMPARE(data.maxRegisters(), INT_MAX);
}

/*!
 * \brief maxRegisters returns 0 when maxRegisters is explicitly 0, since that is a legitimate
 * limit ("no data points allowed") rather than invalid capability data.
 */
void TestAdapterData::maxRegistersReturnsZeroWhenExplicit()
{
    AdapterData data;

    QJsonObject caps;
    caps["maxRegisters"] = 0;

    QJsonObject describeResult;
    describeResult["capabilities"] = caps;
    data.updateFromDescribe(describeResult);

    QCOMPARE(data.maxRegisters(), 0);
}

/*!
 * \brief configForWire caps the "devices" array to maxDevicesFromSchema(), unlike
 * effectiveConfig() which returns every stored device.
 */
void TestAdapterData::configForWireCapsDevicesToMaxItems()
{
    AdapterData data;

    QJsonObject devicesSchema;
    devicesSchema["type"] = "array";
    devicesSchema["maxItems"] = 2;
    QJsonObject properties;
    properties["devices"] = devicesSchema;
    QJsonObject schema;
    schema["type"] = "object";
    schema["properties"] = properties;

    QJsonObject describeResult;
    describeResult["schema"] = schema;
    data.updateFromDescribe(describeResult);

    QJsonArray devices;
    for (int i = 1; i <= 5; ++i)
    {
        QJsonObject dev;
        dev["id"] = i;
        devices.append(dev);
    }
    QJsonObject config;
    config["devices"] = devices;
    data.setCurrentConfig(config);
    data.setHasStoredConfig(true);

    QCOMPARE(data.effectiveConfig().value("devices").toArray().size(), 5);
    QCOMPARE(data.configForWire().value("devices").toArray().size(), 2);
}

/*!
 * \brief configForWire returns every device unchanged when the schema has no devices.maxItems.
 */
void TestAdapterData::configForWireReturnsAllDevicesWhenNoLimit()
{
    AdapterData data;

    QJsonArray devices;
    for (int i = 1; i <= 5; ++i)
    {
        QJsonObject dev;
        dev["id"] = i;
        devices.append(dev);
    }
    QJsonObject config;
    config["devices"] = devices;
    data.setCurrentConfig(config);
    data.setHasStoredConfig(true);

    QCOMPARE(data.configForWire().value("devices").toArray().size(), 5);
}

/*!
 * \brief configForWire does not truncate devices when the schema's maxItems is negative,
 * since a negative limit is treated as unbounded rather than causing the removal loop
 * to run without a valid stopping point.
 */
void TestAdapterData::configForWireDoesNotTruncateWhenMaxItemsNegative()
{
    AdapterData data;

    QJsonObject devicesSchema;
    devicesSchema["type"] = "array";
    devicesSchema["maxItems"] = -1;
    QJsonObject properties;
    properties["devices"] = devicesSchema;
    QJsonObject schema;
    schema["type"] = "object";
    schema["properties"] = properties;

    QJsonObject describeResult;
    describeResult["schema"] = schema;
    data.updateFromDescribe(describeResult);

    QJsonArray devices;
    for (int i = 1; i <= 5; ++i)
    {
        QJsonObject dev;
        dev["id"] = i;
        devices.append(dev);
    }
    QJsonObject config;
    config["devices"] = devices;
    data.setCurrentConfig(config);
    data.setHasStoredConfig(true);

    QCOMPARE(data.configForWire().value("devices").toArray().size(), 5);
}

/*!
 * \brief configForWire removes every device when the schema's maxItems is explicitly 0.
 */
void TestAdapterData::configForWireRemovesAllDevicesWhenMaxItemsZero()
{
    AdapterData data;

    QJsonObject devicesSchema;
    devicesSchema["type"] = "array";
    devicesSchema["maxItems"] = 0;
    QJsonObject properties;
    properties["devices"] = devicesSchema;
    QJsonObject schema;
    schema["type"] = "object";
    schema["properties"] = properties;

    QJsonObject describeResult;
    describeResult["schema"] = schema;
    data.updateFromDescribe(describeResult);

    QJsonArray devices;
    for (int i = 1; i <= 5; ++i)
    {
        QJsonObject dev;
        dev["id"] = i;
        devices.append(dev);
    }
    QJsonObject config;
    config["devices"] = devices;
    data.setCurrentConfig(config);
    data.setHasStoredConfig(true);

    QCOMPARE(data.configForWire().value("devices").toArray().size(), 0);
}

/*!
 * \brief configForWire caps the "devices" array to capabilities.maxDevices even when the
 * schema's maxItems is larger, reproducing the real Modbus adapter's license-aware limit.
 */
void TestAdapterData::configForWireCapsDevicesToCapabilitiesMaxDevices()
{
    AdapterData data;

    QJsonObject devicesSchema;
    devicesSchema["type"] = "array";
    devicesSchema["maxItems"] = 99;
    QJsonObject properties;
    properties["devices"] = devicesSchema;
    QJsonObject schema;
    schema["type"] = "object";
    schema["properties"] = properties;

    QJsonObject caps;
    caps["maxDevices"] = 2;

    QJsonObject describeResult;
    describeResult["schema"] = schema;
    describeResult["capabilities"] = caps;
    data.updateFromDescribe(describeResult);

    QJsonArray devices;
    for (int i = 1; i <= 5; ++i)
    {
        QJsonObject dev;
        dev["id"] = i;
        devices.append(dev);
    }
    QJsonObject config;
    config["devices"] = devices;
    data.setCurrentConfig(config);
    data.setHasStoredConfig(true);

    QCOMPARE(data.configForWire().value("devices").toArray().size(), 2);
}

void TestAdapterData::isMbcCompatibleTrue()
{
    AdapterData data;

    QJsonObject caps;
    caps["mbcCompatible"] = true;

    QJsonObject describeResult;
    describeResult["name"] = "modbusAdapter";
    describeResult["capabilities"] = caps;
    data.updateFromDescribe(describeResult);

    QVERIFY(data.isMbcCompatible());
}

void TestAdapterData::isMbcCompatibleFalse()
{
    AdapterData data;

    QJsonObject caps;
    caps["mbcCompatible"] = false;

    QJsonObject describeResult;
    describeResult["name"] = "testAdapter";
    describeResult["capabilities"] = caps;
    data.updateFromDescribe(describeResult);

    QVERIFY(!data.isMbcCompatible());
}

void TestAdapterData::settingsModelIsMbcCompatible()
{
    SettingsModel model;

    /* No adapters yet → not compatible */
    QVERIFY(!model.isMbcCompatible());

    /* Register an adapter without mbcCompatible */
    QJsonObject descNone;
    descNone["name"] = "noMbc";
    descNone["capabilities"] = QJsonObject{ { "mbcCompatible", false } };
    model.updateAdapterFromDescribe("none", descNone);
    QVERIFY(!model.isMbcCompatible());

    /* Add a second adapter with mbcCompatible: true */
    QJsonObject descMbc;
    descMbc["name"] = "withMbc";
    descMbc["capabilities"] = QJsonObject{ { "mbcCompatible", true } };
    model.updateAdapterFromDescribe("mbc", descMbc);
    QVERIFY(model.isMbcCompatible());
}

void TestAdapterData::licenseFieldParsed()
{
    AdapterData data;

    QJsonObject license;
    license["state"] = "valid";
    license["path"] = "/home/user/.config/ModbusScope/modbusAdapter/licenses/modbusAdapter.lic";
    license["product"] = "modbusAdapter";
    license["customer"] = "ACME Corp";
    license["email"] = "customer@example.com";
    license["licenseId"] = "LIC-2026-001";
    license["expires"] = "2027-01-01";

    QJsonObject describeResult;
    describeResult["name"] = "modbusAdapter";
    describeResult["license"] = license;

    data.updateFromDescribe(describeResult);

    QCOMPARE(data.license().value("state").toString(), QStringLiteral("valid"));
    QCOMPARE(data.license().value("customer").toString(), QStringLiteral("ACME Corp"));
    QCOMPARE(data.license().value("email").toString(), QStringLiteral("customer@example.com"));
    QCOMPARE(data.license().value("licenseId").toString(), QStringLiteral("LIC-2026-001"));
    QCOMPARE(data.license().value("expires").toString(), QStringLiteral("2027-01-01"));

    const AdapterLicenseInfo info = data.licenseInfo();
    QCOMPARE(info.state, AdapterLicenseInfo::State::Valid);
    QCOMPARE(info.customer, QStringLiteral("ACME Corp"));
    QCOMPARE(info.email, QStringLiteral("customer@example.com"));
    QCOMPARE(info.licenseId, QStringLiteral("LIC-2026-001"));
    QCOMPARE(info.expires, QStringLiteral("2027-01-01"));
}

void TestAdapterData::updateFromDescribeMissingLicense()
{
    AdapterData data;

    QJsonObject describeResult;
    describeResult["name"] = "minimalAdapter";

    data.updateFromDescribe(describeResult);

    QVERIFY(data.license().isEmpty());
}

/*!
 * \brief configForWire drops a repeated device id, keeping the first occurrence.
 *
 * A project file can carry the same device id twice (the legacy XML format synthesises ids from
 * <connectionid>, which defaults to 0 when absent), and the stored config keeps it verbatim.
 * Sending it on would tell the adapter subprocess about more devices than the model holds.
 */
void TestAdapterData::configForWireRemovesDuplicateDeviceIds()
{
    AdapterData data;

    QJsonObject describeResult;
    describeResult["schema"] = QJsonObject();
    data.updateFromDescribe(describeResult);

    QJsonArray devices;
    QJsonObject firstDev;
    firstDev["id"] = 1;
    firstDev["slaveId"] = 2;
    devices.append(firstDev);
    QJsonObject duplicateDev;
    duplicateDev["id"] = 1;
    duplicateDev["slaveId"] = 3;
    devices.append(duplicateDev);
    QJsonObject otherDev;
    otherDev["id"] = 2;
    devices.append(otherDev);

    QJsonObject config;
    config["devices"] = devices;
    data.setCurrentConfig(config);
    data.setHasStoredConfig(true);

    /* effectiveConfig() is the stored config and keeps the duplicate; only the wire config drops it. */
    QCOMPARE(data.effectiveConfig().value("devices").toArray().size(), 3);

    const QJsonArray wireDevices = data.configForWire().value("devices").toArray();
    QCOMPARE(wireDevices.size(), 2);
    QCOMPARE(wireDevices.at(0).toObject().value("id").toInt(), 1);
    QCOMPARE(wireDevices.at(0).toObject().value("slaveId").toInt(), 2);
    QCOMPARE(wireDevices.at(1).toObject().value("id").toInt(), 2);
}

/*!
 * \brief configForWire drops duplicates before capping, so a duplicate costs no device slot.
 *
 * Truncating first would let a repeated id push a distinct device past the adapter's limit and
 * out of the wire config.
 */
void TestAdapterData::configForWireDedupesBeforeTruncating()
{
    AdapterData data;

    QJsonObject devicesSchema;
    devicesSchema["type"] = "array";
    devicesSchema["maxItems"] = 2;
    QJsonObject properties;
    properties["devices"] = devicesSchema;
    QJsonObject schema;
    schema["type"] = "object";
    schema["properties"] = properties;

    QJsonObject describeResult;
    describeResult["schema"] = schema;
    data.updateFromDescribe(describeResult);

    QJsonArray devices;
    for (int id : { 1, 1, 2, 3 })
    {
        QJsonObject dev;
        dev["id"] = id;
        devices.append(dev);
    }

    QJsonObject config;
    config["devices"] = devices;
    data.setCurrentConfig(config);
    data.setHasStoredConfig(true);

    const QJsonArray wireDevices = data.configForWire().value("devices").toArray();
    QCOMPARE(wireDevices.size(), 2);
    QCOMPARE(wireDevices.at(0).toObject().value("id").toInt(), 1);
    QCOMPARE(wireDevices.at(1).toObject().value("id").toInt(), 2);
}

/*!
 * \brief configForWire leaves entries carrying no usable device id alone.
 *
 * Only a genuine repeat of a real id is dropped. Entries with no "id" key are not duplicates of
 * one another, so collapsing them would silently delete adapter device config.
 */
void TestAdapterData::configForWireKeepsDevicesWithoutId()
{
    AdapterData data;

    QJsonObject describeResult;
    describeResult["schema"] = QJsonObject();
    data.updateFromDescribe(describeResult);

    QJsonArray devices;
    QJsonObject firstNoId;
    firstNoId["slaveId"] = 7;
    devices.append(firstNoId);
    QJsonObject secondNoId;
    secondNoId["slaveId"] = 8;
    devices.append(secondNoId);
    QJsonObject realDev;
    realDev["id"] = 1;
    devices.append(realDev);
    QJsonObject duplicateDev;
    duplicateDev["id"] = 1;
    devices.append(duplicateDev);

    QJsonObject config;
    config["devices"] = devices;
    data.setCurrentConfig(config);
    data.setHasStoredConfig(true);

    const QJsonArray wireDevices = data.configForWire().value("devices").toArray();
    QCOMPARE(wireDevices.size(), 3);
    QCOMPARE(wireDevices.at(0).toObject().value("slaveId").toInt(), 7);
    QCOMPARE(wireDevices.at(1).toObject().value("slaveId").toInt(), 8);
    QCOMPARE(wireDevices.at(2).toObject().value("id").toInt(), 1);
}

QTEST_GUILESS_MAIN(TestAdapterData)
