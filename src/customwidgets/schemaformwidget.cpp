#include "schemaformwidget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QJsonArray>
#include <QLineEdit>
#include <QSignalBlocker>
#include <QSpinBox>
#include <limits>
#include <utility>

/*!
 * \brief Constructs an empty form.
 * \param parent  Parent widget.
 */
SchemaFormWidget::SchemaFormWidget(QWidget* parent) : QWidget(parent), _pFormLayout(new QFormLayout(this))
{
}

/*!
 * \brief Populate the form from a JSON Schema object and current values.
 *
 * Reference options set earlier through setReferenceOptions() or setReferenceConfig() are kept and
 * offered to the new form.
 * \param schema  A JSON Schema with \c "type":"object" and a \c "properties" map.
 * \param values  Initial values matching the schema properties.
 */
void SchemaFormWidget::setSchema(const QJsonObject& schema, const QJsonObject& values)
{
    _schema = schema;
    _fields.clear();
    _referenceCombos.clear();
    _conditionalTriggerKey.clear();
    _conditionalTriggerConst.clear();
    _thenKeys.clear();
    _elseKeys.clear();
    _currentTriggerValue.clear();

    applyReferenceConfig();

    // removeRow() deletes both the label and field widget
    while (_pFormLayout->rowCount() > 0)
    {
        _pFormLayout->removeRow(0);
    }

    QJsonObject properties = schema.value("properties").toObject();

    // Determine a stable key order: propertyOrder array > required array > sorted keys
    QStringList orderedKeys;
    const QJsonArray propertyOrder = schema.value("propertyOrder").toArray();
    if (!propertyOrder.isEmpty())
    {
        for (const auto& v : propertyOrder)
        {
            const QString k = v.toString();
            if (properties.contains(k))
            {
                orderedKeys.append(k);
            }
        }
    }
    else
    {
        const QJsonArray required = schema.value("required").toArray();
        for (const auto& v : required)
        {
            const QString k = v.toString();
            if (properties.contains(k))
            {
                orderedKeys.append(k);
            }
        }
    }
    // Append any keys not yet listed (maintains determinism via sorted QJsonObject keys)
    for (auto it = properties.constBegin(); it != properties.constEnd(); ++it)
    {
        if (!orderedKeys.contains(it.key()))
        {
            orderedKeys.append(it.key());
        }
    }

    for (const auto& key : orderedKeys)
    {
        addFieldRow(key, properties.value(key).toObject(), values.value(key));
    }

    if (!parseConditional(schema))
    {
        return;
    }

    /* Every row added above starts visible, including these then/else fields, so
     * applyConditional() below must run before returning to hide whichever branch isn't active. */
    const QJsonObject thenProps = schema.value("then").toObject().value("properties").toObject();
    for (const QString& key : std::as_const(_thenKeys))
    {
        addFieldRow(key, thenProps.value(key).toObject(), values.value(key));
    }

    const QJsonObject elseProps = schema.value("else").toObject().value("properties").toObject();
    for (const QString& key : std::as_const(_elseKeys))
    {
        addFieldRow(key, elseProps.value(key).toObject(), values.value(key));
    }

    QComboBox* triggerCombo = nullptr;
    for (const auto& [key, widget] : std::as_const(_fields))
    {
        if (key == _conditionalTriggerKey)
        {
            triggerCombo = qobject_cast<QComboBox*>(widget);
            break;
        }
    }

    if (triggerCombo != nullptr)
    {
        applyConditional(triggerCombo->currentData().toString());
        connect(triggerCombo, &QComboBox::currentIndexChanged, this, &SchemaFormWidget::onTriggerChanged);
    }
    else
    {
        applyConditional(values.value(_conditionalTriggerKey).toVariant().toString());
    }
}

