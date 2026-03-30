#ifndef ADAPTERSETTINGS_H
#define ADAPTERSETTINGS_H

#include <QJsonObject>
#include <QString>
#include <QWidget>

class SettingsModel;
class SchemaFormWidget;
class AddableTabWidget;
class QVBoxLayout;

/*!
 * \brief Settings page for a single adapter schema property.
 *
 * Renders one top-level property from an adapter's JSON Schema as an editable
 * form. The property key (e.g. \c "connections" or \c "general") is supplied at
 * construction time. Schema type determines layout:
 * \c "type":"array" renders as a tabbed interface (one tab per item);
 * \c "type":"object" renders as a single form.
 */
class AdapterSettings : public QWidget
{
    Q_OBJECT

public:
    explicit AdapterSettings(SettingsModel* pSettingsModel,
                             const QString& adapterId,
                             const QString& propertyKey,
                             QWidget* parent = nullptr);
    ~AdapterSettings() = default;

    /*!
     * \brief Returns true if the schema property produces a non-empty page.
     *
     * \c "type":"array" requires non-empty items schema.
     * \c "type":"object" requires non-empty properties.
     * Scalar types always return false.
     */
    static bool isRenderableProperty(const QJsonObject& propSchema);

    /*!
     * \brief Write the current form values back to the adapter's configuration.
     */
    void acceptValues();

private:
    void buildSection(const QJsonObject& propSchema, const QJsonValue& configValue, QVBoxLayout* layout);

    SettingsModel* _pSettingsModel;
    QString _adapterId;
    QString _propertyKey;

    AddableTabWidget* _pItemTabs{ nullptr };
    SchemaFormWidget* _pForm{ nullptr };

    QJsonObject _itemSchema;
};

#endif // ADAPTERSETTINGS_H
