#ifndef ADAPTERSETTINGS_H
#define ADAPTERSETTINGS_H

#include <QJsonObject>
#include <QString>
#include <QWidget>

class AdapterConfigDraft;
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
                             QWidget* parent = nullptr,
                             AdapterConfigDraft* pDraft = nullptr);
    ~AdapterSettings() = default;

    static bool isRenderableProperty(const QJsonObject& propSchema);

    void acceptValues();

private:
    void buildSection(const QJsonObject& propSchema, const QJsonValue& configValue, QVBoxLayout* layout);
    void addItemTab();

    //! \brief Returns a display name for a tab at the given 1-based \a index.
    QString formatTabName(int index) const;

    //! \brief Returns this page's current value for its schema property.
    QJsonValue sectionValue() const;

    //! \brief Publishes sectionValue() to the draft so other pages see it.
    void publishSection();

    //! \brief Feeds every item form the options of the referenced collections.
    void refreshReferenceOptions(const QString& adapterId);

    //! \brief Wires \a form to the draft and to the tab label.
    void connectForm(SchemaFormWidget* form);

    void onSchemaFieldNameChanged(SchemaFormWidget* form, const QString& key, const QString& value);

    SettingsModel* _pSettingsModel;
    QString _adapterId;
    QString _propertyKey;
    AdapterConfigDraft* _pDraft;

    AddableTabWidget* _pItemTabs{ nullptr };
    SchemaFormWidget* _pForm{ nullptr };

    QJsonObject _itemSchema;
    int _nextItemTabIndex{ 1 };
};

#endif // ADAPTERSETTINGS_H
