#include "connectiondialog.h"
#include "ui_connectiondialog.h"

#include "settingsmodel.h"

ConnectionDialog::ConnectionDialog(SettingsModel * pSettingsModel, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::ConnectionDialog)
{
    _pUi->setupUi(this);

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    _pSettingsModel = pSettingsModel;

    connect(_pSettingsModel, &SettingsModel::ipChanged, this, &ConnectionDialog::updateIp);
    connect(_pSettingsModel, &SettingsModel::portChanged, this, &ConnectionDialog::updatePort);
    connect(_pSettingsModel, &SettingsModel::slaveIdChanged, this, &ConnectionDialog::updateSlaveId);
    connect(_pSettingsModel, &SettingsModel::timeoutChanged, this, &ConnectionDialog::updateTimeout);
    connect(_pSettingsModel, &SettingsModel::consecutiveMaxChanged, this, &ConnectionDialog::updateConsecutiveMax);
    connect(_pSettingsModel, &SettingsModel::connectionStateChanged, this, &ConnectionDialog::updateConnectionState);
    connect(_pSettingsModel, &SettingsModel::int32LittleEndianChanged, this, &ConnectionDialog::updateInt32LittleEndian);
    connect(_pSettingsModel, &SettingsModel::persistentConnectionChanged, this, &ConnectionDialog::updatePersistentConnection);

    connect(_pUi->checkConn_2, &QCheckBox::stateChanged, this, &ConnectionDialog::secondConnectionStateChanged);

    _pUi->connectionForm_3->setState(false);
    connect(_pUi->checkConn_3, &QCheckBox::stateChanged, _pUi->connectionForm_3, &ConnectionForm::setState);
}

ConnectionDialog::~ConnectionDialog()
{
    delete _pUi;
}

void ConnectionDialog::secondConnectionStateChanged(int state)
{
    bool bState = (state == Qt::Checked);

    _pUi->lineIP_2->setEnabled(bState);
    _pUi->spinPort_2->setEnabled(bState);
    _pUi->spinSlaveId_2->setEnabled(bState);
    _pUi->spinTimeout_2->setEnabled(bState);
    _pUi->spinConsecutiveMax_2->setEnabled(bState);
    _pUi->checkInt32LittleEndian_2->setEnabled(bState);
    _pUi->checkPersistentConn_2->setEnabled(bState);
}

void ConnectionDialog::updateIp(quint8 connectionId)
{
    if (connectionId == SettingsModel::CONNECTION_ID_0)
    {
        _pUi->lineIP->setText(_pSettingsModel->ipAddress(connectionId));
    }
    else
    {
        _pUi->lineIP_2->setText(_pSettingsModel->ipAddress(connectionId));
    }
}

void ConnectionDialog::updatePort(quint8 connectionId)
{
    if (connectionId == SettingsModel::CONNECTION_ID_0)
    {
        _pUi->spinPort->setValue(_pSettingsModel->port(connectionId));
    }
    else
    {
        _pUi->spinPort_2->setValue(_pSettingsModel->port(connectionId));
    }
}

void ConnectionDialog::updateSlaveId(quint8 connectionId)
{
    if (connectionId == SettingsModel::CONNECTION_ID_0)
    {
        _pUi->spinSlaveId->setValue(_pSettingsModel->slaveId(connectionId));
    }
    else
    {
        _pUi->spinSlaveId_2->setValue(_pSettingsModel->slaveId(connectionId));
    }
}

void ConnectionDialog::updateTimeout(quint8 connectionId)
{
    if (connectionId == SettingsModel::CONNECTION_ID_0)
    {
        _pUi->spinTimeout->setValue(static_cast<int>(_pSettingsModel->timeout(connectionId)));
    }
    else
    {
        _pUi->spinTimeout_2->setValue(static_cast<int>(_pSettingsModel->timeout(connectionId)));
    }

}

