#include "devicesettings.h"
#include "ui_devicesettings.h"

#include "customwidgets/deviceform.h"
#include "models/device.h"

#include <QLabel>
#include <QToolButton>

DeviceSettings::DeviceSettings(SettingsModel* pSettingsModel, QWidget* parent)
    : QWidget(parent), _pUi(new Ui::DeviceSettings), _pSettingsModel(pSettingsModel)
{
    _pUi->setupUi(this);

    connect(_pUi->deviceTabs, &AddableTabWidget::tabClosed, this, &DeviceSettings::handleCloseTab,
            Qt::DirectConnection);
    connect(_pUi->deviceTabs, &AddableTabWidget::addTabRequested, this, &DeviceSettings::handleAddTab);

    QList<QWidget*> pages;
    QStringList names;
    for (deviceId_t const& devId : _pSettingsModel->deviceList())
    {
        names.append(constructTabName(devId));
        pages.append(createForm(devId));
    }

    _pUi->deviceTabs->setTabs(pages, names);
}

DeviceSettings::~DeviceSettings()
{
    delete _pUi;
}

DeviceForm* DeviceSettings::createForm(deviceId_t devId)
{
    DeviceForm* page = new DeviceForm(_pSettingsModel, devId, this);

    connect(this, &DeviceSettings::settingsTabsSwitched, page, &DeviceForm::handleSettingsTabSwitch);
    connect(page, &DeviceForm::deviceIdentifiersChanged, this, &DeviceSettings::updateTabName);

    return page;
}

void DeviceSettings::handleAddTab()
{
    deviceId_t devId = _pSettingsModel->addNewDevice();
    QString name = constructTabName(devId);

    _pUi->deviceTabs->addNewTab(name, createForm(devId));
}

QString DeviceSettings::constructTabName(deviceId_t devId)
{
    auto devSettings = _pSettingsModel->deviceSettings(devId);
    return QString("%1 (%2)").arg(devSettings->name()).arg(devId);
}

void DeviceSettings::updateTabName(deviceId_t devId)
{
    int index = -1;
    for (int i = 0; i < _pUi->deviceTabs->count(); ++i)
    {
        auto tabContent = dynamic_cast<DeviceForm*>(_pUi->deviceTabs->tabContent(i));
        if (tabContent && tabContent->deviceId() == devId)
        {
            index = i;
            break;
        }
    }

    if (index != -1)
    {
        QString name = constructTabName(devId);
        _pUi->deviceTabs->setTabName(index, name);
    }
}

void DeviceSettings::handleCloseTab(int index)
{
    Q_UNUSED(index);

    auto tabContent = dynamic_cast<DeviceForm*>(_pUi->deviceTabs->tabContent(index));
    if (tabContent)
    {
        deviceId_t devId = tabContent->deviceId();
        _pSettingsModel->removeDevice(devId);
    }
}
