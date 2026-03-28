
#include "tst_projectfilehandler.h"

#include "importexport/projectfilehandler.h"
#include "models/device.h"
#include "models/graphdatamodel.h"
#include "models/guimodel.h"
#include "models/settingsmodel.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
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
 * \brief When the devices section is absent, the default device 1 is preserved.
 */
void TestProjectFileHandler::applyDeviceSettingsEmptyListPreservesDefault()
{
    const QString path = writeTempProjectFile(QJsonArray(), QJsonArray());
    QVERIFY(!path.isEmpty());

    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel(&settingsModel);
    ProjectFileHandler handler(&guiModel, &settingsModel, &graphDataModel);

    handler.openProjectFile(path);

    QVERIFY(settingsModel.deviceList().contains(Device::cFirstDeviceId));
    QCOMPARE(settingsModel.deviceSettings(Device::cFirstDeviceId)->adapterId(), QStringLiteral("modbus"));

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

QTEST_GUILESS_MAIN(TestProjectFileHandler)