/*!
 * \brief Builds the widget for \a key, wires it up, and adds it as a row.
 *
 * Explicitly shows the new field widget and its label: a widget created under an
 * already-visible parent doesn't inherit visibility on its own until the next event loop
 * turn, so QFormLayout treats it as hidden and reports a QSize(0, 0) row until then, which
 * would throw off any immediate sizeHint() query against this form.
 * \param key        Schema property key, used to look up the field's current value later.
 * \param propSchema Schema fragment describing this single property.
 * \param value      Current value for the property.
 */
void SchemaFormWidget::addFieldRow(const QString& key, const QJsonObject& propSchema, const QJsonValue& value)
{
    QString label = propSchema.value("title").toString(key);
    QWidget* widget = createWidgetForProperty(propSchema, value);
    if (propSchema.value("readOnly").toBool())
    {
        widget->setEnabled(false);
    }
    wireFieldChanged(key, widget);
    _fields.append({ key, widget });
    _pFormLayout->addRow(label + ":", widget);

    widget->show();
    if (QWidget* fieldLabel = _pFormLayout->labelForField(widget))
    {
        fieldLabel->show();
    }
}

/*!
 * \brief Build options from the items of a config array.
 *
 * Items without a value for the value key are skipped; a missing label key falls back to the value.
 * \param items     Array of objects, e.g. the adapter's \c connections.
 * \param valueKey  Item key stored as the field value.
 * \param labelKey  Item key shown to the user.
 */
QList<SchemaFormWidget::ReferenceOption> SchemaFormWidget::optionsFromArray(const QJsonArray& items,
                                                                            const QString& valueKey,
                                                                            const QString& labelKey)
{
    QList<ReferenceOption> options;
    for (const auto& item : items)
    {
        const QJsonObject obj = item.toObject();
        if (obj.value(valueKey).isUndefined() || obj.value(valueKey).isNull())
        {
            continue;
        }

        ReferenceOption option;
        option.value = obj.value(valueKey);
        option.label = obj.value(labelKey).toVariant().toString();
        if (option.label.isEmpty())
        {
            option.label = option.value.toVariant().toString();
        }
        options.append(option);
    }
    return options;
}

/*!
 * \brief Build the options for every collection that \a schema references through \c x-ref.
 * \param schema  Object schema, including the properties of its if/then/else branches.
 * \param config  Config object holding the referenced arrays.
 * \return Options per collection name. If properties reference the same collection with
 *         different keys, the last one wins.
 */
QMap<QString, QList<SchemaFormWidget::ReferenceOption>> SchemaFormWidget::referenceOptionsForSchema(
  const QJsonObject& schema, const QJsonObject& config)
{
    QList<QJsonObject> propertyMaps;
    propertyMaps.append(schema.value("properties").toObject());
    propertyMaps.append(schema.value("then").toObject().value("properties").toObject());
    propertyMaps.append(schema.value("else").toObject().value("properties").toObject());

    QMap<QString, QList<ReferenceOption>> result;
    for (const QJsonObject& properties : std::as_const(propertyMaps))
    {
        for (auto it = properties.constBegin(); it != properties.constEnd(); ++it)
        {
            const QJsonObject xRef = it.value().toObject().value("x-ref").toObject();
            const QString collection = xRef.value("collection").toString();
            if (collection.isEmpty())
            {
                continue;
            }

            const QString valueKey = xRef.value("value").toString("id");
            const QString labelKey = xRef.value("label").toString("name");
            result.insert(collection, optionsFromArray(config.value(collection).toArray(), valueKey, labelKey));
        }
    }
    return result;
}

/*!
 * \brief Set the options offered by \c x-ref properties of \a collection.
 *
 * May be called before or after setSchema(). Existing reference combos for the collection
 * are repopulated and keep their selection by value.
 */
