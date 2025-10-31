#include "connectionsettings.h"
#include "dialogs/ui_connectionsettings.h"

#include "models/settingsmodel.h"

using connectionId_t = ConnectionTypes::connectionId_t;

ConnectionSettings::ConnectionSettings(SettingsModel* pSettingsModel, QWidget* parent)
    : QWidget(parent), _pUi(new Ui::ConnectionSettings), _pSettingsModel(pSettingsModel)
{
    _pUi->setupUi(this);

    _pUi->connectionForm_2->setState(false);
    connect(_pUi->checkConn_2, &QCheckBox::stateChanged, _pUi->connectionForm_2, &ConnectionForm::setState);

    _pUi->connectionForm_3->setState(false);
    connect(_pUi->checkConn_3, &QCheckBox::stateChanged, _pUi->connectionForm_3, &ConnectionForm::setState);

    for (uint8_t idx = 0; idx < ConnectionTypes::ID_CNT; idx++)
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
    _pUi->connectionForm_1->fillSettingsModel(_pSettingsModel->connectionSettings(ConnectionTypes::ID_1));

    _pSettingsModel->setConnectionState(ConnectionTypes::ID_2, _pUi->checkConn_2->checkState() == Qt::Checked);
    _pUi->connectionForm_2->fillSettingsModel(_pSettingsModel->connectionSettings(ConnectionTypes::ID_2));

    _pSettingsModel->setConnectionState(ConnectionTypes::ID_3, _pUi->checkConn_3->checkState() == Qt::Checked);
    _pUi->connectionForm_3->fillSettingsModel(_pSettingsModel->connectionSettings(ConnectionTypes::ID_3));
}

void ConnectionSettings::updateConnectionSettings(connectionId_t connectionId)
{
    updateConnectionState(connectionId);
    auto pConnectionSettings = connectionSettingsWidget(connectionId);
    auto connData = _pSettingsModel->connectionSettings(connectionId);

    pConnectionSettings->setIpAddress(connData->ipAddress());
    pConnectionSettings->setPort(connData->port());
    pConnectionSettings->setConnectionType(connData->connectionType());
    pConnectionSettings->setPortName(connData->portName());
    pConnectionSettings->setParity(connData->parity());
    pConnectionSettings->setBaudrate(connData->baudrate());
    pConnectionSettings->setDatabits(connData->databits());
    pConnectionSettings->setStopbits(connData->stopbits());
    pConnectionSettings->setTimeout(connData->timeout());
    pConnectionSettings->setPersistentConnection(connData->persistentConnection());
}

void ConnectionSettings::updateConnectionState(connectionId_t connectionId)
{
    switch (connectionId)
    {
    case ConnectionTypes::ID_1:
        break;
    case ConnectionTypes::ID_2:
        _pUi->checkConn_2->setChecked(_pSettingsModel->connectionState(connectionId));
        break;
    case ConnectionTypes::ID_3:
        _pUi->checkConn_3->setChecked(_pSettingsModel->connectionState(connectionId));
        break;
    default:
        break;
    }
}

ConnectionForm* ConnectionSettings::connectionSettingsWidget(connectionId_t connectionId)
{
    ConnectionForm* retRef;
    switch (connectionId)
    {
    case ConnectionTypes::ID_1:
        retRef = _pUi->connectionForm_1;
        break;
    case ConnectionTypes::ID_2:
        retRef = _pUi->connectionForm_2;
        break;
    case ConnectionTypes::ID_3:
        retRef = _pUi->connectionForm_3;
        break;
    default:
        retRef = _pUi->connectionForm_1;
        break;
    }

    return retRef;
}
