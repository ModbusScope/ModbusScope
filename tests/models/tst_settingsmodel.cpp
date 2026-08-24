#include "tst_settingsmodel.h"

#include "models/device.h"
#include "models/settingsmodel.h"

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

QTEST_MAIN(TestSettingsModel)
