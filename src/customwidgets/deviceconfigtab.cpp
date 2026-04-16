#include "deviceconfigtab.h"

#include "customwidgets/schemaformwidget.h"
#include "models/adapterdata.h"
#include "models/device.h"
#include "models/settingsmodel.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

DeviceConfigTab::DeviceConfigTab(SettingsModel* pSettingsModel,
                                 const QString& adapterId,
                                 const QJsonObject& deviceValues,
                                 QWidget* parent)
    : QWidget(parent),
      _pLayout(nullptr),
      _pNameEdit(new QLineEdit(this)),
      _pAdapterCombo(new QComboBox(this)),
      _pSchemaForm(nullptr),
      _pSettingsModel(pSettingsModel),
      _deviceId(deviceValues.value("id").toInt(-1))
{
    _pLayout = new QVBoxLayout(this);
    setLayout(_pLayout);

    auto* nameRow = new QHBoxLayout;
    nameRow->addWidget(new QLabel("Name:", this));
    nameRow->addWidget(_pNameEdit);
    nameRow->addStretch();
    _pLayout->addLayout(nameRow);

    int deviceId = deviceValues.value("id").toInt(-1);
    if (deviceId >= 0 && pSettingsModel->deviceList().contains(static_cast<deviceId_t>(deviceId)))
    {
        _pNameEdit->setText(pSettingsModel->deviceSettings(static_cast<deviceId_t>(deviceId))->name());
    }

    auto* adapterRow = new QHBoxLayout;
    adapterRow->addWidget(new QLabel("Protocol adapter:", this));
    adapterRow->addWidget(_pAdapterCombo);
    adapterRow->addStretch();
    _pLayout->addLayout(adapterRow);

    // Add stretch at the end so rebuildSchemaForm can insert before it
    _pLayout->addStretch();

    const QStringList adapterIds = pSettingsModel->adapterIds();
    for (const auto& id : adapterIds)
    {
        const AdapterData* pAdapter = pSettingsModel->adapterData(id);
        QJsonObject itemSchema =
          pAdapter->schema().value("properties").toObject().value("devices").toObject().value("items").toObject();
        if (!itemSchema.isEmpty())
        {
            _pAdapterCombo->addItem(id, id);
        }
    }

    int idx = _pAdapterCombo->findData(adapterId);
    if (idx >= 0)
    {
        _pAdapterCombo->setCurrentIndex(idx);
    }

    connect(_pAdapterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DeviceConfigTab::onAdapterChanged);

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

    // Preserve the existing device id so that switching adapters does not overwrite
    // the unique id assigned when the tab was created. Fall back to _deviceId when
    // the id field is hidden in the schema form and not returned by values().
    int currentId = _pSchemaForm ? _pSchemaForm->values().value("id").toInt(-1) : -1;
    if (currentId < 0)
    {
        currentId = _deviceId;
    }
    defaultValues["id"] = currentId;

    rebuildSchemaForm(newAdapterId, defaultValues);
}

void DeviceConfigTab::rebuildSchemaForm(const QString& adapterId, const QJsonObject& deviceValues)
{
    if (_pSchemaForm)
    {
        _pLayout->removeWidget(_pSchemaForm);
        delete _pSchemaForm;
        _pSchemaForm = nullptr;
    }

    const AdapterData* pAdapter = _pSettingsModel->adapterData(adapterId);
    QJsonObject devicesSchema = pAdapter->schema().value("properties").toObject().value("devices").toObject();
    QJsonObject itemSchema = devicesSchema.value("items").toObject();

    _pSchemaForm = new SchemaFormWidget(this);
    _pSchemaForm->setSchema(itemSchema, deviceValues);

    // Insert before the trailing stretch (last item)
    _pLayout->insertWidget(_pLayout->count() - 1, _pSchemaForm);
}

QJsonObject DeviceConfigTab::values() const
{
    if (_pSchemaForm)
    {
        return _pSchemaForm->values();
    }
    return QJsonObject();
}

QString DeviceConfigTab::adapterId() const
{
    return _pAdapterCombo->currentData().toString();
}

QString DeviceConfigTab::deviceName() const
{
    return _pNameEdit->text();
}
