
#include "tst_projectfilehandler.h"

#include "importexport/projectfilehandler.h"
#include "models/adapterdata.h"
#include "models/device.h"
#include "models/graphdatamodel.h"
#include "models/guimodel.h"
#include "models/settingsmodel.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>

namespace {

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
 * \brief Loading a project with an empty devices section clears all devices, including stale ones.
 *
 * Regression: applyDeviceSettings used to return early before calling removeAllDevice() when
 * the list was empty, leaving previously-loaded devices in the model.
 */
void TestProjectFileHandler::applyDeviceSettingsEmptyListClearsModel()
{
    const QString path = writeTempProjectFile(QJsonArray(), QJsonArray());
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);

    /* Pre-populate a stale device that should be cleared */
    settingsModel.addDevice(99);
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
    settingsModel.addDevice(3);

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

QTEST_GUILESS_MAIN(TestProjectFileHandler)
