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
#include <QVBoxLayout>

#include <utility>

AdapterDeviceSettings::AdapterDeviceSettings(SettingsModel* pSettingsModel, QWidget* parent)
    : QWidget(parent), _pSettingsModel(pSettingsModel)
{
    auto* layout = new QVBoxLayout(this);
    setLayout(layout);

    _validAdapterIds = validAdapterIds();
    if (_validAdapterIds.isEmpty())
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

    /* Must run before the tab-building loop below: it reads each device's owner back from the
     * model, which reconciliation is what decides. */
    pSettingsModel->reconcileDevicesWithAdapters();

    /* One tab per device the model knows. reconcileDevicesWithAdapters() has already
     * registered every device the adapters declare, so this covers those too — and it
     * additionally shows a device the model holds that its own adapter's config does not
     * mention (from an older or hand-edited project file) instead of silently dropping it.
     * deviceList() is ascending, so tabs come out in device ID order without a separate sort. */
    QList<QWidget*> pages;
    QStringList names;
    const QList<deviceId_t> deviceIds = pSettingsModel->deviceList();
    for (const deviceId_t devId : deviceIds)
    {
        const QString adapterId = pSettingsModel->adapterIdForDevice(devId);
        if (!_validAdapterIds.contains(adapterId))
        {
            /* The owning adapter has not described, so there is no schema to render this
             * device with. Showing it under another adapter would make accepting the dialog
             * reassign it and reset its fields to that adapter's defaults, so note it here and
             * hand it back to the model unchanged in acceptValues(). */
            _deviceIdsWithoutTab.append(devId);
            continue;
        }

        auto* tab =
          createTab(adapterId, pSettingsModel->deviceSettings(devId)->name(), deviceValuesFor(adapterId, devId));
        pages.append(tab);
        names.append(constructTabName(tab));
    }

    appendTabsWithoutDeviceId(pages, names);

    if (!pages.isEmpty())
    {
        _pDeviceTabs->setTabs(pages, names);
    }

    updateDeviceLimitIndication();
}

/*! \brief Append a tab for every adapter-declared device that carries no usable device ID.
 *
 * SettingsModel's device list is keyed by device ID, so such an entry cannot be represented
 * there and is missed by the tab loop in the constructor. Appending them last keeps every
 * valid device ahead of them in tab order.
 * \param pages  Tab widgets built so far; appended to.
 * \param names  Matching tab titles; appended to.
 */
void AdapterDeviceSettings::appendTabsWithoutDeviceId(QList<QWidget*>& pages, QStringList& names)
{
    for (const auto& adapterId : std::as_const(_validAdapterIds))
    {
        const QJsonArray devices =
          _pSettingsModel->adapterData(adapterId)->effectiveConfig().value("devices").toArray();
        for (const auto& device : devices)
        {
            const QJsonObject deviceObj = device.toObject();
            if (deviceObj.value("id").toInt(-1) < 0)
            {
                auto* tab = createTab(adapterId, QString(), deviceObj);
                pages.append(tab);
                names.append(constructTabName(tab));
            }
        }
    }
}

/*! \brief Add a new device tab with a unique, auto-incremented device ID.
 *
 * The tab is the working copy: nothing is registered in SettingsModel until acceptValues()
 * runs, so cancelling the dialog leaves the device list untouched. The new ID is one past the
 * highest ID held by the model or by an already open tab, so it collides with neither a saved
 * device nor one added earlier in this session.
 */
void AdapterDeviceSettings::handleAddTab()
{
    const QString adapterId = defaultAdapterId();
    if (adapterId.isEmpty())
    {
        return;
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
        if (tab && tab->deviceId() >= 0)
        {
            maxId = qMax(maxId, static_cast<deviceId_t>(tab->deviceId()));
        }
    }
    const deviceId_t newId = (maxId > 0) ? maxId + 1 : Device::cFirstDeviceId;

    /* newId is higher than every known device, so this yields the adapter's default device
     * with the new ID applied. Device's constructor supplies the same default name the model
     * would have given it. */
    auto* tab = createTab(adapterId, Device(newId).name(), deviceValuesFor(adapterId, newId));
    _pDeviceTabs->addNewTab(constructTabName(tab), tab);

    updateDeviceLimitIndication();
}

