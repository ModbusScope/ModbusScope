
#include "tst_projectfilehandler.h"

#include "../models/devicelisthelpers.h"
#include "importexport/projectfilehandler.h"
#include "legacy/projectfilexmltestdata.h"
#include "models/adapterdata.h"
#include "models/device.h"
#include "models/graphdatamodel.h"
#include "models/guimodel.h"
#include "models/settingsmodel.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>

namespace {

/*!
 * \brief Build an adapter describe object with schema.properties.devices.maxItems set.
 */
QJsonObject buildDescribeWithMaxItems(int maxItems)
{
    QJsonObject devicesSchema;
    devicesSchema["type"] = "array";
    devicesSchema["maxItems"] = maxItems;

    QJsonObject properties;
    properties["devices"] = devicesSchema;

    QJsonObject schema;
    schema["type"] = "object";
    schema["properties"] = properties;

    QJsonObject describe;
    describe["name"] = "testAdapter";
    describe["schema"] = schema;
    describe["defaults"] = QJsonObject();
    describe["capabilities"] = QJsonObject();
    return describe;
}

/*!
 * \brief Write a minimal MBS project JSON to a temp file and return the path.
 *
 * \param adapters  JSON array for the "adapters" key.
 * \param devices   JSON array for the "devices" key.
 */
QString writeTempProjectFile(const QJsonArray& adapters, const QJsonArray& devices)
{
    QJsonObject root;
    root["version"] = 6;
    root["adapters"] = adapters;
    root["devices"] = devices;
    root["log"] = QJsonObject();
    root["scope"] = QJsonArray();
    root["view"] = QJsonObject();

    QTemporaryFile* file = new QTemporaryFile();
    file->setAutoRemove(false);
    if (!file->open())
    {
        delete file;
        return QString();
    }

    file->write(QJsonDocument(root).toJson());
    QString path = file->fileName();
    file->close();
    delete file;
    return path;
}

//! Build a top-level "devices" entry for device \a id owned by adapter \a adapterType.
QJsonObject topLevelDevice(int id, const QString& adapterType)
{
    QJsonObject dev;
    dev["id"] = id;
    dev["adapter"] = QJsonObject{ { "type", adapterType } };
    return dev;
}

//! Build an "adapters" entry whose settings hold one device entry per ID in \a deviceIds.
QJsonObject adapterWithDevices(const QString& type, const QList<int>& deviceIds)
{
    QJsonArray devices;
    for (const int id : deviceIds)
    {
        devices.append(QJsonObject{ { "id", id }, { "slaveId", id * 10 } });
    }

    QJsonObject adapter;
    adapter["type"] = type;
    adapter["settings"] = QJsonObject{ { "devices", devices } };
    return adapter;
}

//! Return the device IDs in the stored config of \a adapterId, in order.
QList<int> storedDeviceIds(SettingsModel& settingsModel, const QString& adapterId)
{
    QList<int> ids;
    const QJsonArray devices = settingsModel.adapterData(adapterId)->currentConfig().value("devices").toArray();
    for (const auto& device : devices)
    {
        ids.append(device.toObject().value("id").toInt(-1));
    }
    return ids;
}

//! Write \a content to \a path; returns false when the file cannot be written.
bool writeTextFile(const QString& path, const QString& content)
{
    QFile file(path);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        return false;
    }
    file.write(content.toUtf8());
    return true;
}

} /* namespace */

void TestProjectFileHandler::init()
{
}

void TestProjectFileHandler::cleanup()
{
}

/*!
 * \brief Opening a project with one modbus device sets its adapterId to "modbus".
 */
void TestProjectFileHandler::applyDeviceSettingsAppliesAdapterId()
{
    QJsonArray adapters;
    QJsonObject adapter;
    adapter["type"] = "modbus";
    adapter["settings"] = QJsonObject();
    adapters.append(adapter);

    QJsonArray devices;
    QJsonObject dev;
    dev["id"] = 1;
    dev["adapterId"] = 0;
    QJsonObject adapterRef;
    adapterRef["type"] = "modbus";
    dev["adapter"] = adapterRef;
    devices.append(dev);

    const QString path = writeTempProjectFile(adapters, devices);
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    handler.openProjectFile(path);

    QCOMPARE(settingsModel.deviceSettings(1)->adapterId(), QStringLiteral("modbus"));

    QFile::remove(path);
}

/*!
 * \brief Opening a project with a named device applies the name to SettingsModel.
 */
void TestProjectFileHandler::applyDeviceSettingsAppliesName()
{
    QJsonArray adapters;
    QJsonObject adapter;
    adapter["type"] = "modbus";
    adapter["settings"] = QJsonObject();
    adapters.append(adapter);

    QJsonArray devices;
    QJsonObject dev;
    dev["id"] = 1;
    dev["name"] = "Pump";
    dev["adapterId"] = 0;
    QJsonObject adapterRef;
    adapterRef["type"] = "modbus";
    dev["adapter"] = adapterRef;
    devices.append(dev);

    const QString path = writeTempProjectFile(adapters, devices);
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    handler.openProjectFile(path);

    QCOMPARE(settingsModel.deviceSettings(1)->name(), QStringLiteral("Pump"));

    QFile::remove(path);
}

