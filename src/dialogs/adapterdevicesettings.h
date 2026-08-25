#ifndef ADAPTERDEVICESETTINGS_H
#define ADAPTERDEVICESETTINGS_H

#include "models/device.h"

#include <QJsonObject>
#include <QList>
#include <QStringList>
#include <QWidget>

class SettingsModel;
class AddableTabWidget;
class DeviceConfigTab;
class QLabel;

/*!
 * \brief Settings page for adapter device configuration.
 *
 * Displays one tab per device. Each tab contains a protocol adapter selector
 * and a schema-driven form for the device-specific settings from the selected
 * adapter's schema. Devices from all registered adapters are shown together.
 *
 * The open tabs are the working copy of the device list: nothing typed on this page
 * reaches SettingsModel until acceptValues() applies the tabs in one transaction, so
 * cancelling the dialog discards every device edit.
 *
 * A device whose owning adapter has no schema yet gets no tab — there is nothing to render it
 * with — and is carried through acceptValues() untouched rather than being shown under, and
 * reassigned to, some other adapter.
 *
 * The set of adapters this page edits is fixed when it is built. An adapter that describes
 * while the dialog is open takes no part in the transaction, so accepting cannot overwrite a
 * config the page never showed.
 */
class AdapterDeviceSettings : public QWidget
{
    Q_OBJECT

public:
    explicit AdapterDeviceSettings(SettingsModel* pSettingsModel, QWidget* parent = nullptr);
    ~AdapterDeviceSettings() = default;

    void acceptValues();

private slots:
    void handleAddTab();
    void handleCloseTab(QWidget* widget);

private:
    QStringList validAdapterIds() const;
    QString defaultAdapterId() const;
    QJsonObject deviceValuesFor(const QString& adapterId, deviceId_t devId) const;
    QString constructTabName(DeviceConfigTab* tab) const;
    DeviceConfigTab* createTab(const QString& adapterId, const QString& deviceName, const QJsonObject& deviceValues);
    void appendTabsWithoutDeviceId(QList<QWidget*>& pages, QStringList& names);
    QString deviceLimitWarningMessage() const;
    void updateDeviceLimitIndication();

    SettingsModel* _pSettingsModel;
    AddableTabWidget* _pDeviceTabs{ nullptr };
    QLabel* _pLimitWarningLabel{ nullptr };

    //! The adapters this page edits, fixed when it is built.
    QStringList _validAdapterIds;

    //! Devices this page cannot render, so that accepting hands them back instead of dropping them.
    QList<deviceId_t> _deviceIdsWithoutTab;
};

#endif // ADAPTERDEVICESETTINGS_H
