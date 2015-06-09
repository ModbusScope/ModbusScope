#include "connectiondialog.h"
#include "ui_connectiondialog.h"

ConnectionDialog::ConnectionDialog(ConnectionModel * pConnectionModel, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::ConnectionDialog)
{
    _pUi->setupUi(this);

    _pConnectionModel = pConnectionModel;

    connect(_pConnectionModel, SIGNAL(ipChanged()), this, SLOT(updateIp()));
    connect(_pConnectionModel, SIGNAL(portChanged()), this, SLOT(updatePort()));
    connect(_pConnectionModel, SIGNAL(pollTimeChanged()), this, SLOT(updatePollTime()));
    connect(_pConnectionModel, SIGNAL(slaveIdChanged()), this, SLOT(updateSlaveId()));
    connect(_pConnectionModel, SIGNAL(timeoutChanged()), this, SLOT(updateTimeout()));
}

ConnectionDialog::~ConnectionDialog()
{
    delete _pUi;
}

void ConnectionDialog::updateIp()
{
    _pUi->lineIP->setText(_pConnectionModel->ipAddress());
}

void ConnectionDialog::updatePort()
{
    _pUi->spinPort->setValue(_pConnectionModel->port());
}

void ConnectionDialog::updatePollTime()
{
    _pUi->spinPollTime->setValue(_pConnectionModel->pollTime());
}

void ConnectionDialog::updateSlaveId()
{
    _pUi->spinSlaveId->setValue(_pConnectionModel->slaveId());
}

void ConnectionDialog::updateTimeout()
{
    _pUi->spinTimeout->setValue(_pConnectionModel->timeout());
}

void ConnectionDialog::done(int r)
{
    bool bValid = true;

    if(QDialog::Accepted == r)  // ok was pressed
    {
        _pConnectionModel->setIpAddress(_pUi->lineIP->text());
        _pConnectionModel->setPort(_pUi->spinPort->text().toInt());
        _pConnectionModel->setSlaveId(_pUi->spinSlaveId->text().toInt());
        _pConnectionModel->setTimeout(_pUi->spinTimeout->text().toUInt());
        _pConnectionModel->setPollTime(_pUi->spinPollTime->text().toUInt());

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
