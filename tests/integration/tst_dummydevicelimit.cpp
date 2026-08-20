#include "tst_dummydevicelimit.h"

#include "ProtocolAdapter/adaptermanager.h"
#include "models/device.h"
#include "models/settingsmodel.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTest>

namespace {
constexpr int cSessionTimeoutMs = 10000;
}

void TestDummyDeviceLimit::init()
{
    _pSettingsModel = new SettingsModel;
    _pAdapterManager = new AdapterManager(
      QStringLiteral("dummy"), QString::fromUtf8(DUMMY_STANDALONE_ADAPTER_EXECUTABLE), _pSettingsModel, this);
}

void TestDummyDeviceLimit::cleanup()
{
    delete _pAdapterManager;
    _pAdapterManager = nullptr;
    delete _pSettingsModel;
    _pSettingsModel = nullptr;
}

void TestDummyDeviceLimit::sessionStartsWithTwoDevicesOverLimit()
{
    QSignalSpy spyReady(_pAdapterManager, &AdapterManager::adapterReady);
    _pAdapterManager->initAdapter();
    QVERIFY2(spyReady.wait(cSessionTimeoutMs), "adapterReady not emitted");

    // The dummy adapter's schema declares devices.maxItems = 1 (capabilities.maxDevices = 1).
    _pSettingsModel->addDevice(1);
    _pSettingsModel->deviceSettings(1)->setAdapterId(QStringLiteral("dummy"));
    _pSettingsModel->addDevice(2);
    _pSettingsModel->deviceSettings(2)->setAdapterId(QStringLiteral("dummy"));

    QJsonObject connection;
    connection["id"] = 1;
    connection["name"] = "Connection 1";

    QJsonObject device1;
    device1["id"] = 1;
    device1["connectionId"] = 1;

    QJsonObject device2;
    device2["id"] = 2;
    device2["connectionId"] = 1;

    QJsonObject config;
    config["version"] = 1;
    config["general"] = QJsonObject();
    config["connections"] = QJsonArray({ connection });
    config["devices"] = QJsonArray({ device1, device2 });

    _pSettingsModel->setAdapterCurrentConfig(QStringLiteral("dummy"), config);

    QSignalSpy spyStarted(_pAdapterManager, &AdapterManager::sessionStarted);
    QSignalSpy spyError(_pAdapterManager, &AdapterManager::sessionError);

    _pAdapterManager->startSession(QStringList());

    QVERIFY2(spyStarted.wait(cSessionTimeoutMs),
             "sessionStarted not emitted with 2 devices for a maxDevices=1 adapter");
    QCOMPARE(spyError.count(), 0);
}

QTEST_GUILESS_MAIN(TestDummyDeviceLimit)
