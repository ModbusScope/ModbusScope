#include "deviceform.h"
#include "ui_deviceform.h"

DeviceForm::DeviceForm(SettingsModel* pSettingsModel, deviceId_t _deviceId, QWidget* parent)
    : QWidget(parent), _pU(new Ui::DeviceForm), _pSettingsModel(pSettingsModel), _deviceId(_deviceId)
{
    _pU->setupUi(this);
}

DeviceForm::~DeviceForm()
{
    delete _pU;
}

deviceId_t DeviceForm::deviceId()
{
    return _deviceId;
}
