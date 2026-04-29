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
    explicit DeviceForm(SettingsModel* pSettingsModel, deviceId_t deviceId, QWidget* parent = nullptr);
    ~DeviceForm();

    deviceId_t deviceId();

signals:
    void deviceIdentifiersChanged(deviceId_t devId);

private slots:
    void onNameChanged(const QString& newName);

private:
    Ui::DeviceForm* _pUi;

    SettingsModel* _pSettingsModel;
    deviceId_t _deviceId;
};

#endif // DEVICEFORM_H
