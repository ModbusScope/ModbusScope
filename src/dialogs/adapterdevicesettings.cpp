#include "adapterdevicesettings.h"

#include "customwidgets/addabletabwidget.h"
#include "customwidgets/deviceconfigtab.h"
#include "models/adapterdata.h"
#include "models/device.h"
#include "models/settingsmodel.h"

#include <QJsonArray>
#include <QLabel>
#include <QMap>
#include <QSet>
#include <QVBoxLayout>

AdapterDeviceSettings::AdapterDeviceSettings(SettingsModel* pSettingsModel, QWidget* parent)
    : QWidget(parent), _pSettingsModel(pSettingsModel)
{
    auto* layout = new QVBoxLayout(this);
    setLayout(layout);

    const QStringList adapterIds = validAdapterIds();
    if (adapterIds.isEmpty())
    {
        layout->addWidget(new QLabel("No adapter schema available.", this));
        layout->addStretch();
        return;
    }

    _pDeviceTabs = new AddableTabWidget(this);
    layout->addWidget(_pDeviceTabs, 1);

    connect(_pDeviceTabs, &AddableTabWidget::addTabRequested, this, &AdapterDeviceSettings::handleAddTab);
    connect(_pDeviceTabs, &AddableTabWidget::tabClosed, this, &AdapterDeviceSettings::handleCloseTab);

    QSet<deviceId_t> configDeviceIds;
    for (const auto& adapterId : adapterIds)
    {
        const QJsonArray devices = pSettingsModel->adapterData(adapterId)->effectiveConfig().value("devices").toArray();
        for (const auto& device : devices)
        {
            const int id = device.toObject().value("id").toInt(-1);
            if (id >= 0)
            {
                configDeviceIds.insert(static_cast<deviceId_t>(id));
            }
        }
    }
    const QList<deviceId_t> modelDeviceIds = pSettingsModel->deviceList();
    for (const deviceId_t devId : modelDeviceIds)
    {
        if (!configDeviceIds.contains(devId))
        {
            pSettingsModel->removeDevice(devId);
        }
    }

    QList<QWidget*> pages;
    QStringList names;
    for (const auto& adapterId : adapterIds)
    {
        const AdapterData* pAdapter = pSettingsModel->adapterData(adapterId);
        const QJsonArray devices = pAdapter->effectiveConfig().value("devices").toArray();

        for (const auto& device : devices)
        {
            const QJsonObject deviceObj = device.toObject();
            const int id = deviceObj.value("id").toInt(-1);
            if (id >= 0)
            {
                pSettingsModel->addDevice(static_cast<deviceId_t>(id));
            }
            auto* tab = new DeviceConfigTab(pSettingsModel, adapterId, deviceObj, _pDeviceTabs);
            connectTabNameTracking(tab);
            pages.append(tab);
            names.append(constructTabName(tab));
        }
    }

    if (!pages.isEmpty())
    {
        _pDeviceTabs->setTabs(pages, names);
    }
}

/*! \brief Add a new device tab with a unique, auto-incremented device ID.
 *
 * Creates a SettingsModel device via addNewDevice() to obtain a unique ID,
 * sets its adapter, then opens a new DeviceConfigTab pre-populated with
 * the adapter's default values and the assigned ID.
 */
void AdapterDeviceSettings::handleAddTab()
{
    const QStringList adapterIds = validAdapterIds();
    if (adapterIds.isEmpty())
    {
        return;
    }
    const QString defaultAdapterId = adapterIds.first();

    QJsonObject defaultValues;
    const QJsonArray defaultDevices =
      _pSettingsModel->adapterData(defaultAdapterId)->defaults().value("devices").toArray();
    if (!defaultDevices.isEmpty())
    {
        defaultValues = defaultDevices.first().toObject();
    }

    deviceId_t maxId = 0;
    const QList<deviceId_t> modelIds = _pSettingsModel->deviceList();
    if (!modelIds.isEmpty())
    {
        maxId = modelIds.last();
    }
    for (int i = 0; i < _pDeviceTabs->count(); ++i)
    {
        auto* tab = qobject_cast<DeviceConfigTab*>(_pDeviceTabs->tabContent(i));
        if (tab)
        {
            const int id = tab->values().value("id").toInt(-1);
            if (id >= 0)
            {
                maxId = qMax(maxId, static_cast<deviceId_t>(id));
            }
        }
    }
    const deviceId_t newId = (maxId > 0) ? maxId + 1 : Device::cFirstDeviceId;
    _pSettingsModel->addDevice(newId);
    _pSettingsModel->deviceSettings(newId)->setAdapterId(defaultAdapterId);
    defaultValues["id"] = static_cast<int>(newId);

    auto* tab = new DeviceConfigTab(_pSettingsModel, defaultAdapterId, defaultValues, _pDeviceTabs);
    connectTabNameTracking(tab);
    _pDeviceTabs->addNewTab(constructTabName(tab), tab);
}

void AdapterDeviceSettings::handleCloseTab(QWidget* widget)
{
    auto* tab = qobject_cast<DeviceConfigTab*>(widget);
    if (tab && tab->deviceId() >= 0)
    {
        _pSettingsModel->removeDevice(static_cast<deviceId_t>(tab->deviceId()));
    }
}

QStringList AdapterDeviceSettings::validAdapterIds() const
{
    QStringList result;
    const QStringList allAdapterIds = _pSettingsModel->adapterIds();
    for (const auto& id : allAdapterIds)
    {
        const AdapterData* pAdapter = _pSettingsModel->adapterData(id);
        if (!pAdapter->schema().isEmpty())
        {
            result.append(id);
        }
    }
    return result;
}

QString AdapterDeviceSettings::constructTabName(DeviceConfigTab* tab) const
{
    const int id = tab->deviceId();
    if (id >= 0)
    {
        const deviceId_t devId = static_cast<deviceId_t>(id);
        if (_pSettingsModel->hasDevice(devId))
        {
            const QString name = _pSettingsModel->deviceSettings(devId)->name();
            if (!name.isEmpty())
            {
                return name;
            }
        }
        return QString("Device #%1").arg(id);
    }
    return QStringLiteral("Device");
}

void AdapterDeviceSettings::connectTabNameTracking(DeviceConfigTab* tab)
{
    connect(tab, &DeviceConfigTab::nameChanged, tab,
            [this, tab]() { _pDeviceTabs->setTabName(_pDeviceTabs->indexOf(tab), constructTabName(tab)); });
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
        if (tab)
        {
            devicesByAdapter[tab->adapterId()].append(tab->values());
        }
    }

    const QStringList allAdapterIds = validAdapterIds();
    for (const auto& adapterId : allAdapterIds)
    {
        QJsonObject config = _pSettingsModel->adapterData(adapterId)->effectiveConfig();
        config["devices"] = devicesByAdapter.value(adapterId);
        _pSettingsModel->setAdapterCurrentConfig(adapterId, config);
    }
}
