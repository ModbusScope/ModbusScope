#ifndef DEVICECONFIGTAB_H
#define DEVICECONFIGTAB_H

#include <QJsonObject>
#include <QWidget>

class AdapterConfigDraft;
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
 *
 * The tab holds the working copy of a single device: it never writes to SettingsModel, so
 * edits are discarded when the dialog is cancelled. Its owner reads the edited values back
 * through values(), deviceName() and adapterId() when the user accepts.
 */
class DeviceConfigTab : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceConfigTab(SettingsModel* pSettingsModel,
                             const QString& adapterId,
                             const QString& deviceName,
                             const QJsonObject& deviceValues,
                             QWidget* parent = nullptr,
                             AdapterConfigDraft* pDraft = nullptr);
    ~DeviceConfigTab() = default;

    QJsonObject values() const;

    QString adapterId() const;

    QString deviceName() const;

    int deviceId() const;

signals:
    //! Emitted when the device name field changes.
    void nameChanged(const QString& name);
    //! Emitted when the selected protocol adapter changes.
    void adapterChanged(const QString& newAdapterId);

private slots:
    void onAdapterChanged(int index);
    void onNameChanged(const QString& name);
    void onDraftConfigChanged(const QString& adapterId);

private:
    void rebuildSchemaForm(const QString& adapterId, const QJsonObject& deviceValues);

    QVBoxLayout* _pLayout;
    QLineEdit* _pNameEdit;
    QComboBox* _pAdapterCombo;
    SchemaFormWidget* _pSchemaForm;
    SettingsModel* _pSettingsModel;
    int _deviceId;
    AdapterConfigDraft* _pDraft;
};

#endif // DEVICECONFIGTAB_H
