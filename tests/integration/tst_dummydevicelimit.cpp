#include "tst_dummydevicelimit.h"

#include "../models/devicelisthelpers.h"
#include "modbusconfighelpers.h"

#include "ProtocolAdapter/adaptermanager.h"
#include "models/adapterdata.h"
#include "models/device.h"
#include "models/settingsmodel.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTest>

namespace {
constexpr int cSessionTimeoutMs = 10000;
constexpr char cAdapterId[] = "modbus";
} // namespace

void TestDummyDeviceLimit::init()
{
    _pSettingsModel = new SettingsModel;
    _pAdapterManager = new AdapterManager(QString::fromUtf8(cAdapterId), QString::fromUtf8(DUMMY_ADAPTER_EXECUTABLE),
                                          _pSettingsModel, this);
}

void TestDummyDeviceLimit::cleanup()
{
    delete _pAdapterManager;
    _pAdapterManager = nullptr;
    delete _pSettingsModel;
    _pSettingsModel = nullptr;
}

/*!
 * \brief Configuring more devices than the adapter allows must still start a session: the client
 * cuts the device list down to the limit the adapter declared instead of sending it as is.
 */
void TestDummyDeviceLimit::sessionStartsWithMoreDevicesThanLimit()
{
    QSignalSpy spyReady(_pAdapterManager, &AdapterManager::adapterReady);
    _pAdapterManager->initAdapter();
    QVERIFY2(spyReady.wait(cSessionTimeoutMs), "adapterReady not emitted");

    const AdapterData* pAdapterData = _pSettingsModel->adapterData(QString::fromUtf8(cAdapterId));
    QVERIFY(pAdapterData != nullptr);
    const int deviceLimit = pAdapterData->maxDevices();
    QVERIFY2(deviceLimit >= 1 && deviceLimit < 1000, "expected the adapter to declare a finite device limit");

    QJsonArray devices;
    for (int id = 1; id <= deviceLimit + 1; id++)
    {
        DeviceListHelpers::seedDevice(_pSettingsModel, id, QString::fromUtf8(cAdapterId));
        devices.append(ModbusConfigHelpers::device(id, 1, id));
    }

    _pSettingsModel->setAdapterCurrentConfig(
      QString::fromUtf8(cAdapterId),
      ModbusConfigHelpers::config(QJsonArray({ ModbusConfigHelpers::connection(1) }), devices));

    QSignalSpy spyStarted(_pAdapterManager, &AdapterManager::sessionStarted);
    QSignalSpy spyError(_pAdapterManager, &AdapterManager::sessionError);

    _pAdapterManager->startSession(QStringList());

    QVERIFY2(spyStarted.wait(cSessionTimeoutMs), "sessionStarted not emitted with more devices than the limit");
    QCOMPARE(spyError.count(), 0);
}

QTEST_GUILESS_MAIN(TestDummyDeviceLimit)
