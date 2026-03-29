#ifndef SCHEMAFORMWIDGET_H
#define SCHEMAFORMWIDGET_H

#include <QJsonObject>
#include <QJsonValue>
#include <QStringList>
#include <QWidget>

class QFormLayout;

/*!
 * \brief Renders a JSON Schema \c "type":"object" as a form with input widgets.
 *
 * Creates one labelled row per property in the schema. Supports string, integer,
 * number, boolean, and enum-constrained string/integer properties.
 * The label for each row is taken from the property's \c "title" field, falling
 * back to the property key name if no title is provided.
 *
 * Supports the JSON Schema Draft 7 \c if/then/else pattern with a single-property
 * \c const condition. When detected, the active branch's fields are shown and the
 * inactive branch's fields are hidden, and visibility updates live as the trigger
 * field changes.
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
     *
     * Fields belonging to the inactive conditional branch are omitted.
     * \return A QJsonObject with one entry per visible schema property.
     */
    QJsonObject values() const;

private slots:
    //! Called when the trigger combo selection changes; re-evaluates conditional visibility.
    void onTriggerChanged(int index);

private:
    QWidget* createWidgetForProperty(const QJsonObject& propSchema, const QJsonValue& value);

    /*!
     * \brief Parse the top-level \c if/then/else block and populate conditional state.
     *
     * Only the narrow single-property \c const pattern is supported.
     * \param schema  Full schema object passed to setSchema().
     * \return \c true if a supported pattern was found and state was populated.
     */
    bool parseConditional(const QJsonObject& schema);

    /*!
     * \brief Show or hide rows based on whether the trigger value matches the const.
     * \param triggerValue  Current string value of the trigger field.
     */
    void applyConditional(const QString& triggerValue);

    QFormLayout* _pFormLayout;
    QList<QPair<QString, QWidget*>> _fields;

    //! Key in \c "properties" whose value drives the if/then/else switch.
    QString _conditionalTriggerKey;

    //! The \c "const" value that activates the \c "then" branch.
    QString _conditionalTriggerConst;

    //! Keys belonging to the \c "then" branch (shown when condition is true).
    QStringList _thenKeys;

    //! Keys belonging to the \c "else" branch (shown when condition is false).
    QStringList _elseKeys;

    //! Current value of the trigger field; used to filter \c values() output.
    QString _currentTriggerValue;
};

#endif // SCHEMAFORMWIDGET_H
