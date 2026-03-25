#include "deviceconfigtab.h"

#include "customwidgets/schemaformwidget.h"
#include "models/adapterdata.h"
#include "models/settingsmodel.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

DeviceConfigTab::DeviceConfigTab(SettingsModel* pSettingsModel,
                                 const QString& adapterId,
                                 const QJsonObject& deviceValues,
                                 QWidget* parent)
    : QWidget(parent),
      _pLayout(nullptr),
      _pAdapterCombo(new QComboBox(this)),
      _pSchemaForm(nullptr),
      _pSettingsModel(pSettingsModel)
{
    _pLayout = new QVBoxLayout(this);
    setLayout(_pLayout);

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
    rebuildSchemaForm(newAdapterId, QJsonObject());
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
