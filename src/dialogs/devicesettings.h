#ifndef DEVICESETTINGS_H
#define DEVICESETTINGS_H

#include "customwidgets/centeredbox.h"
#include "models/settingsmodel.h"
#include <QWidget>

namespace Ui {
class DeviceSettings;
}

// forward declaration
class DeviceModel;

class DeviceSettings : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceSettings(SettingsModel* pSettingsModel, QWidget* parent = nullptr);
    ~DeviceSettings();

private slots:
    void onAddDeviceClicked();
    void onRemoveDeviceClicked();

private:
    Ui::DeviceSettings* _pUi;
    SettingsModel* _pSettingsModel;
    DeviceModel* _pDeviceModel;

    CenteredBoxProxyStyle _centeredBoxStyle;
};

#endif // DEVICESETTINGS_H
