#include "adaptersettings.h"

#include "customwidgets/addabletabwidget.h"
#include "customwidgets/schemaformwidget.h"
#include "models/adapterdata.h"
#include "models/settingsmodel.h"

#include <QJsonArray>
#include <QVBoxLayout>

AdapterSettings::AdapterSettings(SettingsModel* pSettingsModel,
                                 const QString& adapterId,
                                 const QString& propertyKey,
                                 QWidget* parent)
    : QWidget(parent), _pSettingsModel(pSettingsModel), _adapterId(adapterId), _propertyKey(propertyKey)
{
    auto* layout = new QVBoxLayout(this);

    const AdapterData* pAdapter = pSettingsModel->adapterData(adapterId);
    const QJsonObject propSchema = pAdapter->schema().value("properties").toObject().value(propertyKey).toObject();
    const QJsonValue configValue = pAdapter->effectiveConfig().value(propertyKey);

    buildSection(propSchema, configValue, layout);
}

bool AdapterSettings::isRenderableProperty(const QJsonObject& propSchema)
{
    const QString type = propSchema.value("type").toString();
    if (type == "array")
    {
        return !propSchema.value("items").toObject().isEmpty();
    }
    if (type == "object")
    {
        return !propSchema.value("properties").toObject().isEmpty();
    }
    return false;
}

void AdapterSettings::buildSection(const QJsonObject& propSchema, const QJsonValue& configValue, QVBoxLayout* layout)
{
    const QString type = propSchema.value("type").toString();

    if (type == "array")
    {
        _itemSchema = propSchema.value("items").toObject();
        _pItemTabs = new AddableTabWidget(this);

        const QJsonArray itemsArray = configValue.toArray();
        QList<QWidget*> pages;
        QStringList names;
        for (int i = 0; i < itemsArray.size(); ++i)
        {
            auto* form = new SchemaFormWidget(_pItemTabs);
            form->setSchema(_itemSchema, itemsArray.at(i).toObject());
            connectTabNameTracking(form);
            pages.append(form);
            const QString itemName = itemsArray.at(i).toObject().value("name").toString();
            names.append(itemName.isEmpty() ? formatTabName(i + 1) : itemName);
        }

        if (!pages.isEmpty())
        {
            _pItemTabs->setTabs(pages, names);
        }

        _nextItemTabIndex = itemsArray.size() + 1;

        connect(_pItemTabs, &AddableTabWidget::addTabRequested, this, &AdapterSettings::addItemTab);

        layout->addWidget(_pItemTabs, 1);
    }
    else
    {
        _pForm = new SchemaFormWidget(this);
        _pForm->setSchema(propSchema, configValue.toObject());
        layout->addWidget(_pForm);
        layout->addStretch();
    }
}

QString AdapterSettings::formatTabName(int index) const
{
    if (_propertyKey.isEmpty())
    {
        return QString("Item %1").arg(index);
    }
    QString label = (_propertyKey.length() > 1 && _propertyKey.endsWith('s')) ? _propertyKey.chopped(1) : _propertyKey;
    if (label.isEmpty())
    {
        return QString("Item %1").arg(index);
    }
    return QString("%1 %2").arg(label[0].toUpper() + label.mid(1)).arg(index);
}

void AdapterSettings::onSchemaFieldNameChanged(SchemaFormWidget* form, const QString& key, const QString& value)
{
    if (key == "name")
    {
        const int tabIndex = _pItemTabs->indexOf(form);
        if (tabIndex >= 0)
        {
            _pItemTabs->setTabName(tabIndex, value.isEmpty() ? formatTabName(tabIndex + 1) : value);
        }
    }
}

void AdapterSettings::connectTabNameTracking(SchemaFormWidget* form)
{
    connect(form, &SchemaFormWidget::fieldChanged, this,
            [this, form](const QString& key, const QString& value) { onSchemaFieldNameChanged(form, key, value); });
}

void AdapterSettings::addItemTab()
{
    auto* form = new SchemaFormWidget(_pItemTabs);
    QJsonObject defaultValues;
    const QJsonArray defaultItems = _pSettingsModel->adapterData(_adapterId)->defaults().value(_propertyKey).toArray();
    if (!defaultItems.isEmpty())
    {
        defaultValues = defaultItems.first().toObject();
    }

    const QJsonObject properties = _itemSchema.value("properties").toObject();

    int nameIndex = _nextItemTabIndex;

    const QJsonObject idProp = properties.value("id").toObject();
    if (!idProp.isEmpty() && idProp.value("type").toString() == "integer")
    {
        int maxId = 0;
        for (int i = 0; i < _pItemTabs->count(); ++i)
        {
            auto* existingForm = qobject_cast<SchemaFormWidget*>(_pItemTabs->tabContent(i));
            if (existingForm)
            {
                maxId = qMax(maxId, existingForm->values().value("id").toInt());
            }
        }
        defaultValues["id"] = maxId + 1;
        nameIndex = maxId + 1;
    }

    const QJsonObject nameProp = properties.value("name").toObject();
    if (!nameProp.isEmpty() && nameProp.value("type").toString() == "string")
    {
        defaultValues["name"] = formatTabName(nameIndex);
    }

    form->setSchema(_itemSchema, defaultValues);
    connectTabNameTracking(form);
    const QString tabName = defaultValues.value("name").toString();
    const QString name = tabName.isEmpty() ? formatTabName(nameIndex) : tabName;
    _nextItemTabIndex++;
    _pItemTabs->addNewTab(name, form);
}

void AdapterSettings::acceptValues()
{
    QJsonObject config = _pSettingsModel->adapterData(_adapterId)->effectiveConfig();

    if (_pItemTabs)
    {
        QJsonArray itemsArray;
        for (int i = 0; i < _pItemTabs->count(); ++i)
        {
            auto* form = qobject_cast<SchemaFormWidget*>(_pItemTabs->tabContent(i));
            if (form)
            {
                itemsArray.append(form->values());
            }
        }
        config[_propertyKey] = itemsArray;
    }
    else if (_pForm)
    {
        config[_propertyKey] = _pForm->values();
    }

    _pSettingsModel->setAdapterCurrentConfig(_adapterId, config);
}
