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

    connect(_pSettingsModel, &SettingsModel::connectionTypeChanged, this, &ConnectionDialog::updateConnectionType);

    connect(_pSettingsModel, &SettingsModel::portNameChanged, this, &ConnectionDialog::updatePortName);
    connect(_pSettingsModel, &SettingsModel::parityChanged  , this, &ConnectionDialog::updateParity);
    connect(_pSettingsModel, &SettingsModel::baudrateChanged, this, &ConnectionDialog::updateBaudrate);
    connect(_pSettingsModel, &SettingsModel::databitsChanged, this, &ConnectionDialog::updateDatabits);
    connect(_pSettingsModel, &SettingsModel::stopbitsChanged, this, &ConnectionDialog::updateStopbits);

    connect(_pSettingsModel, &SettingsModel::slaveIdChanged, this, &ConnectionDialog::updateSlaveId);
    connect(_pSettingsModel, &SettingsModel::timeoutChanged, this, &ConnectionDialog::updateTimeout);
    connect(_pSettingsModel, &SettingsModel::consecutiveMaxChanged, this, &ConnectionDialog::updateConsecutiveMax);
    connect(_pSettingsModel, &SettingsModel::connectionStateChanged, this, &ConnectionDialog::updateConnectionState);
    connect(_pSettingsModel, &SettingsModel::int32LittleEndianChanged, this, &ConnectionDialog::updateInt32LittleEndian);
    connect(_pSettingsModel, &SettingsModel::persistentConnectionChanged, this, &ConnectionDialog::updatePersistentConnection);

    _pUi->connectionForm_2->setState(false);
    connect(_pUi->checkConn_2, &QCheckBox::stateChanged, _pUi->connectionForm_2, &ConnectionForm::setState);

    _pUi->connectionForm_3->setState(false);
    connect(_pUi->checkConn_3, &QCheckBox::stateChanged, _pUi->connectionForm_3, &ConnectionForm::setState);
}

ConnectionDialog::~ConnectionDialog()
{
    delete _pUi;
}

void ConnectionDialog::done(int r)
{
    Q_UNUSED(r);

    _pUi->connectionForm_1->fillSettingsModel(_pSettingsModel, SettingsModel::CONNECTION_ID_0);

    _pSettingsModel->setConnectionState(SettingsModel::CONNECTION_ID_2, _pUi->checkConn_2->checkState() == Qt::Checked);
    _pUi->connectionForm_2->fillSettingsModel(_pSettingsModel, SettingsModel::CONNECTION_ID_2);

    _pSettingsModel->setConnectionState(SettingsModel::CONNECTION_ID_3, _pUi->checkConn_3->checkState() == Qt::Checked);
    _pUi->connectionForm_3->fillSettingsModel(_pSettingsModel, SettingsModel::CONNECTION_ID_3);

    QDialog::done(r);
}

void ConnectionDialog::updateConnectionState(quint8 connectionId)
{
    switch(connectionId)
    {
    case SettingsModel::CONNECTION_ID_0:
        break;
    case SettingsModel::CONNECTION_ID_2:
        _pUi->checkConn_2->setChecked(_pSettingsModel->connectionState(connectionId));
        break;
    case SettingsModel::CONNECTION_ID_3:
        _pUi->checkConn_3->setChecked(_pSettingsModel->connectionState(connectionId));
        break;
    default:

        break;
    }
}

void ConnectionDialog::updateIp(quint8 connectionId)
{
    auto pConnectionSettings = connectionSettingsWidget(connectionId);

    pConnectionSettings->setIpAddress(_pSettingsModel->ipAddress(connectionId));
}

void ConnectionDialog::updatePort(quint8 connectionId)
{
    auto pConnectionSettings = connectionSettingsWidget(connectionId);

    pConnectionSettings->setPort(_pSettingsModel->port(connectionId));
}

void ConnectionDialog::updateConnectionType(quint8 connectionId)
{
    auto pConnectionSettings = connectionSettingsWidget(connectionId);

    pConnectionSettings->setConnectionType(_pSettingsModel->connectionType(connectionId));
}

void ConnectionDialog::updatePortName(quint8 connectionId)
{
    auto pConnectionSettings = connectionSettingsWidget(connectionId);

    pConnectionSettings->setPortName(_pSettingsModel->portName(connectionId));
}

void ConnectionDialog::updateParity(quint8 connectionId)
{
    auto pConnectionSettings = connectionSettingsWidget(connectionId);

    pConnectionSettings->setParity(_pSettingsModel->parity(connectionId));
}

void ConnectionDialog::updateBaudrate(quint8 connectionId)
{
    auto pConnectionSettings = connectionSettingsWidget(connectionId);

    pConnectionSettings->setBaudrate(_pSettingsModel->baudrate(connectionId));
}

void ConnectionDialog::updateDatabits(quint8 connectionId)
{
    auto pConnectionSettings = connectionSettingsWidget(connectionId);

    pConnectionSettings->setDatabits(_pSettingsModel->databits(connectionId));
}

void ConnectionDialog::updateStopbits(quint8 connectionId)
{
    auto pConnectionSettings = connectionSettingsWidget(connectionId);

    pConnectionSettings->setStopbits(_pSettingsModel->stopbits(connectionId));
}

void ConnectionDialog::updateSlaveId(quint8 connectionId)
{
    auto pConnectionSettings = connectionSettingsWidget(connectionId);

    pConnectionSettings->setSlaveId(_pSettingsModel->slaveId(connectionId));
}

void ConnectionDialog::updateTimeout(quint8 connectionId)
{
    auto pConnectionSettings = connectionSettingsWidget(connectionId);

    pConnectionSettings->setTimeout(_pSettingsModel->timeout(connectionId));
}

void ConnectionDialog::updateConsecutiveMax(quint8 connectionId)
{
    auto pConnectionSettings = connectionSettingsWidget(connectionId);

    pConnectionSettings->setConsecutiveMax(_pSettingsModel->consecutiveMax(connectionId));
}

void ConnectionDialog::updateInt32LittleEndian(quint8 connectionId)
{
    auto pConnectionSettings = connectionSettingsWidget(connectionId);

    pConnectionSettings->setInt32LittleEndian(_pSettingsModel->int32LittleEndian(connectionId));
}

void ConnectionDialog::updatePersistentConnection(quint8 connectionId)
{
    auto pConnectionSettings = connectionSettingsWidget(connectionId);

    pConnectionSettings->setPersistentConnection(_pSettingsModel->persistentConnection(connectionId));
}

ConnectionForm* ConnectionDialog::connectionSettingsWidget(quint8 connectionId)
{
    ConnectionForm* retRef;
    switch(connectionId)
    {
    case SettingsModel::CONNECTION_ID_0:
        retRef = _pUi->connectionForm_1;
        break;
    case SettingsModel::CONNECTION_ID_2:
        retRef = _pUi->connectionForm_2;
        break;
    case SettingsModel::CONNECTION_ID_3:
        retRef = _pUi->connectionForm_3;
        break;
    default:
        retRef = _pUi->connectionForm_1;
        break;
    }

    return retRef;
}