/*!
 * \brief The device name is applied before the model announces the new device.
 */
void TestProjectFileHandler::applyDeviceSettingsAppliesNameBeforeNotifying()
{
    QJsonArray adapters;
    QJsonObject adapter;
    adapter["type"] = "modbus";
    adapter["settings"] = QJsonObject();
    adapters.append(adapter);

    QJsonArray devices;
    QJsonObject dev;
    dev["id"] = 1;
    dev["name"] = "Pump";
    dev["adapterId"] = 0;
    QJsonObject adapterRef;
    adapterRef["type"] = "modbus";
    dev["adapter"] = adapterRef;
    devices.append(dev);

    const QString path = writeTempProjectFile(adapters, devices);
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    DeviceNameProbe probe(&settingsModel, 1);

    handler.openProjectFile(path);

    /* Regression guard: the device's real name must be in place before deviceListChanged()
     * is emitted, or every observer of the load sees the placeholder name Device's
     * constructor assigns instead. */
    QVERIFY(!probe.names.isEmpty());
    QCOMPARE(probe.names.first(), QStringLiteral("Pump"));

    QFile::remove(path);
}

/*!
 * \brief Devices referencing different adapters each get the correct adapterId string.
 */
void TestProjectFileHandler::applyDeviceSettingsMultipleAdapters()
{
    QJsonArray adapters;
    QJsonObject adapterModbus;
    adapterModbus["type"] = "modbus";
    adapterModbus["settings"] = QJsonObject();
    adapters.append(adapterModbus);

    QJsonObject adapterCustom;
    adapterCustom["type"] = "custom";
    adapterCustom["settings"] = QJsonObject();
    adapters.append(adapterCustom);

    QJsonArray devices;

    QJsonObject dev1;
    dev1["id"] = 1;
    dev1["adapterId"] = 0;
    QJsonObject ref1;
    ref1["type"] = "modbus";
    dev1["adapter"] = ref1;
    devices.append(dev1);

    QJsonObject dev2;
    dev2["id"] = 2;
    dev2["adapterId"] = 1;
    QJsonObject ref2;
    ref2["type"] = "custom";
    dev2["adapter"] = ref2;
    devices.append(dev2);

    const QString path = writeTempProjectFile(adapters, devices);
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    handler.openProjectFile(path);

    QCOMPARE(settingsModel.deviceSettings(1)->adapterId(), QStringLiteral("modbus"));
    QCOMPARE(settingsModel.deviceSettings(2)->adapterId(), QStringLiteral("custom"));

    QFile::remove(path);
}

/*!
 * \brief A device entry without a "name" keeps Device's constructor default.
 */
void TestProjectFileHandler::applyDeviceSettingsWithoutNameKeepsDefault()
{
    QJsonArray adapters;
    QJsonObject adapter;
    adapter["type"] = "modbus";
    adapter["settings"] = QJsonObject();
    adapters.append(adapter);

    QJsonArray devices;
    QJsonObject dev;
    dev["id"] = 4;
    dev["adapterId"] = 0;
    QJsonObject ref;
    ref["type"] = "modbus";
    dev["adapter"] = ref;
    devices.append(dev);

    const QString path = writeTempProjectFile(adapters, devices);
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    handler.openProjectFile(path);

    QVERIFY(settingsModel.hasDevice(4));
    QCOMPARE(settingsModel.deviceSettings(4)->name(), QStringLiteral("Device 4"));

    QFile::remove(path);
}

/*!
 * \brief A project load reports its device list as one change, not one per device.
 *
 * SettingsModel::applyDeviceList() replaces the whole list at once, so observers of
 * deviceListChanged() (expression validation, the add-register device combo) never see the
 * intermediate states of a device-by-device rebuild.
 */
void TestProjectFileHandler::applyDeviceSettingsEmitsDeviceListChangedOnce()
{
    QJsonArray adapters;
    QJsonObject adapter;
    adapter["type"] = "modbus";
    adapter["settings"] = QJsonObject();
    adapters.append(adapter);

    QJsonArray devices;
    for (int id = 1; id <= 3; ++id)
    {
        QJsonObject dev;
        dev["id"] = id;
        dev["adapterId"] = 0;
        QJsonObject ref;
        ref["type"] = "modbus";
        dev["adapter"] = ref;
        devices.append(dev);
    }

    const QString path = writeTempProjectFile(adapters, devices);
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    QSignalSpy spy(&settingsModel, &SettingsModel::deviceListChanged);
    handler.openProjectFile(path);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(settingsModel.deviceList().size(), 3);

    QFile::remove(path);
}

/*!
 * \brief Loading a project with an empty devices section clears all devices, including stale ones.
 *
 * Regression: applyDeviceSettings used to return early before clearing the device list when
 * the new list was empty, leaving previously-loaded devices in the model.
 */
void TestProjectFileHandler::applyDeviceSettingsEmptyListClearsModel()
{
    const QString path = writeTempProjectFile(QJsonArray(), QJsonArray());
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);

    /* Pre-populate a stale device that should be cleared */
    DeviceListHelpers::seedDevice(&settingsModel, 99);
    QVERIFY(settingsModel.deviceList().contains(99));

    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);
    handler.openProjectFile(path);

    QVERIFY(settingsModel.deviceList().isEmpty());

    QFile::remove(path);
}

