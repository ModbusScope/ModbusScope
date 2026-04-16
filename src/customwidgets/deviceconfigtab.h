#ifndef DEVICECONFIGTAB_H
#define DEVICECONFIGTAB_H

#include <QJsonObject>
#include <QWidget>

class SettingsModel;
class SchemaFormWidget;
class QComboBox;
class QLineEdit;
class QVBoxLayout;

/*!
 * \brief A tab widget for a single device's adapter selection and schema-driven configuration.
 *
 * Displays a combo box to select the protocol adapter for this device, followed by
 * a schema-driven form for device-specific settings from the selected adapter's schema.
 * When the adapter selection changes the form is rebuilt with the new adapter's schema.
 */
class DeviceConfigTab : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceConfigTab(SettingsModel* pSettingsModel,
                             const QString& adapterId,
                             const QJsonObject& deviceValues,
                             QWidget* parent = nullptr);
    ~DeviceConfigTab() = default;

    /*!
     * \brief Return the current device values from the schema form.
     * \return Device field values as a JSON object.
     */
    QJsonObject values() const;

    /*!
     * \brief Return the currently selected adapter ID.
     * \return Adapter identifier string.
     */
    QString adapterId() const;

    /*!
     * \brief Return the current device name from the name field.
     * \return Device name string (may be empty).
     */
    QString deviceName() const;

private slots:
    void onAdapterChanged(int index);

private:
    void rebuildSchemaForm(const QString& adapterId, const QJsonObject& deviceValues);

    QVBoxLayout* _pLayout;
    QLineEdit* _pNameEdit;
    QComboBox* _pAdapterCombo;
    SchemaFormWidget* _pSchemaForm;
    SettingsModel* _pSettingsModel;
    int _deviceId;
};

#endif // DEVICECONFIGTAB_H
