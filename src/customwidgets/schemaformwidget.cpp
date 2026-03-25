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

SchemaFormWidget::SchemaFormWidget(QWidget* parent) : QWidget(parent), _pFormLayout(new QFormLayout(this))
{
    setLayout(_pFormLayout);
}

void SchemaFormWidget::setSchema(const QJsonObject& schema, const QJsonObject& values)
{
    _fields.clear();

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
}

QWidget* SchemaFormWidget::createWidgetForProperty(const QJsonObject& propSchema, const QJsonValue& value)
{
    QString type = propSchema.value("type").toString();
    const QJsonArray enumValues = propSchema.value("enum").toArray();

    if (!enumValues.isEmpty())
    {
        auto* combo = new QComboBox(this);
        bool isInteger = (type == "integer");

        for (const auto& v : enumValues)
        {
            if (isInteger)
            {
                int intVal = v.toInt();
                combo->addItem(QString::number(intVal), intVal);
            }
            else
            {
                QString strVal = v.toString();
                combo->addItem(strVal, strVal);
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

QJsonObject SchemaFormWidget::values() const
{
    QJsonObject result;
    for (const auto& [key, widget] : _fields)
    {
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
