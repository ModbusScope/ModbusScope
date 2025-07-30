#include "deviceform.h"
#include "ui_deviceform.h"

DeviceForm::DeviceForm(QWidget* parent) : QWidget(parent), _pUi(new Ui::DeviceForm)
{
    _pUi->setupUi(this);
}

DeviceForm::~DeviceForm()
{
    delete _pUi;
}

void DeviceForm::fillSettingsModel(Device* devData)
{
    devData->setSlaveId(_pUi->spinSlaveId->value());
    devData->setConsecutiveMax(_pUi->spinConsecutiveMax->value());
    devData->setInt32LittleEndian(_pUi->checkInt32LittleEndian->checkState() == Qt::Checked);
}

#if 0
TODO: dev
void ConnectionForm::setSlaveId(quint8 id)
{
    _pUi->spinSlaveId->setValue(id);
}

void ConnectionForm::setTimeout(quint32 timeout)
{
    _pUi->spinTimeout->setValue(timeout);
}

void ConnectionForm::setConsecutiveMax(quint8 max)
{
    _pUi->spinConsecutiveMax->setValue(max);
}

void ConnectionForm::setInt32LittleEndian(bool int32LittleEndian)
{
    _pUi->checkInt32LittleEndian->setChecked(int32LittleEndian);
}

#endif
