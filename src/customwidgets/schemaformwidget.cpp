#include "schemaformwidget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QJsonArray>
#include <QLineEdit>
#include <QSpinBox>
#include <climits>
#include <limits>
#include <utility>

SchemaFormWidget::SchemaFormWidget(QWidget* parent) : QWidget(parent), _pFormLayout(new QFormLayout(this))
{
    setLayout(_pFormLayout);
}

void SchemaFormWidget::setSchema(const QJsonObject& schema, const QJsonObject& values)
{
    _fields.clear();
    _conditionalTriggerKey.clear();
    _conditionalTriggerConst.clear();
    _thenKeys.clear();
    _elseKeys.clear();
    _currentTriggerValue.clear();

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
        QJsonObject propSchema = properties.value(key).toObject();
        QJsonValue currentValue = values.value(key);

        QString label = propSchema.value("title").toString(key);
        QWidget* widget = createWidgetForProperty(propSchema, currentValue);
        _fields.append({ key, widget });
        _pFormLayout->addRow(label + ":", widget);
    }

    if (!parseConditional(schema))
    {
        return;
    }

    const QJsonObject thenProps = schema.value("then").toObject().value("properties").toObject();
    for (const QString& key : std::as_const(_thenKeys))
    {
        QJsonObject propSchema = thenProps.value(key).toObject();
        QString label = propSchema.value("title").toString(key);
        QWidget* widget = createWidgetForProperty(propSchema, values.value(key));
        _fields.append({ key, widget });
        _pFormLayout->addRow(label + ":", widget);
    }

    const QJsonObject elseProps = schema.value("else").toObject().value("properties").toObject();
    for (const QString& key : std::as_const(_elseKeys))
    {
        QJsonObject propSchema = elseProps.value(key).toObject();
        QString label = propSchema.value("title").toString(key);
        QWidget* widget = createWidgetForProperty(propSchema, values.value(key));
        _fields.append({ key, widget });
        _pFormLayout->addRow(label + ":", widget);
    }

    applyConditional(values.value(_conditionalTriggerKey).toVariant().toString());

    for (const auto& [key, widget] : std::as_const(_fields))
    {
        if (key == _conditionalTriggerKey)
        {
            auto* combo = qobject_cast<QComboBox*>(widget);
            if (combo != nullptr)
            {
                connect(combo, &QComboBox::currentIndexChanged, this, &SchemaFormWidget::onTriggerChanged);
            }
            break;
        }
    }
}

QWidget* SchemaFormWidget::createWidgetForProperty(const QJsonObject& propSchema, const QJsonValue& value)
{
    QString type = propSchema.value("type").toString();
    const QJsonArray enumValues = propSchema.value("enum").toArray();

    if (!enumValues.isEmpty())
    {
        auto* combo = new QComboBox(this);
        bool isInteger = (type == "integer");
        const QJsonArray enumLabels = propSchema.value("x-enumLabels").toArray();

        for (int i = 0; i < enumValues.size(); ++i)
        {
            const QJsonValue& v = enumValues.at(i);
            QString label = (i < enumLabels.size()) ? enumLabels.at(i).toString() : QString();

            if (isInteger)
            {
                int intVal = v.toInt();
                if (label.isEmpty())
                {
                    label = QString::number(intVal);
                }
                combo->addItem(label, intVal);
            }
            else
            {
                QString strVal = v.toString();
                if (label.isEmpty())
                {
                    label = strVal;
                }
                combo->addItem(label, strVal);
            }
        }

        QVariant currentVariant = isInteger ? QVariant(value.toInt()) : QVariant(value.toString());
        int idx = combo->findData(currentVariant);
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
        int minVal = !minJson.isUndefined() ? minJson.toInt() : INT_MIN;
        int maxVal = !maxJson.isUndefined() ? maxJson.toInt() : INT_MAX;
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

bool SchemaFormWidget::parseConditional(const QJsonObject& schema)
{
    const QJsonObject ifObj = schema.value("if").toObject();
    const QJsonObject thenObj = schema.value("then").toObject();
    const QJsonObject elseObj = schema.value("else").toObject();

    if (ifObj.isEmpty() || thenObj.isEmpty() || elseObj.isEmpty())
    {
        return false;
    }

    const QJsonArray required = ifObj.value("required").toArray();
    if (required.size() != 1)
    {
        return false;
    }

    const QString triggerKey = required.at(0).toString();
    if (triggerKey.isEmpty())
    {
        return false;
    }

    const QJsonObject ifProperties = ifObj.value("properties").toObject();
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
}

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
            result[key] = spin->value();
        }
        else if (auto* dspin = qobject_cast<QDoubleSpinBox*>(widget))
        {
            result[key] = dspin->value();
        }
        else if (auto* check = qobject_cast<QCheckBox*>(widget))
        {
            result[key] = check->isChecked();
        }
        else if (auto* combo = qobject_cast<QComboBox*>(widget))
        {
            QVariant data = combo->currentData();
            if (data.userType() == QMetaType::Int)
            {
                result[key] = data.toInt();
            }
            else
            {
                result[key] = data.toString();
            }
        }
        else if (auto* edit = qobject_cast<QLineEdit*>(widget))
        {
            result[key] = edit->text();
        }
    }
    return result;
}
