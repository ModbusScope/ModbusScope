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
        auto devSettings = _pSettingsModel->deviceSettings(devId);
        names.append(QString("%1 (%2)").arg(devSettings->name()).arg(devId));
        pages.append(new DeviceForm(_pSettingsModel, devId, this));
    }

    _pUi->deviceTabs->setTabs(pages, names);
}

DeviceSettings::~DeviceSettings()
{
    delete _pUi;
}

void DeviceSettings::handleAddTab()
{
    deviceId_t devId = _pSettingsModel->addNewDevice();
    auto devSettings = _pSettingsModel->deviceSettings(devId);
    QString name = QString("%1 (%2)").arg(devSettings->name()).arg(devId);
    DeviceForm* page = new DeviceForm(_pSettingsModel, devId, this);

    _pUi->deviceTabs->addNewTab(name, page);
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