/*!
 * \brief Devices not in the project file are removed from SettingsModel on load.
 */
void TestProjectFileHandler::applyDeviceSettingsClearsPreviousDevices()
{
    QJsonArray adapters;
    QJsonObject adapter;
    adapter["type"] = "modbus";
    adapter["settings"] = QJsonObject();
    adapters.append(adapter);

    QJsonArray devices;

    QJsonObject dev1;
    dev1["id"] = 1;
    dev1["adapterId"] = 0;
    QJsonObject ref1;
    ref1["type"] = "modbus";
    dev1["adapter"] = ref1;
    devices.append(dev1);

    QJsonObject dev2;
    dev2["id"] = 2;
    dev2["adapterId"] = 0;
    QJsonObject ref2;
    ref2["type"] = "modbus";
    dev2["adapter"] = ref2;
    devices.append(dev2);

    const QString path = writeTempProjectFile(adapters, devices);
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);

    /* Pre-populate a device that should be cleared on load */
    DeviceListHelpers::seedDevice(&settingsModel, 3);

    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);
    handler.openProjectFile(path);

    const QList<deviceId_t> list = settingsModel.deviceList();
    QCOMPARE(list.size(), 2);
    QVERIFY(list.contains(1));
    QVERIFY(list.contains(2));
    QVERIFY(!list.contains(3));

    QFile::remove(path);
}

/*!
 * \brief A repeated device ID whose later entry omits the name keeps the earlier entry's name.
 *
 * A missing "name" key means the entry does not specify one, not that the name should be reset
 * to Device's placeholder. The user would otherwise see "Device 1" in place of the name their
 * project file gave the device.
 */
void TestProjectFileHandler::duplicateDeviceIdKeepsNameFromEarlierEntry()
{
    QJsonArray adapters;
    QJsonObject adapter;
    adapter["type"] = "modbus";
    adapter["settings"] = QJsonObject();
    adapters.append(adapter);

    QJsonObject adapterRef;
    adapterRef["type"] = "modbus";

    QJsonArray devices;

    QJsonObject dev1;
    dev1["id"] = 1;
    dev1["name"] = "Boiler";
    dev1["adapterId"] = 0;
    dev1["adapter"] = adapterRef;
    devices.append(dev1);

    QJsonObject dev2;
    dev2["id"] = 1;
    dev2["adapterId"] = 0;
    dev2["adapter"] = adapterRef;
    devices.append(dev2);

    const QString path = writeTempProjectFile(adapters, devices);
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    handler.openProjectFile(path);

    QCOMPARE(settingsModel.deviceList().size(), 1);
    QCOMPARE(settingsModel.deviceSettings(1)->name(), QStringLiteral("Boiler"));

    QFile::remove(path);
}

/*!
 * \brief A repeated device ID takes every field the later entry actually specifies.
 *
 * The merge must not degenerate into "first entry wins": a field present in a later entry
 * overrides the earlier one, in file order.
 */
void TestProjectFileHandler::duplicateDeviceIdLastSpecifiedFieldWins()
{
    QJsonArray adapters;
    QJsonObject modbusAdapter;
    modbusAdapter["type"] = "modbus";
    modbusAdapter["settings"] = QJsonObject();
    adapters.append(modbusAdapter);
    QJsonObject dummyAdapter;
    dummyAdapter["type"] = "dummy";
    dummyAdapter["settings"] = QJsonObject();
    adapters.append(dummyAdapter);

    QJsonArray devices;

    QJsonObject dev1;
    dev1["id"] = 1;
    dev1["name"] = "Boiler";
    dev1["adapterId"] = 0;
    QJsonObject modbusRef;
    modbusRef["type"] = "modbus";
    dev1["adapter"] = modbusRef;
    devices.append(dev1);

    QJsonObject dev2;
    dev2["id"] = 1;
    dev2["name"] = "Pump";
    dev2["adapterId"] = 1;
    QJsonObject dummyRef;
    dummyRef["type"] = "dummy";
    dev2["adapter"] = dummyRef;
    devices.append(dev2);

    const QString path = writeTempProjectFile(adapters, devices);
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    handler.openProjectFile(path);

    QCOMPARE(settingsModel.deviceList().size(), 1);
    QCOMPARE(settingsModel.deviceSettings(1)->name(), QStringLiteral("Pump"));
    QCOMPARE(settingsModel.deviceSettings(1)->adapterId(), QStringLiteral("dummy"));

    QFile::remove(path);
}

/*!
 * \brief A repeated device ID whose later entry omits the adapter keeps the earlier adapter.
 *
 * An absent adapter type means the entry does not specify one; falling back to "modbus" here
 * would hand the device to an adapter no entry in the file ever named.
 */
