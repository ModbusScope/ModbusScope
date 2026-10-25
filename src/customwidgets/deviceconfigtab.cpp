#include "deviceconfigtab.h"

#include "customwidgets/schemaformwidget.h"
#include "models/adapterconfigdraft.h"
#include "models/adapterdata.h"
#include "models/settingsmodel.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QVBoxLayout>

/*!
 * \brief Constructs the tab for one device.
 * \param pSettingsModel  Model providing the adapters and their schemas.
 * \param adapterId       Adapter to preselect.
 * \param deviceName      Name shown in the name field.
 * \param deviceValues    Initial values of the schema form.
 * \param parent          Parent widget.
 * \param pDraft          Config the \c x-ref fields read their options from, shared with the other settings pages.
 *                       When null the tab uses a private draft that only follows the model.
 */
DeviceConfigTab::DeviceConfigTab(SettingsModel* pSettingsModel,
                                 const QString& adapterId,
                                 const QString& deviceName,
                                 const QJsonObject& deviceValues,
                                 QWidget* parent,
                                 AdapterConfigDraft* pDraft)
    : QWidget(parent),
      _pLayout(new QVBoxLayout),
      _pNameEdit(new QLineEdit(this)),
      _pAdapterCombo(new QComboBox(this)),
      _pSchemaForm(nullptr),
      _pSettingsModel(pSettingsModel),
      _deviceId(deviceValues.value("id").toInt(-1)),
      _pDraft(pDraft != nullptr ? pDraft : new AdapterConfigDraft(pSettingsModel, this))
{
    setLayout(_pLayout);

    auto* nameRow = new QHBoxLayout;
    nameRow->addWidget(new QLabel("Name:", this));
    nameRow->addWidget(_pNameEdit);
    nameRow->addStretch();
    _pLayout->addLayout(nameRow);

    _pNameEdit->setText(deviceName);

    auto* adapterRow = new QHBoxLayout;
    adapterRow->addWidget(new QLabel("Protocol adapter:", this));
    adapterRow->addWidget(_pAdapterCombo);
    adapterRow->addStretch();
    _pLayout->addLayout(adapterRow);

    // Add stretch at the end so rebuildSchemaForm can insert before it
    _pLayout->addStretch();

    QStringList deviceAdapterIds;
    QMap<QString, int> nameCounts;
    const QStringList adapterIds = pSettingsModel->adapterIds();
    for (const auto& id : adapterIds)
    {
        const AdapterData* pAdapter = pSettingsModel->adapterData(id);
        QJsonObject itemSchema =
          pAdapter->schema().value("properties").toObject().value("devices").toObject().value("items").toObject();
        if (!itemSchema.isEmpty())
        {
            deviceAdapterIds << id;
            nameCounts[pSettingsModel->adapterDisplayName(id)]++;
        }
    }

    // Protocol names are not guaranteed unique, so append the adapter ID to names that collide
    for (const auto& id : std::as_const(deviceAdapterIds))
    {
        const QString name = pSettingsModel->adapterDisplayName(id);
        const QString label = nameCounts.value(name) > 1 ? QString("%1 (%2)").arg(name, id) : name;
        _pAdapterCombo->addItem(label, id);
    }

    int idx = _pAdapterCombo->findData(adapterId);
    if (idx >= 0)
    {
        _pAdapterCombo->setCurrentIndex(idx);
    }

    connect(_pNameEdit, &QLineEdit::textChanged, this, &DeviceConfigTab::onNameChanged);
    connect(_pAdapterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DeviceConfigTab::onAdapterChanged);
    connect(_pDraft, &AdapterConfigDraft::configChanged, this, &DeviceConfigTab::onDraftConfigChanged);

    QString effectiveAdapterId = _pAdapterCombo->currentData().toString();
    if (!effectiveAdapterId.isEmpty())
    {
        rebuildSchemaForm(effectiveAdapterId, deviceValues);
    }
}

void DeviceConfigTab::onAdapterChanged(int index)
{
    QString newAdapterId = _pAdapterCombo->itemData(index).toString();
    QJsonObject defaultValues;
    const QJsonArray defaultDevices = _pSettingsModel->adapterData(newAdapterId)->defaults().value("devices").toArray();
    if (!defaultDevices.isEmpty())
    {
        defaultValues = defaultDevices.first().toObject();
    }

    defaultValues.insert("id", deviceId());

    rebuildSchemaForm(newAdapterId, defaultValues);
    emit adapterChanged(newAdapterId);
}

void DeviceConfigTab::onDraftConfigChanged(const QString& adapterId)
{
    if (_pSchemaForm != nullptr && adapterId == this->adapterId())
    {
        _pSchemaForm->setReferenceConfig(_pDraft->config(adapterId));
    }
}

void DeviceConfigTab::onNameChanged(const QString& name)
{
    emit nameChanged(name);
}

void DeviceConfigTab::rebuildSchemaForm(const QString& adapterId, const QJsonObject& deviceValues)
{
    if (_pSchemaForm)
    {
        _pLayout->removeWidget(_pSchemaForm);
        _pSchemaForm->deleteLater();
        _pSchemaForm = nullptr;
    }

    const AdapterData* pAdapter = _pSettingsModel->adapterData(adapterId);
    QJsonObject devicesSchema = pAdapter->schema().value("properties").toObject().value("devices").toObject();
    QJsonObject itemSchema = devicesSchema.value("items").toObject();

    // The "id" field is read-only: a device ID is assigned once at creation and is never edited, because it is both
    // the SettingsModel device map key and the "id" in the adapter config, and both must stay identical.
    QJsonObject propsObj = itemSchema.value("properties").toObject();
    QJsonObject idProp = propsObj.value("id").toObject();
    idProp.insert("readOnly", true);
    propsObj.insert("id", idProp);
    itemSchema.insert("properties", propsObj);

    _pSchemaForm = new SchemaFormWidget(this);
    _pSchemaForm->setReferenceConfig(_pDraft->config(adapterId));
    _pSchemaForm->setSchema(itemSchema, deviceValues);

    _pLayout->insertWidget(_pLayout->count() - 1, _pSchemaForm);
}

/*!
 * \brief Return the current device values from the schema form.
 * \return Device field values as a JSON object.
 */
QJsonObject DeviceConfigTab::values() const
{
    QJsonObject result = _pSchemaForm ? _pSchemaForm->values() : QJsonObject();

    /* The "id" field is always readOnly (see rebuildSchemaForm()), so its schema-bound widget
     * value must never be trusted: a missing id is clamped to the widget's minimum by Qt,
     * silently fabricating a plausible-looking id instead of preserving the invalid sentinel. */
    result.insert("id", _deviceId);
    return result;
}

/*!
 * \brief Return the currently selected adapter ID.
 * \return Adapter identifier string.
 */
QString DeviceConfigTab::adapterId() const
{
    return _pAdapterCombo->currentData().toString();
}

/*!
 * \brief Return the current device name from the name field.
 * \return Device name string (may be empty).
 */
QString DeviceConfigTab::deviceName() const
{
    return _pNameEdit->text();
}

/*!
 * \brief Return the device ID assigned to this tab, or -1 if none.
 * \return Device ID as int.
 */
int DeviceConfigTab::deviceId() const
{
    return _deviceId;
}