void SchemaFormWidget::setReferenceOptions(const QString& collection, const QList<ReferenceOption>& options)
{
    if (_references.contains(collection) && _references.value(collection) == options)
    {
        return;
    }
    _references.insert(collection, options);

    for (const ReferenceCombo& ref : std::as_const(_referenceCombos))
    {
        if (ref.collection == collection)
        {
            const QVariant before = ref.pCombo->currentData();
            fillReferenceCombo(ref, QJsonValue::fromVariant(before));
            if (ref.pCombo->currentData() != before)
            {
                // fillReferenceCombo() blocks the combo's signals, so report the moved selection here
                emit valuesChanged();
            }
            if (!_conditionalTriggerKey.isEmpty() && isTriggerWidget(ref.pCombo))
            {
                applyConditional(ref.pCombo->currentData().toString());
            }
        }
    }
}

/*!
 * \brief Set the config that holds the arrays referenced through \c x-ref.
 *
 * May be called before or after setSchema(). The options of every referenced collection are
 * rebuilt from \a config; combos whose options changed keep their selection by value.
 */
void SchemaFormWidget::setReferenceConfig(const QJsonObject& config)
{
    _referenceConfig = config;
    _hasReferenceConfig = true;
    applyReferenceConfig();
}

void SchemaFormWidget::applyReferenceConfig()
{
    // Without a config the options stay whatever setReferenceOptions() was given
    if (!_hasReferenceConfig)
    {
        return;
    }

    const QMap<QString, QList<ReferenceOption>> references = referenceOptionsForSchema(_schema, _referenceConfig);
    for (auto it = references.constBegin(); it != references.constEnd(); ++it)
    {
        setReferenceOptions(it.key(), it.value());
    }
}

QVariant SchemaFormWidget::comboKey(const QJsonValue& value, bool isInteger)
{
    if (!isInteger)
    {
        return value.isString() ? QVariant(value.toString()) : QVariant();
    }

    // Only whole numbers within int range are valid ids; anything else must not alias to 0
    if (!value.isDouble())
    {
        return QVariant();
    }
    const double number = value.toDouble();
    const int intValue = value.toInt();
    return (static_cast<double>(intValue) == number) ? QVariant(intValue) : QVariant();
}

bool SchemaFormWidget::isTriggerWidget(const QWidget* widget) const
{
    for (const auto& [key, field] : _fields)
    {
        if (field == widget)
        {
            return key == _conditionalTriggerKey;
        }
    }
    return false;
}

void SchemaFormWidget::populateCombo(QComboBox* pCombo, const QList<ReferenceOption>& items, bool isInteger)
{
    for (const ReferenceOption& item : items)
    {
        const QVariant key = comboKey(item.value, isInteger);
        if (key.isValid())
        {
            pCombo->addItem(item.label, key);
        }
    }
}

void SchemaFormWidget::fillReferenceCombo(const ReferenceCombo& ref, const QJsonValue& current)
{
    const QSignalBlocker blocker(ref.pCombo);

    ref.pCombo->clear();
    populateCombo(ref.pCombo, _references.value(ref.collection), ref.isInteger);

    if (current.isUndefined() || current.isNull())
    {
        return;
    }

    const QVariant currentVariant = comboKey(current, ref.isInteger);
    int idx = ref.pCombo->findData(currentVariant);
    if (idx < 0)
    {
        // Keep a dangling reference visible instead of silently changing the stored value
        ref.pCombo->addItem(tr("%1 (missing)").arg(current.toVariant().toString()), currentVariant);
        idx = ref.pCombo->count() - 1;
    }
    ref.pCombo->setCurrentIndex(idx);
}