void TestProjectFileHandler::duplicateDeviceIdKeepsAdapterFromEarlierEntry()
{
    QJsonArray adapters;
    QJsonObject modbusAdapter;
    modbusAdapter["type"] = "modbus";
    modbusAdapter["settings"] = QJsonObject();
    adapters.append(modbusAdapter);
    QJsonObject dummyAdapter;
    dummyAdapter["type"] = "dummy";
    dummyAdapter["settings"] = QJsonObject();
    adapters.append(dummyAdapter);

    QJsonArray devices;

    QJsonObject dev1;
    dev1["id"] = 1;
    dev1["name"] = "Boiler";
    dev1["adapterId"] = 1;
    QJsonObject dummyRef;
    dummyRef["type"] = "dummy";
    dev1["adapter"] = dummyRef;
    devices.append(dev1);

    QJsonObject dev2;
    dev2["id"] = 1;
    dev2["name"] = "Pump";
    devices.append(dev2);

    const QString path = writeTempProjectFile(adapters, devices);
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    handler.openProjectFile(path);

    QCOMPARE(settingsModel.deviceList().size(), 1);
    QCOMPARE(settingsModel.deviceSettings(1)->name(), QStringLiteral("Pump"));
    QCOMPARE(settingsModel.deviceSettings(1)->adapterId(), QStringLiteral("dummy"));

    QFile::remove(path);
}

/*!
 * \brief A duplicate device ID is logged, and the project file still loads.
 *
 * The legacy XML format synthesises device IDs from <connectionid>, which defaults to 0 when
 * absent, so a file that has always opened can carry a duplicate. Reporting it through
 * GeneralError would abort the parse and refuse such a file, so it is a warning instead.
 */
void TestProjectFileHandler::duplicateDeviceIdIsReportedWithoutRefusingTheFile()
{
    QJsonArray adapters;
    QJsonObject adapter;
    adapter["type"] = "modbus";
    adapter["settings"] = QJsonObject();
    adapters.append(adapter);

    QJsonObject adapterRef;
    adapterRef["type"] = "modbus";

    QJsonArray devices;

    QJsonObject dev1;
    dev1["id"] = 1;
    dev1["name"] = "Boiler";
    dev1["adapterId"] = 0;
    dev1["adapter"] = adapterRef;
    devices.append(dev1);

    QJsonObject dev2;
    dev2["id"] = 1;
    dev2["adapterId"] = 0;
    dev2["adapter"] = adapterRef;
    devices.append(dev2);

    const QString path = writeTempProjectFile(adapters, devices);
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    /* ignoreMessage fails the test when the message never arrives, so this is the assertion
       that the duplicate is reported at all. */
    QTest::ignoreMessage(QtWarningMsg, "ProjectFileHandler: duplicate device id 1 in project file - "
                                       "later entries override the fields they specify");

    handler.openProjectFile(path);

    QVERIFY(settingsModel.hasDevice(1));

    /* Only the success branch sets the project file path, so this proves the load was not
       aborted the way a GeneralError would have aborted it. */
    QCOMPARE(guiModel.projectFilePath(), path);

    QFile::remove(path);
}

/*!
 * \brief Loading a project file applies adapter settings to SettingsModel.
 */
void TestProjectFileHandler::loadSetsAdapterConfigInSettingsModel()
{
    QJsonObject connections;
    connections["ip"] = "192.168.1.1";
    connections["port"] = 502;
    QJsonArray connectionsArray;
    connectionsArray.append(connections);

    QJsonObject adapterSettings;
    adapterSettings["connections"] = connectionsArray;

    QJsonArray adapters;
    QJsonObject adapter;
    adapter["type"] = "modbus";
    adapter["settings"] = adapterSettings;
    adapters.append(adapter);

    const QString path = writeTempProjectFile(adapters, QJsonArray());
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    handler.openProjectFile(path);

    const AdapterData* pAdapter = settingsModel.adapterData("modbus");
    QVERIFY(pAdapter->hasStoredConfig());
    QCOMPARE(pAdapter->currentConfig()["connections"].toArray(), connectionsArray);

    QFile::remove(path);
}

/*!
 * \brief After a simulated dialog accept (setAdapterCurrentConfig), saveProjectFile writes the
 * updated adapter config to the file.
 */
void TestProjectFileHandler::savePreservesAdapterConfigAfterDialogAccept()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString savePath = tmpDir.filePath("test.mbs");

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    QJsonObject connection;
    connection["id"] = 1;
    connection["type"] = "tcp";
    connection["ip"] = "10.0.0.1";
    connection["port"] = 502;
    QJsonArray connectionsArray;
    connectionsArray.append(connection);

    QJsonObject config;
    config["connections"] = connectionsArray;

    settingsModel.setAdapterCurrentConfig("modbus", config);
    guiModel.setProjectFilePath(savePath);
    handler.saveProjectFile();

    QFile file(savePath);
    QVERIFY(file.open(QFile::ReadOnly | QFile::Text));
    const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();

    const QJsonArray savedAdapters = root["adapters"].toArray();
    QCOMPARE(savedAdapters.size(), 1);

    const QJsonObject savedAdapter = savedAdapters.first().toObject();
    QCOMPARE(savedAdapter["type"].toString(), QStringLiteral("modbus"));
    QCOMPARE(savedAdapter["settings"].toObject()["connections"].toArray(), connectionsArray);
}

/*!
 * \brief Round-trip: load a project file, save it to a new path, reload — adapter config is
 * preserved unchanged.
 */
