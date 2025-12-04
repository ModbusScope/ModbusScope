#ifndef DEVICEFORM_H
#define DEVICEFORM_H

#include "models/settingsmodel.h"

#include <QWidget>

namespace Ui {
class DeviceForm;
}

class DeviceForm : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceForm(SettingsModel* pSettingsModel, deviceId_t _deviceId, QWidget* parent = nullptr);
    ~DeviceForm();

    deviceId_t deviceId();

signals:
    void deviceIdentifiersChanged(deviceId_t devId);

public slots:
    void handleSettingsTabSwitch();

private:
    void updateConnectionList();
    void checkConnectionState();

    Ui::DeviceForm* _pUi;

    SettingsModel* _pSettingsModel;
    deviceId_t _deviceId;
};

#endif // DEVICEFORM_H
