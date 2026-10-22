#include "devicecombobox.h"

#include "models/settingsmodel.h"

#include <QSignalBlocker>

/*!
 * \brief Constructs an empty device selector; setSettingsModel() fills it.
 * \param parent Optional parent widget.
 */
DeviceComboBox::DeviceComboBox(QWidget* parent) : QComboBox(parent)
{
    connect(this, &QComboBox::currentIndexChanged, this, &DeviceComboBox::onCurrentIndexChanged);
}

/*!
 * \brief Fill the selector with the configured devices and follow changes to the device list.
 *
 * The first device is selected. No deviceChanged() is emitted for this initial selection.
 * \param pSettingsModel Settings model that holds the devices.
 */
void DeviceComboBox::setSettingsModel(SettingsModel* pSettingsModel)
{
    if (_pSettingsModel != nullptr)
    {
        disconnect(_pSettingsModel, nullptr, this, nullptr);
    }

    _pSettingsModel = pSettingsModel;
    connect(_pSettingsModel, &SettingsModel::deviceListChanged, this, &DeviceComboBox::onDeviceListChanged);

    const QSignalBlocker blocker(this);
    populate();
}

/*!
 * \brief Set how devices are labelled: by name only, or by name, protocol and id.
 * \param style The label style.
 */
void DeviceComboBox::setLabelStyle(LabelStyle style)
{
    _labelStyle = style;

    if (_pSettingsModel == nullptr)
    {
        return;
    }

    for (int idx = 0; idx < count(); idx++)
    {
        setItemText(idx, deviceLabel(static_cast<deviceId_t>(itemData(idx).toUInt())));
    }
}

/*!
 * \brief Returns the id of the selected device, or 0 when there are no devices.
 */
deviceId_t DeviceComboBox::selectedDeviceId() const
{
    return static_cast<deviceId_t>(currentData().toUInt());
}

/*!
 * \brief Select a device. Emits deviceChanged() when the selection changes.
 *
 * Does nothing when the device is not in the list.
 * \param deviceId The device to select.
 */
void DeviceComboBox::setSelectedDeviceId(deviceId_t deviceId)
{
    const int idx = findData(QVariant(deviceId));
    if (idx >= 0)
    {
        setCurrentIndex(idx);
    }
}

void DeviceComboBox::onCurrentIndexChanged(int index)
{
    Q_UNUSED(index);
    emit deviceChanged(selectedDeviceId());
}

/*!
 * \brief Rebuild the list and keep the selected device.
 *
 * When the selected device is gone, the first device is selected and deviceChanged() is emitted
 * once; with no devices left, deviceChanged(0) is emitted.
 */
void DeviceComboBox::onDeviceListChanged()
{
    const deviceId_t previousSelection = selectedDeviceId();

    {
        const QSignalBlocker blocker(this);
        populate();

        const int idx = findData(QVariant(previousSelection));
        setCurrentIndex(idx >= 0 ? idx : 0);
    }

    if (selectedDeviceId() != previousSelection)
    {
        emit deviceChanged(selectedDeviceId());
    }
}

void DeviceComboBox::populate()
{
    clear();

    const QList<deviceId_t> deviceIds = _pSettingsModel->deviceList();
    for (deviceId_t deviceId : deviceIds)
    {
        addItem(deviceLabel(deviceId), QVariant(deviceId));
    }
}

QString DeviceComboBox::deviceLabel(deviceId_t deviceId) const
{
    Device* pDevice = _pSettingsModel->deviceSettings(deviceId);
    if (_labelStyle == LabelStyle::Name)
    {
        return pDevice->name();
    }

    const QString protocolLabel = _pSettingsModel->adapterDisplayName(pDevice->adapterId());
    return QStringLiteral("%1 (%2) [%3]").arg(pDevice->name(), protocolLabel, QString::number(deviceId));
}