void TestProjectFileHandler::roundTripPreservesAdapterConfig()
{
    QJsonObject connection;
    connection["id"] = 1;
    connection["type"] = "tcp";
    connection["ip"] = "172.16.0.5";
    connection["port"] = 1502;
    QJsonArray connectionsArray;
    connectionsArray.append(connection);

    QJsonObject adapterSettings;
    adapterSettings["connections"] = connectionsArray;

    QJsonArray adapters;
    QJsonObject adapter;
    adapter["type"] = "modbus";
    adapter["settings"] = adapterSettings;
    adapters.append(adapter);

    QJsonArray devices;
    QJsonObject dev;
    dev["id"] = 1;
    dev["name"] = "Sensor";
    dev["adapterId"] = 0;
    QJsonObject adapterRef;
    adapterRef["type"] = "modbus";
    dev["adapter"] = adapterRef;
    devices.append(dev);

    const QString loadPath = writeTempProjectFile(adapters, devices);
    QVERIFY(!loadPath.isEmpty());

    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString savePath = tmpDir.filePath("roundtrip.mbs");

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    handler.openProjectFile(loadPath);
    guiModel.setProjectFilePath(savePath);
    handler.saveProjectFile();

    QFile file(savePath);
    QVERIFY(file.open(QFile::ReadOnly | QFile::Text));
    const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();

    const QJsonArray savedAdapters = root["adapters"].toArray();
    QCOMPARE(savedAdapters.size(), 1);
    QCOMPARE(savedAdapters.first().toObject()["settings"].toObject()["connections"].toArray(), connectionsArray);

    const QJsonArray savedDevices = root["devices"].toArray();
    QCOMPARE(savedDevices.size(), 1);
    QCOMPARE(savedDevices.first().toObject()["id"].toInt(), 1);
    QCOMPARE(savedDevices.first().toObject()["name"].toString(), QStringLiteral("Sensor"));

    QFile::remove(loadPath);
}

/*!
 * \brief Adapter settings with more devices than maxItems are still all stored on load; only
 * AdapterData::configForWire() (used when sending to the adapter subprocess) caps the count.
 * Truncating here as well would desync currentConfig() from the generic device list SettingsModel
 * builds from the same project file's top-level "devices" section, causing AdapterDeviceSettings
 * to silently delete the "missing" devices the next time it's opened (see
 * openingProjectWithExcessDevicesKeepsAllDevicesInDialog below).
 */
void TestProjectFileHandler::applyAdapterSettingsDoesNotTruncateDevicesExceedingMaxItems()
{
    QJsonArray adapterDevices;
    for (int i = 1; i <= 5; ++i)
    {
        QJsonObject dev;
        dev["id"] = i;
        adapterDevices.append(dev);
    }

    QJsonObject adapterSettings;
    adapterSettings["devices"] = adapterDevices;

    QJsonArray adapters;
    QJsonObject adapter;
    adapter["type"] = "modbus";
    adapter["settings"] = adapterSettings;
    adapters.append(adapter);

    const QString path = writeTempProjectFile(adapters, QJsonArray());
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    settingsModel.updateAdapterFromDescribe("modbus", buildDescribeWithMaxItems(2));
    handler.openProjectFile(path);

    const QJsonArray storedDevices = settingsModel.adapterData("modbus")->currentConfig().value("devices").toArray();
    QCOMPARE(storedDevices.size(), 5);

    QFile::remove(path);
}

/*!
 * \brief Regression test: opening a project file with more devices for an adapter than its
 * current maxItems allows must leave the adapter's stored config and SettingsModel's generic
 * device list in agreement. Previously, applyAdapterSettings() truncated the adapter's own
 * stored "devices" array to maxItems while applyDeviceSettings() loaded every device into the
 * generic device list — the two disagreed, which (see
 * TestAdapterDeviceSettings::doesNotDropDevicesWhenAdapterConfigMatchesDeviceList in
 * tst_adapterdevicesettings.cpp, a GUI test) made AdapterDeviceSettings's constructor silently
 * delete any device present in the generic list but absent from the (truncated) adapter config.
 */
void TestProjectFileHandler::openingProjectWithExcessDevicesKeepsAdapterConfigAndDeviceListConsistent()
{
    QJsonArray adapterDevices;
    QJsonArray topLevelDevices;
    for (int i = 1; i <= 5; ++i)
    {
        QJsonObject dev;
        dev["id"] = i;
        adapterDevices.append(dev);

        QJsonObject topDev;
        topDev["id"] = i;
        QJsonObject adapterRef;
        adapterRef["type"] = "modbus";
        topDev["adapter"] = adapterRef;
        topLevelDevices.append(topDev);
    }

    QJsonObject adapterSettings;
    adapterSettings["devices"] = adapterDevices;

    QJsonArray adapters;
    QJsonObject adapter;
    adapter["type"] = "modbus";
    adapter["settings"] = adapterSettings;
    adapters.append(adapter);

    const QString path = writeTempProjectFile(adapters, topLevelDevices);
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    settingsModel.updateAdapterFromDescribe("modbus", buildDescribeWithMaxItems(2));
    handler.openProjectFile(path);

    const QJsonArray storedDevices = settingsModel.adapterData("modbus")->currentConfig().value("devices").toArray();
    QCOMPARE(storedDevices.size(), 5);
    QCOMPARE(settingsModel.deviceList().size(), 5);

    QFile::remove(path);
}

