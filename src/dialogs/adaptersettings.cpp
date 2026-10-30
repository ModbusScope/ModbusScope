#include "adaptersettings.h"

#include "customwidgets/addabletabwidget.h"
#include "customwidgets/schemaformwidget.h"
#include "models/adapterconfigdraft.h"
#include "models/adapterdata.h"
#include "models/settingsmodel.h"

#include <QJsonArray>
#include <QVBoxLayout>

/*!
 * \brief Constructs the page for one property of an adapter's schema.
 * \param pSettingsModel  Model providing the adapter's schema and saved config.
 * \param adapterId       The adapter this page belongs to.
 * \param propertyKey     Top-level schema property this page edits.
 * \param parent          Parent widget.
 * \param pDraft          Draft this page publishes its values to and reads \c x-ref options from. When null
 *                       the page uses a private draft.
 */
AdapterSettings::AdapterSettings(SettingsModel* pSettingsModel,
                                 const QString& adapterId,
                                 const QString& propertyKey,
                                 QWidget* parent,
                                 AdapterConfigDraft* pDraft)
    : QWidget(parent),
      _pSettingsModel(pSettingsModel),
      _adapterId(adapterId),
      _propertyKey(propertyKey),
      _pDraft(pDraft != nullptr ? pDraft : new AdapterConfigDraft(pSettingsModel, this))
{
    auto* layout = new QVBoxLayout(this);

    const AdapterData* pAdapter = pSettingsModel->adapterData(adapterId);
    const QJsonObject propSchema = pAdapter->schema().value("properties").toObject().value(propertyKey).toObject();
    const QJsonValue configValue = pAdapter->effectiveConfig().value(propertyKey);

    buildSection(propSchema, configValue, layout);

    connect(_pDraft, &AdapterConfigDraft::configChanged, this, &AdapterSettings::refreshReferenceOptions);
}

/*!
 * \brief Returns true if the schema property produces a non-empty page.
 *
 * \c "type":"array" requires non-empty items schema.
 * \c "type":"object" requires non-empty properties.
 * Scalar types always return false.
 */
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

        // An integer "id" is assigned by addItemTab() and referenced by other items, so the user must not edit it.
        QJsonObject itemProps = _itemSchema.value("properties").toObject();
        QJsonObject idProp = itemProps.value("id").toObject();
        if (idProp.value("type").toString() == "integer")
        {
            idProp.insert("readOnly", true);
            itemProps.insert("id", idProp);
            _itemSchema.insert("properties", itemProps);
        }

        _pItemTabs = new AddableTabWidget(this);

        // A previous accept may have stripped an inactive if/then/else branch's fields
        // from a stored item (see SchemaFormWidget::values()); backfill them here so
        // switching back to that branch shows the adapter's defaults instead of blanks.
        const QJsonObject defaultItem =
          _pSettingsModel->adapterData(_adapterId)->defaults().value(_propertyKey).toArray().at(0).toObject();

        const QJsonArray itemsArray = configValue.toArray();
        QList<QWidget*> pages;
        QStringList names;
        for (int i = 0; i < itemsArray.size(); ++i)
        {
            QJsonObject itemValues = defaultItem;
            const QJsonObject storedValues = itemsArray.at(i).toObject();
            for (auto it = storedValues.constBegin(); it != storedValues.constEnd(); ++it)
            {
                itemValues.insert(it.key(), it.value());
            }

            auto* form = new SchemaFormWidget(_pItemTabs);
            form->setReferenceConfig(_pDraft->config(_adapterId));
            form->setSchema(_itemSchema, itemValues);
            connectForm(form);
            pages.append(form);
            const QString itemName = storedValues.value("name").toString();
            names.append(itemName.isEmpty() ? formatTabName(i + 1) : itemName);
        }

        if (!pages.isEmpty())
        {
            _pItemTabs->setTabs(pages, names);
        }

        _nextItemTabIndex = itemsArray.size() + 1;

        connect(_pItemTabs, &AddableTabWidget::addTabRequested, this, &AdapterSettings::addItemTab);
        connect(_pItemTabs, &AddableTabWidget::tabClosed, this, &AdapterSettings::publishSection);

        layout->addWidget(_pItemTabs, 1);
    }
    else
    {
        _pForm = new SchemaFormWidget(this);
        _pForm->setReferenceConfig(_pDraft->config(_adapterId));
        _pForm->setSchema(propSchema, configValue.toObject());
        connectForm(_pForm);
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

/*!
 * \brief Feeds every item form the options of the collections its schema references via \c x-ref.
 *
 * Options come from the draft, so they include what other pages are editing.
 */
void AdapterSettings::refreshReferenceOptions(const QString& adapterId)
{
    if (_pItemTabs == nullptr || adapterId != _adapterId)
    {
        return;
    }

    const QJsonObject config = _pDraft->config(_adapterId);
    for (int i = 0; i < _pItemTabs->count(); ++i)
    {
        auto* form = qobject_cast<SchemaFormWidget*>(_pItemTabs->tabContent(i));
        if (form)
        {
            form->setReferenceConfig(config);
        }
    }
}

void AdapterSettings::connectForm(SchemaFormWidget* form)
{
    connect(form, &SchemaFormWidget::valuesChanged, this, &AdapterSettings::publishSection);
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

    form->setReferenceConfig(_pDraft->config(_adapterId));
    form->setSchema(_itemSchema, defaultValues);
    connectForm(form);
    const QString tabName = defaultValues.value("name").toString();
    const QString name = tabName.isEmpty() ? formatTabName(nameIndex) : tabName;
    _nextItemTabIndex++;
    _pItemTabs->addNewTab(name, form);
    publishSection();
}

QJsonValue AdapterSettings::sectionValue() const
{
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
        return itemsArray;
    }

    return _pForm ? QJsonValue(_pForm->values()) : QJsonValue();
}

void AdapterSettings::publishSection()
{
    _pDraft->setSection(_adapterId, _propertyKey, sectionValue());
}

/*!
 * \brief Write the current form values back to the adapter's configuration.
 */
void AdapterSettings::acceptValues()
{
    QJsonObject config = _pSettingsModel->adapterData(_adapterId)->effectiveConfig();
    config[_propertyKey] = sectionValue();
    _pSettingsModel->setAdapterCurrentConfig(_adapterId, config);
}
