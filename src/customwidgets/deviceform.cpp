#include "deviceform.h"
#include "ui_deviceform.h"

DeviceForm::DeviceForm(SettingsModel* pSettingsModel, deviceId_t deviceId, QWidget* parent)
    : QWidget(parent), _pUi(new Ui::DeviceForm), _pSettingsModel(pSettingsModel), _deviceId(deviceId)
{
    _pUi->setupUi(this);

    Device* dev = _pSettingsModel->deviceSettings(this->_deviceId);

    _pUi->lineName->setText(dev->name());
    _pUi->spinId->setValue(static_cast<int>(this->_deviceId));

    connect(_pUi->lineName, &QLineEdit::textChanged, this, &DeviceForm::onNameChanged);
    connect(_pUi->spinId, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int newId) {
        _pSettingsModel->updateDeviceId(this->_deviceId, static_cast<deviceId_t>(newId));
        this->_deviceId = newId;
        emit deviceIdentifiersChanged(newId);
    });
}

DeviceForm::~DeviceForm()
{
    delete _pUi;
}

deviceId_t DeviceForm::deviceId()
{
    return _deviceId;
}

void DeviceForm::onNameChanged(const QString& newName)
{
    Device* device = _pSettingsModel->deviceSettings(_deviceId);
    device->setName(newName);
    emit deviceIdentifiersChanged(_deviceId);
}