/*!
 * \brief Generic devices exceeding the adapter schema maxItems are still loaded (no silent
 *        drop); the limit is surfaced as a UI indication rather than enforced here.
 */
void TestProjectFileHandler::applyDeviceSettingsDoesNotTruncateDevicesExceedingMaxItems()
{
    QJsonArray adapters;
    QJsonObject adapter;
    adapter["type"] = "modbus";
    adapter["settings"] = QJsonObject();
    adapters.append(adapter);

    QJsonArray devices;
    for (int i = 1; i <= 5; ++i)
    {
        QJsonObject dev;
        dev["id"] = i;
        QJsonObject adapterRef;
        adapterRef["type"] = "modbus";
        dev["adapter"] = adapterRef;
        devices.append(dev);
    }

    const QString path = writeTempProjectFile(adapters, devices);
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    settingsModel.updateAdapterFromDescribe("modbus", buildDescribeWithMaxItems(2));
    handler.openProjectFile(path);

    QCOMPARE(settingsModel.deviceList().size(), 5);

    QFile::remove(path);
}

/*!
 * \brief A low maxItems on one adapter must not cap devices belonging to a different adapter.
 */
void TestProjectFileHandler::applyDeviceSettingsNotCappedByOtherAdaptersLowerLimit()
{
    QJsonArray adapters;
    QJsonObject modbusAdapter;
    modbusAdapter["type"] = "modbus";
    modbusAdapter["settings"] = QJsonObject();
    adapters.append(modbusAdapter);
    QJsonObject dummyAdapter;
    dummyAdapter["type"] = "dummy";
    dummyAdapter["settings"] = QJsonObject();
    adapters.append(dummyAdapter);

    QJsonArray devices;
    QJsonObject dummyDevice;
    dummyDevice["id"] = 1;
    QJsonObject dummyRef;
    dummyRef["type"] = "dummy";
    dummyDevice["adapter"] = dummyRef;
    devices.append(dummyDevice);

    for (int i = 2; i <= 5; ++i)
    {
        QJsonObject dev;
        dev["id"] = i;
        QJsonObject adapterRef;
        adapterRef["type"] = "modbus";
        dev["adapter"] = adapterRef;
        devices.append(dev);
    }

    const QString path = writeTempProjectFile(adapters, devices);
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    // Dummy allows only 1 device; modbus allows plenty. Dummy's low limit must not
    // truncate the 4 modbus devices.
    settingsModel.updateAdapterFromDescribe("dummy", buildDescribeWithMaxItems(1));
    settingsModel.updateAdapterFromDescribe("modbus", buildDescribeWithMaxItems(99));
    handler.openProjectFile(path);

    QCOMPARE(settingsModel.deviceList().size(), 5);

    QFile::remove(path);
}

/*!
 * \brief All devices are loaded when no adapter schema is present (no maxItems limit).
 */
void TestProjectFileHandler::applyDeviceSettingsNoLimitWhenSchemaAbsent()
{
    QJsonArray adapters;
    QJsonObject adapter;
    adapter["type"] = "modbus";
    adapter["settings"] = QJsonObject();
    adapters.append(adapter);

    QJsonArray devices;
    for (int i = 1; i <= 5; ++i)
    {
        QJsonObject dev;
        dev["id"] = i;
        QJsonObject adapterRef;
        adapterRef["type"] = "modbus";
        dev["adapter"] = adapterRef;
        devices.append(dev);
    }

    const QString path = writeTempProjectFile(adapters, devices);
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    /* No describe call — adapter schema is empty, no limit applies */
    handler.openProjectFile(path);

    QCOMPARE(settingsModel.deviceList().size(), 5);

    QFile::remove(path);
}

/*!
 * \brief A device the adapter config lists but the devices section omits is added to the device list.
 *
 * Without this the device only appears once the adapter describes, so an expression using it fails
 * validation with "no such device" in the meantime.
 */
void TestProjectFileHandler::deviceOnlyInAdapterConfigIsAddedToDeviceList()
{
    const QJsonArray adapters{ adapterWithDevices("modbus", { 1, 2 }) };
    const QJsonArray devices{ topLevelDevice(1, "modbus") };

    const QString path = writeTempProjectFile(adapters, devices);
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    handler.openProjectFile(path);

    QCOMPARE(settingsModel.deviceList(), QList<deviceId_t>({ 1, 2 }));
    QCOMPARE(settingsModel.adapterIdForDevice(2), QStringLiteral("modbus"));
    QCOMPARE(settingsModel.deviceSettings(2)->name(), QStringLiteral("Device 2"));
    QCOMPARE(storedDeviceIds(settingsModel, "modbus"), QList<int>({ 1, 2 }));

    QFile::remove(path);
}

/*!
 * \brief A device the devices section lists but its adapter config omits is added to that config.
 *
 * Without this the device shows in the Settings dialog but is never polled.
 */
