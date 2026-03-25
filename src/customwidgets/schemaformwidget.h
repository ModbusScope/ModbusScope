#ifndef SCHEMAFORMWIDGET_H
#define SCHEMAFORMWIDGET_H

#include <QJsonObject>
#include <QJsonValue>
#include <QWidget>

class QFormLayout;

/*!
 * \brief Renders a JSON Schema \c "type":"object" as a form with input widgets.
 *
 * Creates one labelled row per property in the schema. Supports string, integer,
 * number, boolean, and enum-constrained string/integer properties.
 * The label for each row is taken from the property's \c "title" field, falling
 * back to the property key name if no title is provided.
 */
class SchemaFormWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SchemaFormWidget(QWidget* parent = nullptr);
    ~SchemaFormWidget() = default;

    /*!
     * \brief Populate the form from a JSON Schema object and current values.
     * \param schema  A JSON Schema with \c "type":"object" and a \c "properties" map.
     * \param values  Initial values matching the schema properties.
     */
    void setSchema(const QJsonObject& schema, const QJsonObject& values);

    /*!
     * \brief Return current form input as a JSON object.
     * \return A QJsonObject with one entry per schema property.
     */
    QJsonObject values() const;

private:
    QWidget* createWidgetForProperty(const QJsonObject& propSchema, const QJsonValue& value);

    QFormLayout* _pFormLayout;
    QList<QPair<QString, QWidget*>> _fields;
};

#endif // SCHEMAFORMWIDGET_H
