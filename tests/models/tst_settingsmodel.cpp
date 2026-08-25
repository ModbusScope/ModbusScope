#include "tst_settingsmodel.h"

#include "models/device.h"
#include "models/settingsmodel.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QSignalSpy>
#include <QTest>

namespace {

//! Build a device with the given name and owning adapter.
Device makeDevice(deviceId_t devId, const QString& name, const QString& adapterId)
{
    Device device(devId);
    device.setName(name);
    device.setAdapterId(adapterId);
    return device;
}

/*! \brief Build a describe result declaring the given device IDs as untouched defaults.
 *
 * The schema only has to be non-empty: reconcileDevicesWithAdapters() skips adapters that
 * have not published one, but never looks inside it.
 */
QJsonObject makeDescribeWithDefaultDevices(const QList<int>& deviceIds)
{
    QJsonArray devices;
    for (const int id : deviceIds)
    {
        QJsonObject device;
        device["id"] = id;
        devices.append(device);
    }

    QJsonObject defaults;
    defaults["devices"] = devices;

    QJsonObject schema;
    schema["type"] = "object";

    QJsonObject describe;
    describe["schema"] = schema;
    describe["defaults"] = defaults;
    return describe;
}

} // namespace

void TestSettingsModel::applyDeviceListReplacesList()
{
    SettingsModel model;

    // SettingsModel starts with device 1; it must not survive a list that omits it.
    QVERIFY(model.hasDevice(Device::cFirstDeviceId));

    QMap<deviceId_t, Device> devices;
    devices.insert(2, makeDevice(2, "Pump", "modbus"));
    devices.insert(5, makeDevice(5, "Sensor", "dummy"));

    model.applyDeviceList(devices);

    QCOMPARE(model.deviceList(), QList<deviceId_t>({ 2, 5 }));
    QVERIFY(!model.hasDevice(Device::cFirstDeviceId));
    QCOMPARE(model.deviceSettings(2)->name(), QStringLiteral("Pump"));
    QCOMPARE(model.deviceSettings(5)->adapterId(), QStringLiteral("dummy"));
}

void TestSettingsModel::applyDeviceListEmitsOnceForMultipleChanges()
{
    SettingsModel model;
    model.addDevice(2);

    QSignalSpy spy(&model, &SettingsModel::deviceListChanged);

    // Removes device 1, keeps 2 under a new name, adds 3 — one coherent change.
    QMap<deviceId_t, Device> devices;
    devices.insert(2, makeDevice(2, "Renamed", "modbus"));
    devices.insert(3, makeDevice(3, "Added", "dummy"));

    model.applyDeviceList(devices);

    QCOMPARE(spy.count(), 1);
}

void TestSettingsModel::applyDeviceListIsSilentWhenNothingChanged()
{
    SettingsModel model;

    QMap<deviceId_t, Device> devices;
    devices.insert(1, makeDevice(1, "Pump", "modbus"));
    model.applyDeviceList(devices);

    QSignalSpy spy(&model, &SettingsModel::deviceListChanged);
    model.applyDeviceList(devices);

    QCOMPARE(spy.count(), 0);
}

void TestSettingsModel::applyDeviceListDetectsFieldOnlyChange()
{
    SettingsModel model;

    QMap<deviceId_t, Device> devices;
    devices.insert(1, makeDevice(1, "Pump", "modbus"));
    model.applyDeviceList(devices);

    QSignalSpy spy(&model, &SettingsModel::deviceListChanged);

    // Same device IDs, different owner: renames and adapter reassignments must notify too,
    // since Device's own setters are plain field writes that emit nothing.
    devices.insert(1, makeDevice(1, "Pump", "dummy"));
    model.applyDeviceList(devices);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(model.deviceSettings(1)->adapterId(), QStringLiteral("dummy"));
}

void TestSettingsModel::applyDeviceListEmptyClearsList()
{
    SettingsModel model;

    QSignalSpy spy(&model, &SettingsModel::deviceListChanged);
    model.applyDeviceList(QMap<deviceId_t, Device>());

    QVERIFY(model.deviceList().isEmpty());
    QCOMPARE(spy.count(), 1);
}

void TestSettingsModel::reconcileEmitsOnceWhenClaimingMultipleDevices()
{
    SettingsModel model;

    QSignalSpy spy(&model, &SettingsModel::deviceListChanged);

    /* Devices 5, 6 and 7 are new to the model and device 1 - the one SettingsModel's own
     * constructor creates - moves off its "modbus" default. Four changes, one describe,
     * one signal. */
    model.updateAdapterFromDescribe("adapterA", makeDescribeWithDefaultDevices({ 1, 5, 6, 7 }));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(model.deviceList(), QList<deviceId_t>({ 1, 5, 6, 7 }));
    QCOMPARE(model.deviceSettings(5)->adapterId(), QStringLiteral("adapterA"));
    QCOMPARE(model.deviceSettings(1)->adapterId(), QStringLiteral("adapterA"));
}

void TestSettingsModel::reconcileKeepsDeviceNoAdapterDeclares()
{
    SettingsModel model;

    QMap<deviceId_t, Device> devices;
    devices.insert(3, makeDevice(3, "Orphan", "adapterA"));
    model.applyDeviceList(devices);

    QSignalSpy spy(&model, &SettingsModel::deviceListChanged);

    // adapterA declares device 1 only; device 3 is in no adapter's config and must survive.
    model.updateAdapterFromDescribe("adapterA", makeDescribeWithDefaultDevices({ 1 }));

    QVERIFY(model.hasDevice(3));
    QCOMPARE(model.deviceSettings(3)->name(), QStringLiteral("Orphan"));
    QCOMPARE(model.deviceSettings(3)->adapterId(), QStringLiteral("adapterA"));
    QVERIFY(model.hasDevice(1));
    QCOMPARE(spy.count(), 1);
}

void TestSettingsModel::reconcileKeepsNameWhenAdapterClaimsDevice()
{
    SettingsModel model;

    QMap<deviceId_t, Device> devices;
    devices.insert(1, makeDevice(1, "Pump", "modbus"));
    model.applyDeviceList(devices);

    // Claiming a device for another adapter changes its owner, never its name.
    model.updateAdapterFromDescribe("adapterA", makeDescribeWithDefaultDevices({ 1 }));

    QCOMPARE(model.deviceSettings(1)->adapterId(), QStringLiteral("adapterA"));
    QCOMPARE(model.deviceSettings(1)->name(), QStringLiteral("Pump"));
}

void TestSettingsModel::reconcileIsSilentWhenNothingChanges()
{
    SettingsModel model;
    model.updateAdapterFromDescribe("modbus", makeDescribeWithDefaultDevices({ 1 }));

    QSignalSpy spy(&model, &SettingsModel::deviceListChanged);
    model.reconcileDevicesWithAdapters();

    QCOMPARE(spy.count(), 0);
}

QTEST_MAIN(TestSettingsModel)