void TestProjectFileHandler::deviceOnlyInDevicesSectionIsAddedToAdapterConfig()
{
    const QJsonArray adapters{ adapterWithDevices("modbus", { 1 }) };
    QJsonObject namedDevice = topLevelDevice(2, "modbus");
    namedDevice["name"] = "Boiler";
    const QJsonArray devices{ topLevelDevice(1, "modbus"), namedDevice };

    const QString path = writeTempProjectFile(adapters, devices);
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    handler.openProjectFile(path);

    QCOMPARE(settingsModel.deviceList(), QList<deviceId_t>({ 1, 2 }));
    QCOMPARE(settingsModel.deviceSettings(2)->name(), QStringLiteral("Boiler"));
    QCOMPARE(storedDeviceIds(settingsModel, "modbus"), QList<int>({ 1, 2 }));

    /* The existing entry is kept as the file gave it */
    const QJsonArray stored = settingsModel.adapterData("modbus")->currentConfig().value("devices").toArray();
    QCOMPARE(stored.at(0).toObject().value("slaveId").toInt(), 10);

    QFile::remove(path);
}

/*!
 * \brief A device whose owner's config omits it, while another adapter's config lists it, moves to that adapter.
 *
 * The adapter config is what gets polled, so it decides ownership rather than the devices section.
 */
void TestProjectFileHandler::deviceListedByOtherAdapterMovesOwnership()
{
    const QJsonArray adapters{ adapterWithDevices("modbus", { 1 }), adapterWithDevices("custom", { 2 }) };
    const QJsonArray devices{ topLevelDevice(1, "modbus"), topLevelDevice(2, "modbus") };

    const QString path = writeTempProjectFile(adapters, devices);
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    handler.openProjectFile(path);

    QCOMPARE(settingsModel.adapterIdForDevice(2), QStringLiteral("custom"));
    QCOMPARE(storedDeviceIds(settingsModel, "modbus"), QList<int>({ 1 }));
    QCOMPARE(storedDeviceIds(settingsModel, "custom"), QList<int>({ 2 }));

    QFile::remove(path);
}

/*!
 * \brief An adapter config without a "devices" key is not cross-checked against the devices section.
 *
 * Such a config takes its devices from the adapter defaults, which may not be known yet at load.
 */
void TestProjectFileHandler::adapterConfigWithoutDevicesKeyIsNotCrossChecked()
{
    QJsonObject adapter;
    adapter["type"] = "modbus";
    adapter["settings"] = QJsonObject();
    const QJsonArray adapters{ adapter };
    const QJsonArray devices{ topLevelDevice(3, "modbus") };

    const QString path = writeTempProjectFile(adapters, devices);
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    handler.openProjectFile(path);

    QCOMPARE(settingsModel.deviceList(), QList<deviceId_t>({ 3 }));
    QVERIFY(!settingsModel.adapterData("modbus")->currentConfig().contains("devices"));

    QFile::remove(path);
}

/*!
 * \brief An adapter type listed twice is aligned against its last entry, the one the model keeps.
 */
void TestProjectFileHandler::duplicateAdapterTypeAlignsAgainstLastEntry()
{
    const QJsonArray adapters{ adapterWithDevices("modbus", { 1, 2 }), adapterWithDevices("modbus", { 1 }) };
    const QJsonArray devices{ topLevelDevice(1, "modbus"), topLevelDevice(3, "modbus") };

    const QString path = writeTempProjectFile(adapters, devices);
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    handler.openProjectFile(path);

    /* Device 2 is only in the overridden first entry, so it is not added */
    QCOMPARE(settingsModel.deviceList(), QList<deviceId_t>({ 1, 3 }));
    QCOMPARE(storedDeviceIds(settingsModel, "modbus"), QList<int>({ 1, 3 }));

    QFile::remove(path);
}

/*!
 * \brief A device listed by several adapter configs goes to the first of them in alphabetical order.
 */
void TestProjectFileHandler::deviceListedByTwoAdaptersGoesToFirstAlphabetically()
{
    const QJsonArray adapters{ adapterWithDevices("zeta", { 2 }), adapterWithDevices("alpha", { 2 }) };
    const QJsonArray devices{ topLevelDevice(1, "modbus") };

    const QString path = writeTempProjectFile(adapters, devices);
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    handler.openProjectFile(path);

    QCOMPARE(settingsModel.adapterIdForDevice(2), QStringLiteral("alpha"));

    QFile::remove(path);
}

/*!
 * \brief A device whose owner adapter is not in the file is left as the devices section gave it.
 */
void TestProjectFileHandler::deviceOfAdapterAbsentFromFileIsUntouched()
{
    const QJsonArray adapters{ adapterWithDevices("modbus", { 1 }) };
    const QJsonArray devices{ topLevelDevice(1, "modbus"), topLevelDevice(5, "custom") };

    const QString path = writeTempProjectFile(adapters, devices);
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    handler.openProjectFile(path);

    QCOMPARE(settingsModel.deviceList(), QList<deviceId_t>({ 1, 5 }));
    QCOMPARE(settingsModel.adapterIdForDevice(5), QStringLiteral("custom"));
    QCOMPARE(storedDeviceIds(settingsModel, "modbus"), QList<int>({ 1 }));
    QVERIFY(!settingsModel.adapterIds().contains("custom"));

    QFile::remove(path);
}

/*!
 * \brief Aligning the two device sections still reports the device list as a single change.
 */
