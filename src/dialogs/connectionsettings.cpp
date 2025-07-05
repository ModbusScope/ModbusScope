#include "connectionsettings.h"
#include "dialogs/ui_connectionsettings.h"

#include "models/settingsmodel.h"

ConnectionSettings::ConnectionSettings(SettingsModel* pSettingsModel, QWidget* parent)
    : QWidget(parent), _pUi(new Ui::ConnectionSettings), _pSettingsModel(pSettingsModel)
{
    _pUi->setupUi(this);

    _pUi->connectionForm_2->setState(false);
    connect(_pUi->checkConn_2, &QCheckBox::stateChanged, _pUi->connectionForm_2, &ConnectionForm::setState);

    _pUi->connectionForm_3->setState(false);
    connect(_pUi->checkConn_3, &QCheckBox::stateChanged, _pUi->connectionForm_3, &ConnectionForm::setState);

    for (uint8_t idx = 0; idx < Connection::ID_CNT; idx++)
    {
        updateConnectionSettings(idx);
    }
}

ConnectionSettings::~ConnectionSettings()
{
    delete _pUi;
}

void ConnectionSettings::acceptValues()
{
    _pUi->connectionForm_1->fillSettingsModel(_pSettingsModel, Connection::ID_1);

    _pSettingsModel->setConnectionState(Connection::ID_2, _pUi->checkConn_2->checkState() == Qt::Checked);
    _pUi->connectionForm_2->fillSettingsModel(_pSettingsModel, Connection::ID_2);

    _pSettingsModel->setConnectionState(Connection::ID_3, _pUi->checkConn_3->checkState() == Qt::Checked);
    _pUi->connectionForm_3->fillSettingsModel(_pSettingsModel, Connection::ID_3);
}

void ConnectionSettings::updateConnectionSettings(quint8 connectionId)
{
    updateConnectionState(connectionId);

    auto pConnectionSettings = connectionSettingsWidget(connectionId);
    pConnectionSettings->setIpAddress(_pSettingsModel->ipAddress(connectionId));
    pConnectionSettings->setPort(_pSettingsModel->port(connectionId));
    pConnectionSettings->setConnectionType(_pSettingsModel->connectionType(connectionId));
    pConnectionSettings->setPortName(_pSettingsModel->portName(connectionId));
    pConnectionSettings->setParity(_pSettingsModel->parity(connectionId));
    pConnectionSettings->setBaudrate(_pSettingsModel->baudrate(connectionId));
    pConnectionSettings->setDatabits(_pSettingsModel->databits(connectionId));
    pConnectionSettings->setStopbits(_pSettingsModel->stopbits(connectionId));
    pConnectionSettings->setSlaveId(_pSettingsModel->slaveId(connectionId));
    pConnectionSettings->setTimeout(_pSettingsModel->timeout(connectionId));
    pConnectionSettings->setConsecutiveMax(_pSettingsModel->consecutiveMax(connectionId));
    pConnectionSettings->setInt32LittleEndian(_pSettingsModel->int32LittleEndian(connectionId));
    pConnectionSettings->setPersistentConnection(_pSettingsModel->persistentConnection(connectionId));
}

void ConnectionSettings::updateConnectionState(quint8 connectionId)
{
    switch (connectionId)
    {
    case Connection::ID_1:
        break;
    case Connection::ID_2:
        _pUi->checkConn_2->setChecked(_pSettingsModel->connectionState(connectionId));
        break;
    case Connection::ID_3:
        _pUi->checkConn_3->setChecked(_pSettingsModel->connectionState(connectionId));
        break;
    default:
        break;
    }
}

ConnectionForm* ConnectionSettings::connectionSettingsWidget(quint8 connectionId)
{
    ConnectionForm* retRef;
    switch (connectionId)
    {
    case Connection::ID_1:
        retRef = _pUi->connectionForm_1;
        break;
    case Connection::ID_2:
        retRef = _pUi->connectionForm_2;
        break;
    case Connection::ID_3:
        retRef = _pUi->connectionForm_3;
        break;
    default:
        retRef = _pUi->connectionForm_1;
        break;
    }

    return retRef;
}
