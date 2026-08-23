#include "adapterdevicesettings.h"

#include "ProtocolAdapter/adapterhub.h"
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

#include <algorithm>
#include <climits>
#include <numeric>

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

    _pLimitWarningLabel = new QLabel(this);
    _pLimitWarningLabel->setObjectName("deviceLimitWarningLabel");
    _pLimitWarningLabel->setWordWrap(true);
    /* Adapter names come from the adapter subprocess's describe response, so treat them as
     * untrusted: force plain text to prevent a misbehaving adapter from injecting rich-text
     * formatting into this label. */
    _pLimitWarningLabel->setTextFormat(Qt::PlainText);
    _pLimitWarningLabel->setStyleSheet("QLabel { color: #b35900; }");
    _pLimitWarningLabel->setVisible(false);
    layout->addWidget(_pLimitWarningLabel);

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

    /* Must run before the tab-building loop below: that loop's ownership check assumes
     * device ownership is already reconciled. */
    pSettingsModel->reconcileDevicesWithAdapters();

    QList<QWidget*> pages;
    QStringList names;
    QSet<deviceId_t> seenDeviceIds;
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
                const deviceId_t devId = static_cast<deviceId_t>(id);
                if (seenDeviceIds.contains(devId))
                {
                    continue;
                }
                if (pSettingsModel->hasDevice(devId) && pSettingsModel->adapterIdForDevice(devId) != adapterId)
                {
                    /* reconcileDevicesWithAdapters() above already resolved this device ID to a
                     * different adapter (e.g. that adapter has an explicit stored config, while
                     * this one still only shares the ID in its untouched defaults). Skip it here
                     * so its tab is built from the reconciled owner's own declaration below,
                     * instead of from this adapter's declaration purely because it was iterated
                     * first — building it from the wrong adapter would make acceptValues() write
                     * an empty devices array back to the real owner, wiping its stored config. */
                    continue;
                }
                seenDeviceIds.insert(devId);
            }
            auto* tab = new DeviceConfigTab(pSettingsModel, adapterId, deviceObj, _pDeviceTabs);
            connectTabTracking(tab);
            pages.append(tab);
            names.append(constructTabName(tab));
        }
    }

    if (!pages.isEmpty())
    {
        sortPagesByDeviceId(pages, names);
        _pDeviceTabs->setTabs(pages, names);
    }

    updateDeviceLimitIndication();
}

/*! \brief Stable-sort device tabs by device ID.
 *
 * Tabs are built adapter-by-adapter (in \c SettingsModel::adapterIds() order, which is
 * alphabetical, not creation order), so a device on an alphabetically-earlier adapter would
 * otherwise be shown before a device added earlier but living on a later adapter. Sorting by
 * ID keeps tab order consistent with device add order across dialog reopens. Tabs without a
 * valid ID (id < 0) keep their relative order and sort after all ID'd tabs. As a side effect,
 * two devices on the same adapter whose stored JSON array was not already ID-ascending get
 * normalized to ID order too, since acceptValues() re-derives that array from tab order.
 */
void AdapterDeviceSettings::sortPagesByDeviceId(QList<QWidget*>& pages, QStringList& names)
{
    QList<long long> ids;
    ids.reserve(pages.size());
    for (auto* page : pages)
    {
        auto* tab = qobject_cast<DeviceConfigTab*>(page);
        const int id = tab ? tab->deviceId() : -1;
        ids.append(id >= 0 ? static_cast<long long>(id) : LLONG_MAX);
    }

    QList<int> order(pages.size());
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(), [&ids](int a, int b) { return ids[a] < ids[b]; });

    QList<QWidget*> sortedPages;
    QStringList sortedNames;
    for (const int index : order)
    {
        sortedPages.append(pages[index]);
        sortedNames.append(names[index]);
    }
    pages = sortedPages;
    names = sortedNames;
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
    const QString defaultAdapterId =
      adapterIds.contains(QString(cModbusAdapterId)) ? QString(cModbusAdapterId) : adapterIds.first();

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
    connectTabTracking(tab);
    _pDeviceTabs->addNewTab(constructTabName(tab), tab);

    updateDeviceLimitIndication();
}

void AdapterDeviceSettings::handleCloseTab(QWidget* widget)
{
    auto* tab = qobject_cast<DeviceConfigTab*>(widget);
    if (tab && tab->deviceId() >= 0)
    {
        _pSettingsModel->removeDevice(static_cast<deviceId_t>(tab->deviceId()));
    }

    updateDeviceLimitIndication();
}

/*! \brief Build a warning message listing adapters whose configured device count exceeds
 *  their effective device limit (the smaller of the schema's maxItems and the adapter's
 *  reported capabilities.maxDevices), or an empty string if all adapters are within their limit.
 */
QString AdapterDeviceSettings::deviceLimitWarningMessage() const
{
    if (!_pDeviceTabs)
    {
        return QString();
    }

    QMap<QString, int> countByAdapter;
    for (int i = 0; i < _pDeviceTabs->count(); ++i)
    {
        auto* tab = qobject_cast<DeviceConfigTab*>(_pDeviceTabs->tabContent(i));
        if (tab)
        {
            ++countByAdapter[tab->adapterId()];
        }
    }

    QStringList warnings;
    for (auto it = countByAdapter.constBegin(); it != countByAdapter.constEnd(); ++it)
    {
        const AdapterData* pAdapter = _pSettingsModel->adapterData(it.key());
        const int deviceLimit = pAdapter->maxDevices();
        if (it.value() > deviceLimit)
        {
            const QString adapterName = pAdapter->name().isEmpty() ? it.key() : pAdapter->name();
            warnings.append(QString("%1 allows at most %2 device(s), but %3 are configured.")
                              .arg(adapterName)
                              .arg(deviceLimit)
                              .arg(it.value()));
        }
    }
    return warnings.join('\n');
}

void AdapterDeviceSettings::updateDeviceLimitIndication()
{
    if (!_pLimitWarningLabel)
    {
        return;
    }
    const QString message = deviceLimitWarningMessage();
    _pLimitWarningLabel->setText(message);
    _pLimitWarningLabel->setVisible(!message.isEmpty());
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

void AdapterDeviceSettings::connectTabTracking(DeviceConfigTab* tab)
{
    connect(tab, &DeviceConfigTab::nameChanged, tab,
            [this, tab]() { _pDeviceTabs->setTabName(_pDeviceTabs->indexOf(tab), constructTabName(tab)); });
    connect(tab, &DeviceConfigTab::adapterChanged, this, &AdapterDeviceSettings::updateDeviceLimitIndication);
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