QWidget* SchemaFormWidget::createWidgetForProperty(const QJsonObject& propSchema, const QJsonValue& value)
{
    QString type = propSchema.value("type").toString();
    const QJsonArray enumValues = propSchema.value("enum").toArray();
    const QJsonObject xRef = propSchema.value("x-ref").toObject();

    if (!xRef.isEmpty())
    {
        auto* combo = new QComboBox(this);
        const ReferenceCombo ref{ combo, xRef.value("collection").toString(), (type == "integer") };
        _referenceCombos.append(ref);
        fillReferenceCombo(ref, value);

        return combo;
    }
    else if (!enumValues.isEmpty())
    {
        auto* combo = new QComboBox(this);
        bool isInteger = (type == "integer");
        const QJsonArray enumLabels = propSchema.value("x-enumLabels").toArray();

        QList<ReferenceOption> items;
        for (int i = 0; i < enumValues.size(); ++i)
        {
            const QJsonValue& v = enumValues.at(i);
            QString label = (i < enumLabels.size()) ? enumLabels.at(i).toString() : QString();
            if (label.isEmpty())
            {
                label = v.toVariant().toString();
            }
            items.append({ v, label });
        }
        populateCombo(combo, items, isInteger);

        int idx = combo->findData(comboKey(value, isInteger));
        const QJsonValue defaultJson = propSchema.value("default");
        if (idx < 0 && !defaultJson.isUndefined() && !defaultJson.isNull())
        {
            idx = combo->findData(comboKey(defaultJson, isInteger));
        }
        if (idx >= 0)
        {
            combo->setCurrentIndex(idx);
        }

        return combo;
    }
    else if (type == "boolean")
    {
        auto* check = new QCheckBox(this);
        check->setChecked(value.toBool());
        return check;
    }
    else if (type == "integer")
    {
        auto* spin = new QSpinBox(this);
        QJsonValue minJson = propSchema.value("minimum");
        QJsonValue maxJson = propSchema.value("maximum");
        int minVal = !minJson.isUndefined() ? minJson.toInt() : std::numeric_limits<int>::min();
        int maxVal = !maxJson.isUndefined() ? maxJson.toInt() : std::numeric_limits<int>::max();
        spin->setRange(minVal, maxVal);
        spin->setValue(value.toInt(0));

        return spin;
    }
    else if (type == "number")
    {
        auto* spin = new QDoubleSpinBox(this);
        QJsonValue minJson = propSchema.value("minimum");
        QJsonValue maxJson = propSchema.value("maximum");
        double minVal = !minJson.isUndefined() ? minJson.toDouble() : std::numeric_limits<double>::lowest();
        double maxVal = !maxJson.isUndefined() ? maxJson.toDouble() : std::numeric_limits<double>::max();
        spin->setRange(minVal, maxVal);
        spin->setValue(value.toDouble(0.0));

        return spin;
    }
    else // "string" or unknown
    {
        auto* edit = new QLineEdit(this);
        edit->setText(value.toString());

        return edit;
    }
}

void SchemaFormWidget::wireFieldChanged(const QString& key, QWidget* widget)
{
    if (auto* edit = qobject_cast<QLineEdit*>(widget))
    {
        connect(edit, &QLineEdit::textChanged, this, &SchemaFormWidget::valuesChanged);
        connect(edit, &QLineEdit::textChanged, this,
                [this, key](const QString& text) { emit fieldChanged(key, text); });
    }
    else if (auto* spin = qobject_cast<QSpinBox*>(widget))
    {
        connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SchemaFormWidget::valuesChanged);
    }
    else if (auto* dspin = qobject_cast<QDoubleSpinBox*>(widget))
    {
        connect(dspin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SchemaFormWidget::valuesChanged);
    }
    else if (auto* check = qobject_cast<QCheckBox*>(widget))
    {
        connect(check, &QCheckBox::toggled, this, &SchemaFormWidget::valuesChanged);
    }
    else if (auto* combo = qobject_cast<QComboBox*>(widget))
    {
        connect(combo, &QComboBox::currentIndexChanged, this, &SchemaFormWidget::valuesChanged);
    }
}