void AdapterDeviceSettings::handleCloseTab(QWidget* widget)
{
    Q_UNUSED(widget);

    /* The tab has already been taken out of _pDeviceTabs, so the working copy no longer holds
     * this device. It leaves SettingsModel only once acceptValues() runs. */
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

/*! \brief Return the adapter a device should target when nothing else decides it.
 * \return "modbus" when it has a usable schema, so the app's built-in initial device keeps its
 * conventional owner; otherwise the first adapter that has one, or an empty string when none has.
 */
QString AdapterDeviceSettings::defaultAdapterId() const
{
    if (_validAdapterIds.contains(QString(cModbusAdapterId)))
    {
        return QString(cModbusAdapterId);
    }
    return _validAdapterIds.isEmpty() ? QString() : _validAdapterIds.first();
}

/*! \brief Build the schema form values for one device on a given adapter.
 * \param adapterId  The adapter whose config and defaults to read.
 * \param devId      The device identifier.
 * \return The device's own entry in the adapter's effective config when it declares one.
 * Otherwise the adapter's default device with \a devId applied, which covers both a device the
 * model knows but the config does not declare, and a device being added for the first time.
 */
QJsonObject AdapterDeviceSettings::deviceValuesFor(const QString& adapterId, deviceId_t devId) const
{
    const AdapterData* pAdapter = _pSettingsModel->adapterData(adapterId);

    const QJsonArray devices = pAdapter->effectiveConfig().value("devices").toArray();
    for (const auto& device : devices)
    {
        const QJsonObject deviceObj = device.toObject();
        if (deviceObj.value("id").toInt(-1) == static_cast<int>(devId))
        {
            return deviceObj;
        }
    }

    QJsonObject values;
    const QJsonArray defaultDevices = pAdapter->defaults().value("devices").toArray();
    if (!defaultDevices.isEmpty())
    {
        values = defaultDevices.first().toObject();
    }
    values["id"] = static_cast<int>(devId);
    return values;
}

QString AdapterDeviceSettings::constructTabName(DeviceConfigTab* tab) const
{
    const int id = tab->deviceId();
    if (id < 0)
    {
        return QStringLiteral("Device");
    }

    const QString name = tab->deviceName();
    return name.isEmpty() ? QString("Device #%1").arg(id) : name;
}

/*! \brief Create a device tab and keep its title and the device-limit warning in sync with it.
 * \param adapterId    The adapter to preselect in the tab's adapter combo.
 * \param deviceName   The name to show in the tab's name field.
 * \param deviceValues The schema form values for the device.
 * \return The new tab. It is not added to the tab widget — the caller decides where it goes.
 */
DeviceConfigTab* AdapterDeviceSettings::createTab(const QString& adapterId,
                                                  const QString& deviceName,
                                                  const QJsonObject& deviceValues)
{
    auto* tab = new DeviceConfigTab(_pSettingsModel, adapterId, deviceName, deviceValues, _pDeviceTabs);

    connect(tab, &DeviceConfigTab::nameChanged, tab,
            [this, tab]() { _pDeviceTabs->setTabName(_pDeviceTabs->indexOf(tab), constructTabName(tab)); });
    connect(tab, &DeviceConfigTab::adapterChanged, this, &AdapterDeviceSettings::updateDeviceLimitIndication);

    return tab;
}

/*! \brief Apply the open tabs to SettingsModel in one transaction.
 *
 * Writes each adapter's device JSON array, then the device list itself. A device added,
 * removed, renamed or moved to another adapter on this page reaches the model only here, so
 * cancelling the dialog discards all of it. The device list is applied in a single call so
 * observers of deviceListChanged() see one coherent change instead of a per-device sequence.
 */
void AdapterDeviceSettings::acceptValues()
{
    if (!_pDeviceTabs)
    {
        return;
    }

    QMap<QString, QJsonArray> devicesByAdapter;

    /* Devices with no tab were never editable here, so they are read back from the model as it
     * stands now rather than from a copy taken when the dialog opened — that would revert a
     * change the model made in the meantime, such as an ownership reconcile. One that has since
     * been removed from the model stays removed. */
    QMap<deviceId_t, Device> devices;
    for (const deviceId_t devId : std::as_const(_deviceIdsWithoutTab))
    {
        if (_pSettingsModel->hasDevice(devId))
        {
            devices.insert(devId, *_pSettingsModel->deviceSettings(devId));
        }
    }

    for (int i = 0; i < _pDeviceTabs->count(); ++i)
    {
        auto* tab = qobject_cast<DeviceConfigTab*>(_pDeviceTabs->tabContent(i));
        if (tab == nullptr)
        {
            continue;
        }

        devicesByAdapter[tab->adapterId()].append(tab->values());

        const int deviceId = tab->deviceId();
        if (deviceId >= 0)
        {
            /* No duplicate-id handling needed here, unlike the project file load path: tabs are
               built one per unique model device id, the id field is read-only, and handleAddTab()
               assigns max+1, so two tabs can never carry the same id. */
            Device device(static_cast<deviceId_t>(deviceId));
            device.setName(tab->deviceName());
            device.setAdapterId(tab->adapterId());
            devices.insert(static_cast<deviceId_t>(deviceId), device);
        }
    }

    /* Deliberately the adapter set captured when this page was built, not a fresh one: an
     * adapter that described while the dialog was open has no tab here, so writing it would
     * clear the very devices config it just brought in. */
    for (const auto& adapterId : std::as_const(_validAdapterIds))
    {
        QJsonObject config = _pSettingsModel->adapterData(adapterId)->effectiveConfig();
        config["devices"] = devicesByAdapter.value(adapterId);
        _pSettingsModel->setAdapterCurrentConfig(adapterId, config);
    }

    _pSettingsModel->applyDeviceList(devices);
}
