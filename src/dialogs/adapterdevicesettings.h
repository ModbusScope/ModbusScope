#ifndef ADAPTERDEVICESETTINGS_H
#define ADAPTERDEVICESETTINGS_H

#include <QJsonObject>
#include <QWidget>

class SettingsModel;
class AddableTabWidget;

/*!
 * \brief Settings page for adapter device configuration.
 *
 * Displays one tab per device. Each tab contains a protocol adapter selector
 * and a schema-driven form for the device-specific settings from the selected
 * adapter's schema. Devices from all registered adapters are shown together.
 */
class AdapterDeviceSettings : public QWidget
{
    Q_OBJECT

public:
    explicit AdapterDeviceSettings(SettingsModel* pSettingsModel, QWidget* parent = nullptr);
    ~AdapterDeviceSettings() = default;

    /*!
     * \brief Write the current form values back to each adapter's device configuration.
     */
    void acceptValues();

private slots:
    void handleAddTab();

private:
    QString constructTabName(const QJsonObject& deviceValues, int tabIndex) const;

    SettingsModel* _pSettingsModel;
    AddableTabWidget* _pDeviceTabs{ nullptr };
};

#endif // ADAPTERDEVICESETTINGS_H
