#include "adapterdevicesettings.h"

#include "customwidgets/addabletabwidget.h"
#include "customwidgets/deviceconfigtab.h"
#include "models/adapterdata.h"
#include "models/settingsmodel.h"

#include <QJsonArray>
#include <QLabel>
#include <QMap>
#include <QVBoxLayout>

AdapterDeviceSettings::AdapterDeviceSettings(SettingsModel* pSettingsModel, QWidget* parent)
    : QWidget(parent), _pSettingsModel(pSettingsModel)
{
    auto* layout = new QVBoxLayout(this);
    setLayout(layout);

    // Collect adapter IDs that have a populated schema
    QStringList validAdapterIds;
    const QStringList allAdapterIds = pSettingsModel->adapterIds();
    for (const auto& id : allAdapterIds)
    {
        const AdapterData* pAdapter = pSettingsModel->adapterData(id);
        if (!pAdapter->schema().isEmpty())
        {
            validAdapterIds.append(id);
        }
    }

    if (validAdapterIds.isEmpty())
    {
        layout->addWidget(new QLabel("No adapter schema available.", this));
        layout->addStretch();
        return;
    }

    _pDeviceTabs = new AddableTabWidget(this);
    layout->addWidget(_pDeviceTabs, 1);

    connect(_pDeviceTabs, &AddableTabWidget::addTabRequested, this, &AdapterDeviceSettings::handleAddTab);

    // Load devices from all adapters with a schema
    QList<QWidget*> pages;
    QStringList names;
    int tabIndex = 1;
    for (const auto& adapterId : validAdapterIds)
    {
        const AdapterData* pAdapter = pSettingsModel->adapterData(adapterId);
        const QJsonArray devices = pAdapter->effectiveConfig().value("devices").toArray();

        for (const auto& device : devices)
        {
            auto* tab = new DeviceConfigTab(pSettingsModel, adapterId, device.toObject(), _pDeviceTabs);
            pages.append(tab);
            names.append(constructTabName(device.toObject(), tabIndex++));
        }
    }

    if (!pages.isEmpty())
    {
        _pDeviceTabs->setTabs(pages, names);
    }
}

void AdapterDeviceSettings::handleAddTab()
{
    QString defaultAdapterId;
    const QStringList adapterIds = _pSettingsModel->adapterIds();
    for (const auto& id : adapterIds)
    {
        const AdapterData* pAdapter = _pSettingsModel->adapterData(id);
        if (!pAdapter->schema().isEmpty())
        {
            defaultAdapterId = id;
            break;
        }
    }

    if (defaultAdapterId.isEmpty())
    {
        return;
    }

    QJsonObject defaultValues;
    const QJsonArray defaultDevices =
      _pSettingsModel->adapterData(defaultAdapterId)->defaults().value("devices").toArray();
    if (!defaultDevices.isEmpty())
    {
        defaultValues = defaultDevices.first().toObject();
    }

    int tabIndex = _pDeviceTabs->count() + 1;
    auto* tab = new DeviceConfigTab(_pSettingsModel, defaultAdapterId, defaultValues, _pDeviceTabs);
    _pDeviceTabs->addNewTab(constructTabName(defaultValues, tabIndex), tab);
}

QString AdapterDeviceSettings::constructTabName(const QJsonObject& deviceValues, int tabIndex) const
{
    int id = deviceValues.value("id").toInt(-1);
    if (id >= 0)
    {
        const deviceId_t devId = static_cast<deviceId_t>(id);
        if (_pSettingsModel->hasDevice(devId))
        {
            Device* pDevice = _pSettingsModel->deviceSettings(devId);
            if (!pDevice->name().isEmpty())
            {
                return pDevice->name();
            }
        }
    }
    return QString("Device %1").arg(tabIndex);
}

void AdapterDeviceSettings::acceptValues()
{
    if (!_pDeviceTabs)
    {
        return;
    }

    QMap<QString, QJsonArray> devicesByAdapter;

    for (int i = 0; i < _pDeviceTabs->count(); ++i)
    {
        auto* tab = qobject_cast<DeviceConfigTab*>(_pDeviceTabs->tabContent(i));
        if (!tab)
        {
            continue;
        }
        const QJsonObject tabValues = tab->values();
        devicesByAdapter[tab->adapterId()].append(tabValues);

        int deviceId = tabValues.value("id").toInt(-1);
        if (deviceId >= 0 && _pSettingsModel->deviceList().contains(static_cast<deviceId_t>(deviceId)))
        {
            _pSettingsModel->deviceSettings(static_cast<deviceId_t>(deviceId))->setName(tab->deviceName());
        }
    }

    const QStringList adapterIds = _pSettingsModel->adapterIds();
    for (const auto& adapterId : adapterIds)
    {
        if (!devicesByAdapter.contains(adapterId))
        {
            continue;
        }
        const AdapterData* pAdapter = _pSettingsModel->adapterData(adapterId);
        if (pAdapter->schema().isEmpty())
        {
            continue;
        }
        QJsonObject config = pAdapter->effectiveConfig();
        config["devices"] = devicesByAdapter.value(adapterId);
        _pSettingsModel->setAdapterCurrentConfig(adapterId, config);
    }
}
