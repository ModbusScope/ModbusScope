#ifndef ADAPTERDEVICESETTINGS_H
#define ADAPTERDEVICESETTINGS_H

#include <QStringList>
#include <QWidget>

class SettingsModel;
class AddableTabWidget;
class DeviceConfigTab;

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
     * \brief Write the per-adapter device JSON arrays back to each adapter's stored config.
     */
    void acceptValues();

private slots:
    void handleAddTab();
    void handleCloseTab(QWidget* widget);

private:
    QStringList validAdapterIds() const;
    QString constructTabName(DeviceConfigTab* tab) const;
    void connectTabNameTracking(DeviceConfigTab* tab);

    SettingsModel* _pSettingsModel;
    AddableTabWidget* _pDeviceTabs{ nullptr };
};

#endif // ADAPTERDEVICESETTINGS_H
