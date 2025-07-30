#ifndef DEVICEFORM_H
#define DEVICEFORM_H

#include <QWidget>

#include "models/device.h"

namespace Ui {
class DeviceForm;
}

class DeviceForm : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceForm(QWidget* parent = nullptr);
    ~DeviceForm();

    void fillSettingsModel(Device* devData);

private:
    Ui::DeviceForm* _pUi;
};

#endif // DEVICEFORM_H