void TestProjectFileHandler::alignedLoadEmitsDeviceListChangedOnce()
{
    const QJsonArray adapters{ adapterWithDevices("modbus", { 1, 2 }) };
    const QJsonArray devices{ topLevelDevice(1, "modbus"), topLevelDevice(3, "modbus") };

    const QString path = writeTempProjectFile(adapters, devices);
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    QSignalSpy spy(&settingsModel, &SettingsModel::deviceListChanged);
    handler.openProjectFile(path);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(settingsModel.deviceList(), QList<deviceId_t>({ 1, 2, 3 }));

    QFile::remove(path);
}

/*!
 * \brief Saving a project loaded with mismatched device sections writes sections that agree.
 */
void TestProjectFileHandler::roundTripOfMismatchedDevicesSavesAlignedSections()
{
    const QJsonArray adapters{ adapterWithDevices("modbus", { 1, 2 }) };
    const QJsonArray devices{ topLevelDevice(1, "modbus"), topLevelDevice(3, "modbus") };

    const QString loadPath = writeTempProjectFile(adapters, devices);
    QVERIFY(!loadPath.isEmpty());

    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString savePath = tmpDir.filePath("aligned.mbs");

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    handler.openProjectFile(loadPath);
    guiModel.setProjectFilePath(savePath);
    handler.saveProjectFile();

    QFile file(savePath);
    QVERIFY(file.open(QFile::ReadOnly | QFile::Text));
    const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();

    QList<int> adapterDeviceIds;
    const QJsonObject savedAdapter = root.value("adapters").toArray().first().toObject();
    const QJsonArray savedAdapterDevices = savedAdapter.value("settings").toObject().value("devices").toArray();
    for (const auto& device : savedAdapterDevices)
    {
        adapterDeviceIds.append(device.toObject().value("id").toInt(-1));
    }

    QList<int> topLevelDeviceIds;
    const QJsonArray savedDevices = root["devices"].toArray();
    for (const auto& device : savedDevices)
    {
        topLevelDeviceIds.append(device.toObject().value("id").toInt(-1));
    }

    QCOMPARE(adapterDeviceIds, QList<int>({ 1, 2, 3 }));
    QCOMPARE(topLevelDeviceIds, QList<int>({ 1, 2, 3 }));

    QFile::remove(loadPath);
}

/*!
 * \brief Opening a legacy XML project stores a config in the Modbus adapter's own shape.
 */
void TestProjectFileHandler::openXmlProjectStoresAdapterShapedConfig()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString path = tmpDir.filePath("legacy.mbs");
    QVERIFY(writeTextFile(path, ProjectFileXmlTestData::cDataLevel5Demo));

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    handler.openProjectFile(path);

    QVERIFY(settingsModel.adapterData("modbus") != nullptr);
    const QJsonObject config = settingsModel.adapterData("modbus")->currentConfig();

    const QJsonArray connections = config.value("connections").toArray();
    QCOMPARE(connections.size(), 2);
    QCOMPARE(connections[0].toObject().value("id").toInt(), 1);
    QCOMPARE(connections[0].toObject().value("type").toString(), QStringLiteral("tcp"));
    QCOMPARE(connections[1].toObject().value("id").toInt(), 2);

    const QJsonArray devices = config.value("devices").toArray();
    QCOMPARE(devices.size(), 2);
    QCOMPARE(devices[0].toObject().value("connectionId").toInt(), 1);
    QCOMPARE(devices[1].toObject().value("connectionId").toInt(), 2);
    QVERIFY(!devices[0].toObject().contains("connectionid"));

    QCOMPARE(storedDeviceIds(settingsModel, "modbus"), QList<int>({ 1, 2 }));
}

/*!
 * \brief A legacy XML project saved as JSON and reopened keeps the same connections and devices.
 */
void TestProjectFileHandler::xmlProjectSavedAsJsonRoundTrips()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString xmlPath = tmpDir.filePath("legacy.mbs");
    const QString jsonPath = tmpDir.filePath("converted.mbs");
    QVERIFY(writeTextFile(xmlPath, ProjectFileXmlTestData::cDataLevel4Demo));

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    handler.openProjectFile(xmlPath);
    const QJsonObject loadedConfig = settingsModel.adapterData("modbus")->currentConfig();
    guiModel.setProjectFilePath(jsonPath);
    handler.saveProjectFile();

    GuiModel reloadGuiModel;
    SettingsModel reloadSettingsModel;
    GraphDataModel reloadGraphDataModel(&reloadSettingsModel);
    ProjectFileHandler reloadHandler(&reloadGuiModel, &reloadSettingsModel, &reloadGraphDataModel);

    reloadHandler.openProjectFile(jsonPath);
    const QJsonObject reloadedConfig = reloadSettingsModel.adapterData("modbus")->currentConfig();

    QCOMPARE(reloadedConfig.value("connections").toArray(), loadedConfig.value("connections").toArray());
    QCOMPARE(reloadedConfig.value("devices").toArray(), loadedConfig.value("devices").toArray());
    QCOMPARE(reloadedConfig.value("connections").toArray().size(), 2);
    QCOMPARE(storedDeviceIds(reloadSettingsModel, "modbus"), QList<int>({ 1, 2 }));
}

QTEST_GUILESS_MAIN(TestProjectFileHandler)
