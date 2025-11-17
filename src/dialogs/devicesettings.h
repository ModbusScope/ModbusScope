#ifndef DEVICESETTINGS_H
#define DEVICESETTINGS_H

#include "models/settingsmodel.h"
#include <QWidget>

// Forward declaration
class DeviceForm;

namespace Ui {
class DeviceSettings;
}

class DeviceSettings : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceSettings(SettingsModel* pSettingsModel, QWidget* parent = nullptr);
    ~DeviceSettings();

signals:
    void settingsTabsSwitched();

private slots:
    void handleAddTab();
    void handleCloseTab(int index);
    void updateTabName(deviceId_t devId);

private:
    DeviceForm* createForm(deviceId_t devId);
    QString constructTabName(deviceId_t devId);

    Ui::DeviceSettings* _pUi;
    SettingsModel* _pSettingsModel;
};

#endif // DEVICESETTINGS_H
