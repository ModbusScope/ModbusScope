#include "deviceform.h"
#include "ui_deviceform.h"

DeviceForm::DeviceForm(SettingsModel* pSettingsModel, deviceId_t _deviceId, QWidget* parent)
    : QWidget(parent), _pUi(new Ui::DeviceForm), _pSettingsModel(pSettingsModel), _deviceId(_deviceId)
{
    _pUi->setupUi(this);

    Device* dev = _pSettingsModel->deviceSettings(this->_deviceId);

    _pUi->lineName->setText(dev->name());
    _pUi->spinId->setValue(static_cast<int>(this->_deviceId));
    _pUi->spinSlaveId->setValue(dev->slaveId());
    _pUi->spinConsecutiveMax->setValue(dev->consecutiveMax());
    _pUi->checkEndianness->setChecked(dev->int32LittleEndian());
    updateConnectionList();

    connect(_pUi->lineName, &QLineEdit::textChanged, this, [this](const QString& newName) {
        Device* device = _pSettingsModel->deviceSettings(this->_deviceId);
        device->setName(newName);
        emit deviceIdentifiersChanged(this->_deviceId);
    });
    connect(_pUi->spinId, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int newId) {
        _pSettingsModel->updateDeviceId(this->_deviceId, static_cast<deviceId_t>(newId));
        this->_deviceId = newId;
        emit deviceIdentifiersChanged(newId);
    });

    connect(_pUi->spinSlaveId, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int newSlaveId) {
        Device* device = _pSettingsModel->deviceSettings(this->_deviceId);
        device->setSlaveId(static_cast<quint8>(newSlaveId));
    });

    connect(_pUi->spinConsecutiveMax, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int newConsecutiveMax) {
        Device* device = _pSettingsModel->deviceSettings(this->_deviceId);
        device->setConsecutiveMax(static_cast<quint8>(newConsecutiveMax));
    });

    connect(_pUi->checkEndianness, &QCheckBox::toggled, this, [this](bool bInt32LittleEndian) {
        Device* device = _pSettingsModel->deviceSettings(this->_deviceId);
        device->setInt32LittleEndian(bInt32LittleEndian);
    });

    connect(_pUi->comboConnection, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        Device* device = _pSettingsModel->deviceSettings(this->_deviceId);
        QVariant data = _pUi->comboConnection->itemData(index);
        if (data.isValid())
        {
            device->setConnectionId(static_cast<ConnectionTypes::connectionId_t>(data.toUInt()));
        }
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

void DeviceForm::handleSettingsTabSwitch()
{
    updateConnectionList();
}

void DeviceForm::updateConnectionList()
{
    _pUi->comboConnection->clear();
    for (auto connId : _pSettingsModel->connectionList())
    {
        _pUi->comboConnection->addItem(QString("Connection %1").arg(connId + 1), static_cast<quint32>(connId));
    }

    for (int i = 0; i < _pUi->comboConnection->count(); i++)
    {
        QVariant data = _pUi->comboConnection->itemData(i);
        if (data.isValid() && static_cast<ConnectionTypes::connectionId_t>(data.toUInt()) ==
                                _pSettingsModel->deviceSettings(this->_deviceId)->connectionId())
        {
            _pUi->comboConnection->setCurrentIndex(i);
            break;
        }
    }
}
