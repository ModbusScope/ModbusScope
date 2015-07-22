#include "connectiondialog.h"
#include "ui_connectiondialog.h"

#include "settingsmodel.h"

ConnectionDialog::ConnectionDialog(SettingsModel * pSettingsModel, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::ConnectionDialog)
{
    _pUi->setupUi(this);

    _pSettingsModel = pSettingsModel;

    connect(_pSettingsModel, SIGNAL(ipChanged()), this, SLOT(updateIp()));
    connect(_pSettingsModel, SIGNAL(portChanged()), this, SLOT(updatePort()));
    connect(_pSettingsModel, SIGNAL(slaveIdChanged()), this, SLOT(updateSlaveId()));
    connect(_pSettingsModel, SIGNAL(timeoutChanged()), this, SLOT(updateTimeout()));
}

ConnectionDialog::~ConnectionDialog()
{
    delete _pUi;
}

void ConnectionDialog::updateIp()
{
    _pUi->lineIP->setText(_pSettingsModel->ipAddress());
}

void ConnectionDialog::updatePort()
{
    _pUi->spinPort->setValue(_pSettingsModel->port());
}

void ConnectionDialog::updateSlaveId()
{
    _pUi->spinSlaveId->setValue(_pSettingsModel->slaveId());
}

void ConnectionDialog::updateTimeout()
{
    _pUi->spinTimeout->setValue(_pSettingsModel->timeout());
}

void ConnectionDialog::done(int r)
{
    bool bValid = true;

    if(QDialog::Accepted == r)  // ok was pressed
    {
        _pSettingsModel->setIpAddress(_pUi->lineIP->text());
        _pSettingsModel->setPort(_pUi->spinPort->text().toInt());
        _pSettingsModel->setSlaveId(_pUi->spinSlaveId->text().toInt());
        _pSettingsModel->setTimeout(_pUi->spinTimeout->text().toUInt());

        // Validate the data
        //bValid = validateSettingsData();
        bValid = true;
    }
    else
    {
        // cancel, close or exc was pressed
        bValid = true;
    }

    if (bValid)
    {
        QDialog::done(r);
    }
}