bool SchemaFormWidget::parseConditional(const QJsonObject& schema)
{
    const QJsonObject ifObj = schema.value("if").toObject();
    const QJsonObject thenObj = schema.value("then").toObject();
    const QJsonObject elseObj = schema.value("else").toObject();

    if (ifObj.isEmpty() || thenObj.isEmpty() || elseObj.isEmpty())
    {
        return false;
    }

    const QJsonObject ifProperties = ifObj.value("properties").toObject();

    QString triggerKey;
    const QJsonArray required = ifObj.value("required").toArray();
    if (required.size() == 1)
    {
        triggerKey = required.at(0).toString();
    }
    else if (ifProperties.size() == 1)
    {
        triggerKey = ifProperties.constBegin().key();
    }

    if (triggerKey.isEmpty())
    {
        return false;
    }

    const QJsonObject constObj = ifProperties.value(triggerKey).toObject();
    if (!constObj.contains("const"))
    {
        return false;
    }

    _conditionalTriggerKey = triggerKey;
    _conditionalTriggerConst = constObj.value("const").toVariant().toString();

    const QJsonObject thenProps = thenObj.value("properties").toObject();
    for (auto it = thenProps.constBegin(); it != thenProps.constEnd(); ++it)
    {
        _thenKeys.append(it.key());
    }

    const QJsonObject elseProps = elseObj.value("properties").toObject();
    for (auto it = elseProps.constBegin(); it != elseProps.constEnd(); ++it)
    {
        _elseKeys.append(it.key());
    }

    return true;
}

void SchemaFormWidget::applyConditional(const QString& triggerValue)
{
    _currentTriggerValue = triggerValue;
    const bool conditionMet = (triggerValue == _conditionalTriggerConst);

    for (const auto& [key, widget] : std::as_const(_fields))
    {
        const bool isThen = _thenKeys.contains(key);
        const bool isElse = _elseKeys.contains(key);

        if (!isThen && !isElse)
        {
            continue;
        }

        const bool visible = isThen ? conditionMet : !conditionMet;
        widget->setVisible(visible);

        QWidget* label = _pFormLayout->labelForField(widget);
        if (label != nullptr)
        {
            label->setVisible(visible);
        }
    }
}

void SchemaFormWidget::onTriggerChanged(int /*index*/)
{
    auto* combo = qobject_cast<QComboBox*>(sender());
    if (combo == nullptr)
    {
        return;
    }

    applyConditional(combo->currentData().toString());

    // The generic change notification of the trigger combo fired before the branch switched
    emit valuesChanged();
}

/*!
 * \brief Return current form input as a JSON object.
 *
 * Fields belonging to the inactive conditional branch are omitted.
 * \return A QJsonObject with one entry per visible schema property.
 */
QJsonObject SchemaFormWidget::values() const
{
    const bool conditionMet = !_conditionalTriggerKey.isEmpty() && (_currentTriggerValue == _conditionalTriggerConst);

    QJsonObject result;
    for (const auto& [key, widget] : _fields)
    {
        const bool isThen = _thenKeys.contains(key);
        const bool isElse = _elseKeys.contains(key);

        if (isThen && !conditionMet)
        {
            continue;
        }
        if (isElse && conditionMet)
        {
            continue;
        }

        if (auto* spin = qobject_cast<QSpinBox*>(widget))
        {
            result.insert(key, spin->value());
        }
        else if (auto* dspin = qobject_cast<QDoubleSpinBox*>(widget))
        {
            result.insert(key, dspin->value());
        }
        else if (auto* check = qobject_cast<QCheckBox*>(widget))
        {
            result.insert(key, check->isChecked());
        }
        else if (auto* combo = qobject_cast<QComboBox*>(widget))
        {
            QVariant data = combo->currentData();
            if (!data.isValid())
            {
                // Empty combo (e.g. no referenced items yet): leave the key out instead of writing ""
                continue;
            }

            result.insert(key, QJsonValue::fromVariant(data));
        }
        else if (auto* edit = qobject_cast<QLineEdit*>(widget))
        {
            result.insert(key, edit->text());
        }
    }
    return result;
}
