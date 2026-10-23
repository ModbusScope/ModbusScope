#include "tst_devicecombobox.h"

#include "../models/devicelisthelpers.h"
#include "customwidgets/devicecombobox.h"

#include <QJsonObject>
#include <QSignalSpy>
#include <QTest>

namespace {

constexpr deviceId_t cSecondDeviceId = 2;
constexpr deviceId_t cThirdDeviceId = 3;

} // namespace

void TestDeviceComboBox::init()
{
    _pSettingsModel = new SettingsModel();
    DeviceListHelpers::clearDevices(_pSettingsModel);
    DeviceListHelpers::seedDevice(_pSettingsModel, Device::cFirstDeviceId, QStringLiteral("modbus"),
                                  QStringLiteral("Boiler"));
    DeviceListHelpers::seedDevice(_pSettingsModel, cSecondDeviceId, QStringLiteral("sim"), QStringLiteral("Pump"));

    _pCombo = new DeviceComboBox();
    _pCombo->setSettingsModel(_pSettingsModel);
}

void TestDeviceComboBox::cleanup()
{
    delete _pCombo;
    _pCombo = nullptr;
    delete _pSettingsModel;
    _pSettingsModel = nullptr;
}

void TestDeviceComboBox::fillsFromModel()
{
    QCOMPARE(_pCombo->count(), 2);
    QCOMPARE(_pCombo->itemText(0), QStringLiteral("Boiler"));
    QCOMPARE(_pCombo->itemText(1), QStringLiteral("Pump"));
    QCOMPARE(_pCombo->selectedDeviceId(), Device::cFirstDeviceId);
}

void TestDeviceComboBox::nameProtocolIdLabels()
{
    _pCombo->setLabelStyle(DeviceComboBox::LabelStyle::NameProtocolId);

    QCOMPARE(_pCombo->itemText(0), QStringLiteral("Boiler (modbus) [1]"));
    QCOMPARE(_pCombo->itemText(1), QStringLiteral("Pump (sim) [2]"));
}

void TestDeviceComboBox::nameWithPlaceMarkerKeptInLabel()
{
    _pSettingsModel->deviceSettings(cSecondDeviceId)->setName(QStringLiteral("Pump %3"));
    _pCombo->setLabelStyle(DeviceComboBox::LabelStyle::NameProtocolId);

    QCOMPARE(_pCombo->itemText(1), QStringLiteral("Pump %3 (sim) [2]"));
}

void TestDeviceComboBox::adapterNameChangeUpdatesLabels()
{
    _pCombo->setLabelStyle(DeviceComboBox::LabelStyle::NameProtocolId);

    _pSettingsModel->updateAdapterFromDescribe(QStringLiteral("modbus"),
                                               QJsonObject{ { "protocolName", QStringLiteral("Modbus TCP") } });

    QCOMPARE(_pCombo->itemText(0), QStringLiteral("Boiler (Modbus TCP) [1]"));
    QCOMPARE(_pCombo->itemText(1), QStringLiteral("Pump (sim) [2]"));
}

void TestDeviceComboBox::setSelectedDeviceEmitsDeviceChanged()
{
    QSignalSpy spy(_pCombo, &DeviceComboBox::deviceChanged);

    _pCombo->setSelectedDeviceId(cSecondDeviceId);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<deviceId_t>(), cSecondDeviceId);
    QCOMPARE(_pCombo->selectedDeviceId(), cSecondDeviceId);
}

void TestDeviceComboBox::selectionKeptWhenOtherDeviceAdded()
{
    _pCombo->setSelectedDeviceId(cSecondDeviceId);
    QSignalSpy spy(_pCombo, &DeviceComboBox::deviceChanged);

    DeviceListHelpers::seedDevice(_pSettingsModel, cThirdDeviceId, QStringLiteral("modbus"));

    QCOMPARE(_pCombo->count(), 3);
    QCOMPARE(_pCombo->selectedDeviceId(), cSecondDeviceId);
    QCOMPARE(spy.count(), 0);
}

void TestDeviceComboBox::selectionKeptWhenOtherDeviceRemoved()
{
    _pCombo->setSelectedDeviceId(cSecondDeviceId);
    QSignalSpy spy(_pCombo, &DeviceComboBox::deviceChanged);

    DeviceListHelpers::dropDevice(_pSettingsModel, Device::cFirstDeviceId);

    QCOMPARE(_pCombo->count(), 1);
    QCOMPARE(_pCombo->selectedDeviceId(), cSecondDeviceId);
    QCOMPARE(spy.count(), 0);
}

void TestDeviceComboBox::selectedDeviceRemovedFallsBackToFirst()
{
    _pCombo->setSelectedDeviceId(cSecondDeviceId);
    QSignalSpy spy(_pCombo, &DeviceComboBox::deviceChanged);

    DeviceListHelpers::dropDevice(_pSettingsModel, cSecondDeviceId);

    QCOMPARE(_pCombo->selectedDeviceId(), Device::cFirstDeviceId);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<deviceId_t>(), Device::cFirstDeviceId);
}

void TestDeviceComboBox::lastDeviceRemovedEmitsZero()
{
    QSignalSpy spy(_pCombo, &DeviceComboBox::deviceChanged);

    DeviceListHelpers::clearDevices(_pSettingsModel);

    QCOMPARE(_pCombo->count(), 0);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<deviceId_t>(), deviceId_t(0));
}

QTEST_MAIN(TestDeviceComboBox)
