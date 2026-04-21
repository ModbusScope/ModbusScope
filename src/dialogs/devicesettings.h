#ifndef DEVICESETTINGS_H
#define DEVICESETTINGS_H

#include "models/settingsmodel.h"
#include <QWidget>

// Forward declarations
class AddableTabWidget;
class DeviceForm;

class DeviceSettings : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceSettings(SettingsModel* pSettingsModel, QWidget* parent = nullptr);

private slots:
    void handleAddTab();
    void handleCloseTab(QWidget* widget);
    void updateTabName(deviceId_t devId);

private:
    DeviceForm* createForm(deviceId_t devId);
    QString constructTabName(deviceId_t devId);

    AddableTabWidget* _pDeviceTabs;
    SettingsModel* _pSettingsModel;
};

#endif // DEVICESETTINGS_H
