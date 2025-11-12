#ifndef DEVICESETTINGS_H
#define DEVICESETTINGS_H

#include "models/settingsmodel.h"
#include <QWidget>

namespace Ui {
class DeviceSettings;
}

class DeviceSettings : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceSettings(SettingsModel* pSettingsModel, QWidget* parent = nullptr);
    ~DeviceSettings();

private slots:
    void handleAddTab();
    void handleCloseTab(int index);

private:
    Ui::DeviceSettings* _pUi;
    SettingsModel* _pSettingsModel;
};

#endif // DEVICESETTINGS_H