void ConnectionDialog::updateConsecutiveMax(quint8 connectionId)
{
    if (connectionId == SettingsModel::CONNECTION_ID_0)
    {
        _pUi->spinConsecutiveMax->setValue(_pSettingsModel->consecutiveMax(connectionId));
    }
    else
    {
        _pUi->spinConsecutiveMax_2->setValue(_pSettingsModel->consecutiveMax(connectionId));
    }
}

void ConnectionDialog::updateConnectionState(quint8 connectionId)
{
    /* TODO: change for more than 2 connections */
    if (connectionId == SettingsModel::CONNECTION_ID_1)
    {
        _pUi->checkConn_2->setChecked(_pSettingsModel->connectionState(connectionId));
    }
}

void ConnectionDialog::updateInt32LittleEndian(quint8 connectionId)
{
    if (connectionId == SettingsModel::CONNECTION_ID_0)
    {
        _pUi->checkInt32LittleEndian->setChecked(_pSettingsModel->int32LittleEndian(connectionId));
    }
    else
    {
        _pUi->checkInt32LittleEndian_2->setChecked(_pSettingsModel->int32LittleEndian(connectionId));
    }
}

void ConnectionDialog::updatePersistentConnection(quint8 connectionId)
{
    if (connectionId == SettingsModel::CONNECTION_ID_0)
    {
        _pUi->checkPersistentConn->setChecked(_pSettingsModel->persistentConnection(connectionId));
    }
    else
    {
        _pUi->checkPersistentConn_2->setChecked(_pSettingsModel->persistentConnection(connectionId));
    }
}

void ConnectionDialog::done(int r)
{
    bool bValid = true;

    if(QDialog::Accepted == r)  // ok was pressed
    {
        _pSettingsModel->setIpAddress(SettingsModel::CONNECTION_ID_0, _pUi->lineIP->text());
        _pSettingsModel->setPort(SettingsModel::CONNECTION_ID_0, _pUi->spinPort->text().toUInt());
        _pSettingsModel->setSlaveId(SettingsModel::CONNECTION_ID_0, _pUi->spinSlaveId->text().toInt());
        _pSettingsModel->setTimeout(SettingsModel::CONNECTION_ID_0, _pUi->spinTimeout->text().toUInt());
        _pSettingsModel->setConsecutiveMax(SettingsModel::CONNECTION_ID_0, _pUi->spinConsecutiveMax->text().toUInt());
        _pSettingsModel->setInt32LittleEndian(SettingsModel::CONNECTION_ID_0, _pUi->checkInt32LittleEndian->checkState() == Qt::Checked);
        _pSettingsModel->setPersistentConnection(SettingsModel::CONNECTION_ID_0, _pUi->checkPersistentConn->checkState() == Qt::Checked);

        _pSettingsModel->setIpAddress(SettingsModel::CONNECTION_ID_1, _pUi->lineIP_2->text());
        _pSettingsModel->setPort(SettingsModel::CONNECTION_ID_1, _pUi->spinPort_2->text().toUInt());
        _pSettingsModel->setSlaveId(SettingsModel::CONNECTION_ID_1, _pUi->spinSlaveId_2->text().toUInt());
        _pSettingsModel->setTimeout(SettingsModel::CONNECTION_ID_1, _pUi->spinTimeout_2->text().toUInt());
        _pSettingsModel->setConsecutiveMax(SettingsModel::CONNECTION_ID_1, _pUi->spinConsecutiveMax_2->text().toUInt());
        _pSettingsModel->setConnectionState(SettingsModel::CONNECTION_ID_1, _pUi->checkConn_2->checkState() == Qt::Checked);
        _pSettingsModel->setInt32LittleEndian(SettingsModel::CONNECTION_ID_1, _pUi->checkInt32LittleEndian_2->checkState() == Qt::Checked);
        _pSettingsModel->setPersistentConnection(SettingsModel::CONNECTION_ID_1, _pUi->checkPersistentConn_2->checkState() == Qt::Checked);

        _pUi->connectionForm_3->fillSettingsModel(_pSettingsModel, SettingsModel::CONNECTION_ID_2);

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
