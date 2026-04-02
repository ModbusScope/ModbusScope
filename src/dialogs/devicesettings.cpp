#include "devicesettings.h"

#include "customwidgets/deviceform.h"
#include "models/device.h"

#include <QVBoxLayout>

DeviceSettings::DeviceSettings(SettingsModel* pSettingsModel, QWidget* parent)
    : QWidget(parent), _pDeviceTabs(new AddableTabWidget(this)), _pSettingsModel(pSettingsModel)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(_pDeviceTabs);

    connect(_pDeviceTabs, &AddableTabWidget::tabClosed, this, &DeviceSettings::handleCloseTab, Qt::DirectConnection);
    connect(_pDeviceTabs, &AddableTabWidget::addTabRequested, this, &DeviceSettings::handleAddTab);

    QList<QWidget*> pages;
    QStringList names;
    const auto devList = _pSettingsModel->deviceList();
    for (deviceId_t const& devId : devList)
    {
        names.append(constructTabName(devId));
        pages.append(createForm(devId));
    }

    _pDeviceTabs->setTabs(pages, names);
}

DeviceForm* DeviceSettings::createForm(deviceId_t devId)
{
    DeviceForm* page = new DeviceForm(_pSettingsModel, devId, this);

    connect(page, &DeviceForm::deviceIdentifiersChanged, this, &DeviceSettings::updateTabName);

    return page;
}

void DeviceSettings::handleAddTab()
{
    deviceId_t devId = _pSettingsModel->addNewDevice();
    QString name = constructTabName(devId);

    _pDeviceTabs->addNewTab(name, createForm(devId));
}

QString DeviceSettings::constructTabName(deviceId_t devId)
{
    auto devSettings = _pSettingsModel->deviceSettings(devId);
    return QString("%1 (%2)").arg(devSettings->name()).arg(devId);
}

void DeviceSettings::updateTabName(deviceId_t devId)
{
    int index = -1;
    for (int i = 0; i < _pDeviceTabs->count(); ++i)
    {
        auto tabContent = dynamic_cast<DeviceForm*>(_pDeviceTabs->tabContent(i));
        if (tabContent && tabContent->deviceId() == devId)
        {
            index = i;
            break;
        }
    }

    if (index != -1)
    {
        QString name = constructTabName(devId);
        _pDeviceTabs->setTabName(index, name);
    }
}

void DeviceSettings::handleCloseTab(int index)
{
    auto tabContent = dynamic_cast<DeviceForm*>(_pDeviceTabs->tabContent(index));
    if (tabContent)
    {
        deviceId_t devId = tabContent->deviceId();
        _pSettingsModel->removeDevice(devId);
    }
}
