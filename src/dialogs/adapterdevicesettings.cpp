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
    for (const auto& id : pSettingsModel->adapterIds())
    {
        if (!pSettingsModel->adapterData(id)->schema().isEmpty())
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

    connect(_pDeviceTabs, &AddableTabWidget::addTabRequested, this,
            &AdapterDeviceSettings::handleAddTab);

    // Load devices from all adapters with a schema
    QList<QWidget*> pages;
    QStringList names;
    int tabIndex = 1;
    for (const auto& adapterId : validAdapterIds)
    {
        AdapterData* pAdapter = pSettingsModel->adapterData(adapterId);
        QJsonArray devices = pAdapter->effectiveConfig().value("devices").toArray();

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
    for (const auto& id : _pSettingsModel->adapterIds())
    {
        if (!_pSettingsModel->adapterData(id)->schema().isEmpty())
        {
            defaultAdapterId = id;
            break;
        }
    }

    if (defaultAdapterId.isEmpty())
    {
        return;
    }

    int tabIndex = _pDeviceTabs->count() + 1;
    auto* tab = new DeviceConfigTab(_pSettingsModel, defaultAdapterId, QJsonObject(), _pDeviceTabs);
    _pDeviceTabs->addNewTab(constructTabName(QJsonObject(), tabIndex), tab);
}

QString AdapterDeviceSettings::constructTabName(const QJsonObject& deviceValues, int tabIndex) const
{
    QString name = deviceValues.value("name").toString();
    if (!name.isEmpty())
    {
        return name;
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
        devicesByAdapter[tab->adapterId()].append(tab->values());
    }

    for (auto it = devicesByAdapter.constBegin(); it != devicesByAdapter.constEnd(); ++it)
    {
        AdapterData* pAdapter = _pSettingsModel->adapterData(it.key());
        QJsonObject config = pAdapter->effectiveConfig();
        config["devices"] = it.value();
        pAdapter->setCurrentConfig(config);
        pAdapter->setHasStoredConfig(true);
    }
}
