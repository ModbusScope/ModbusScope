#ifndef SCHEMAFORMWIDGET_H
#define SCHEMAFORMWIDGET_H

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QWidget>

class QComboBox;
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
 *
 * A property carrying \c "x-ref":{"collection","value","label"} is rendered as a combo box that
 * shows the item label and stores the item value. The items come from the host through
 * setReferenceOptions(), keyed by collection name.
 */
class SchemaFormWidget : public QWidget
{
    Q_OBJECT

public:
    //! One selectable target of an \c x-ref property: the stored value and the text shown for it.
    struct ReferenceOption
    {
        QJsonValue value;
        QString label;

        bool operator==(const ReferenceOption& other) const
        {
            return value == other.value && label == other.label;
        }
    };

    explicit SchemaFormWidget(QWidget* parent = nullptr);

    static QList<ReferenceOption> optionsFromArray(const QJsonArray& items,
                                                   const QString& valueKey = QStringLiteral("id"),
                                                   const QString& labelKey = QStringLiteral("name"));

    static QMap<QString, QList<ReferenceOption>> referenceOptionsForSchema(const QJsonObject& schema,
                                                                           const QJsonObject& config);

    void setReferenceOptions(const QString& collection, const QList<ReferenceOption>& options);

    void setReferenceConfig(const QJsonObject& config);

    void setSchema(const QJsonObject& schema, const QJsonObject& values);

    QJsonObject values() const;

signals:
    //! Emitted when a string field's value changes; \a key is the property name.
    void fieldChanged(const QString& key, const QString& value);

    //! Emitted when the user changes any field of the form.
    void valuesChanged();

private slots:
    //! Called when the trigger combo selection changes; re-evaluates conditional visibility.
    void onTriggerChanged(int index);

private:
    //! A combo box rendered for an \c x-ref property.
    struct ReferenceCombo
    {
        QComboBox* pCombo;
        QString collection;
        bool isInteger;
    };

    //! Combo item data for \a value: an int when \a isInteger, otherwise a string; invalid if the type does not fit.
    static QVariant comboKey(const QJsonValue& value, bool isInteger);

    //! Recompute the reference options from the stored schema and reference config.
    void applyReferenceConfig();

    //! Fill \a pCombo with \a items; item data is an int when \a isInteger, otherwise a string.
    static void populateCombo(QComboBox* pCombo, const QList<ReferenceOption>& items, bool isInteger);

    //! Repopulate a reference combo and select \a current, adding a "(missing)" item if it is unknown.
    void fillReferenceCombo(const ReferenceCombo& ref, const QJsonValue& current);

    QWidget* createWidgetForProperty(const QJsonObject& propSchema, const QJsonValue& value);

    void addFieldRow(const QString& key, const QJsonObject& propSchema, const QJsonValue& value);

    //! Forwards changes of \a widget to valuesChanged(); a QLineEdit also emits fieldChanged(\a key).
    void wireFieldChanged(const QString& key, QWidget* widget);

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

    //! Return true if \a widget is the field that drives the if/then/else switch.
    bool isTriggerWidget(const QWidget* widget) const;

    QFormLayout* _pFormLayout;
    QJsonObject _schema;
    QJsonObject _referenceConfig;
    bool _hasReferenceConfig{ false };
    QList<QPair<QString, QWidget*>> _fields;

    //! Combo boxes of \c x-ref properties, repopulated by setReferenceOptions().
    QList<ReferenceCombo> _referenceCombos;

    //! Options offered per referenced collection.
    QMap<QString, QList<ReferenceOption>> _references;

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
