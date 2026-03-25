#ifndef ADAPTERCONNECTIONSETTINGS_H
#define ADAPTERCONNECTIONSETTINGS_H

#include <QJsonObject>
#include <QWidget>

class SettingsModel;
class SchemaFormWidget;
class AddableTabWidget;
class QVBoxLayout;

/*!
 * \brief Settings page for adapter general and connection configuration.
 *
 * Displays the general section (if non-empty) and connections section from the
 * active adapter's JSON Schema. Schema type determines layout:
 * \c "type":"array" renders as a tabbed interface (one tab per connection);
 * \c "type":"object" renders as a single form.
 */
class AdapterConnectionSettings : public QWidget
{
    Q_OBJECT

public:
    explicit AdapterConnectionSettings(SettingsModel* pSettingsModel, QWidget* parent = nullptr);
    ~AdapterConnectionSettings() = default;

    /*!
     * \brief Write the current form values back to the adapter's configuration.
     */
    void acceptValues();

private:
    void buildGeneralSection(const QJsonObject& schema, const QJsonObject& config, QVBoxLayout* layout);
    void buildConnectionsSection(const QJsonObject& schema, const QJsonObject& config, QVBoxLayout* layout);

    SettingsModel* _pSettingsModel;
    QString _adapterId;

    SchemaFormWidget* _pGeneralForm{ nullptr };
    AddableTabWidget* _pConnectionTabs{ nullptr };
    SchemaFormWidget* _pConnectionForm{ nullptr };

    QJsonObject _connectionItemSchema;
};

#endif // ADAPTERCONNECTIONSETTINGS_H
