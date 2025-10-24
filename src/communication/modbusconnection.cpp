
#include "modbusconnection.h"

#include "util/modbusdataunit.h"
#include "util/scopelogging.h"

#include <QModbusRtuSerialClient>
#include <QModbusTcpClient>
#include <QVariant>
#include <QtSerialPort/QSerialPort>

using RegisterType = QModbusDataUnit::RegisterType;
using ObjectType = ModbusAddress::ObjectType;

/*!
 * Constructor for ModbusConnection module
 */
ModbusConnection::ModbusConnection(QObject* parent) : QObject(parent)
{
    _bWaitingForConnection = false;
}

void ModbusConnection::configureTcpConnection(tcpSettings_t tcpSettings)
{
    _tcpSettings = tcpSettings;
    _connectionType = ConnectionTypes::TYPE_TCP;
}

void ModbusConnection::configureSerialConnection(serialSettings_t serialSettings)
{
    _serialSettings = serialSettings;
    _connectionType = ConnectionTypes::TYPE_SERIAL;
}

/*!
 * Start opening of connection
 * Emits signals (\ref connectionSuccess, \ref errorOccurred) when connection is ready or failed
 *
 * \param[in]   timeout         Timeout of connection (in seconds)
 */
void ModbusConnection::open(quint32 timeout)
{
    if (prepareConnectionOpen())
    {
        if (_connectionType == ConnectionTypes::TYPE_SERIAL)
        {
            auto connectionData =
              QPointer<ConnectionData>(new ConnectionData(std::make_unique<QModbusRtuSerialClient>()));

            connectionData->pModbusClient->setConnectionParameter(QModbusDevice::SerialPortNameParameter,
                                                                  QVariant(_serialSettings.portName));
            connectionData->pModbusClient->setConnectionParameter(QModbusDevice::SerialParityParameter,
                                                                  QVariant::fromValue(_serialSettings.parity));
            connectionData->pModbusClient->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,
                                                                  QVariant::fromValue(_serialSettings.baudrate));
            connectionData->pModbusClient->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,
                                                                  QVariant::fromValue(_serialSettings.databits));
            connectionData->pModbusClient->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,
                                                                  QVariant::fromValue(_serialSettings.stopbits));

            openConnection(connectionData, timeout);
        }
        else if (_connectionType == ConnectionTypes::TYPE_TCP)
        {
            auto connectionData = QPointer<ConnectionData>(new ConnectionData(std::make_unique<QModbusTcpClient>()));

            connectionData->pModbusClient->setConnectionParameter(QModbusDevice::NetworkAddressParameter,
                                                                  QVariant(_tcpSettings.ip));
            connectionData->pModbusClient->setConnectionParameter(QModbusDevice::NetworkPortParameter,
                                                                  QVariant(_tcpSettings.port));

            openConnection(connectionData, timeout);
        }
        else
        {
            qCDebug(scopeCommConnection) << "Unknown connection type";
            emit connectionError(QModbusDevice::ConnectionError, QString("Unknown connection type"));
        }
    }
}

/*!
 *  Close connection
 */
void ModbusConnection::close(void)
{
    if (!_connectionList.isEmpty())
    {
        qCDebug(scopeCommConnection) << "Connection close: " << _connectionList.last();
        _connectionList.last()->connectionTimeoutTimer.stop();
        _connectionList.last()->pModbusClient->disconnectDevice();
    }
}

/*!
 * Send read request over connection
 *
 * \param regAddress    register address
 * \param size          number of registers
 * \param serverAddress     slave address
 */
void ModbusConnection::sendReadRequest(ModbusDataUnit regAddress, quint16 size)
{
    if (isConnected())
    {
        auto type = registerType(regAddress.objectType());
        QModbusDataUnit dataUnit(type, static_cast<int>(regAddress.protocolAddress()), size);
        _connectionList.last()->pReply =
          _connectionList.last()->pModbusClient->sendReadRequest(dataUnit, regAddress.slaveId());

        connect(_connectionList.last()->pReply, &QModbusReply::finished, this,
                &ModbusConnection::handleRequestFinished);
    }
    else
    {
        emit connectionError(QModbusDevice::ReadError, QString("Not connected"));
    }
}

/*!
 *  Return whether connection is ok
 *
 * \return Connection state
 */
bool ModbusConnection::isConnected(void)
{
    if (_connectionList.isEmpty())
    {
        return false;
    }
    else if (_connectionList.last()->pModbusClient->state() != QModbusDevice::ConnectedState)
    {
        return false;
    }
    else
    {
        return true;
    }
}

/*!
 * Handle change of internal connection object
 *
 * \param state New state of connection
 */
void ModbusConnection::handleConnectionStateChanged(QModbusDevice::State state)
{
    QModbusClient* pClient = qobject_cast<QModbusClient*>(QObject::sender());
    const qint32 senderIdx = findConnectionData(nullptr, pClient);

    if (senderIdx != -1)
    {
        qCDebug(scopeCommConnection) << "Connection state change: " << _connectionList[senderIdx]
                                     << ", state: " << state;
    }
    else
    {
        qCDebug(scopeCommConnection) << "Connection state change: Unknown connection id" << ", state: " << state;
    }

    if (state == QModbusDevice::ConnectingState)
    {
        // Wait for connection or error
    }
    else if (state == QModbusDevice::ConnectedState)
    {

        if (senderIdx != -1)
        {
            // Stop timeout counter
            _connectionList[senderIdx]->connectionTimeoutTimer.stop();
        }

        if (senderIdx == _connectionList.size() - 1)
        {
            _bWaitingForConnection = false;

            // Most recent connection is opened
            emit connectionSuccess();
        }
        else if (senderIdx != -1)
        {
            // Stale connection is open, close it
            _connectionList[senderIdx]->pModbusClient->disconnectDevice();
        }
        else
        {
            // Nothing to do
        }
    }
    else if (state == QModbusDevice::UnconnectedState)
    {
        // Unconnected state can occur before timeout when connection is refused
        if (_bWaitingForConnection)
        {
            if (senderIdx == _connectionList.size() - 1)
            {
                handleConnectionError(_connectionList.last(), QString("Connection timeout"));
            }
        }
        else if (senderIdx != -1)
        {
            // Prepare to remove old connection
            _connectionList[senderIdx]->pModbusClient->disconnect();
            _connectionList[senderIdx]->connectionTimeoutTimer.disconnect();

            connect(_connectionList[senderIdx], &ConnectionData::destroyed, this,
                    &ModbusConnection::connectionDestroyed);

            // Use deleteLater because otherwise we receive SIGSEGV error
            _connectionList[senderIdx]->deleteLater();
        }
    }
    else if (state == QModbusDevice::ClosingState)
    {
        // Wait for final state
    }
    else
    {
        // Do nothing
    }
}

/*!
 * Handle error on connecting
 * \param error The error
 */
void ModbusConnection::handleConnectionErrorOccurred(QModbusDevice::Error error)
{
    QModbusClient* pClient = qobject_cast<QModbusClient*>(QObject::sender());
    const qint32 senderIdx = findConnectionData(nullptr, pClient);

    // Only handle error is latest connection, the rest is automatically closed on state change
    if (senderIdx == _connectionList.size() - 1)
    {
        handleConnectionError(_connectionList.last(), QString("Error: %0").arg(error));
    }
}

/*!
 * Handle time out of connection start
 */
void ModbusConnection::connectionTimeOut()
{
    QTimer* pTimeoutTimer = qobject_cast<QTimer*>(QObject::sender());
    const qint32 senderIdx = findConnectionData(pTimeoutTimer, nullptr);

    /* Stop timer */
    pTimeoutTimer->stop();

    // Only handle error is latest connection, the rest is automatically closed on state change
    if (senderIdx == _connectionList.size() - 1)
    {
        handleConnectionError(_connectionList.last(), QString("Connection timeout"));
    }
}

/*!
 * Handle deletion of connection object
 */
void ModbusConnection::connectionDestroyed()
{
    // Remove all nullptr from list
    _connectionList.removeAll(nullptr);
}

/*!
 * Handle request finished
 */
void ModbusConnection::handleRequestFinished()
{
    QModbusReply* pReply = qobject_cast<QModbusReply*>(QObject::sender());
    auto err = pReply->error();

    // Start deletion of reply object before handling data (and closing connection)
    pReply->deleteLater();

    /* Check if reply is for valid connection (the last) */
    if (pReply == _connectionList.last()->pReply)
    {
        if (err == QModbusDevice::NoError)
        {
            QModbusDataUnit dataUnit = pReply->result();
            auto addr =
              ModbusDataUnit(static_cast<quint16>(dataUnit.startAddress()), objectType(dataUnit.registerType()),
                             static_cast<ModbusDataUnit::slaveId_t>(pReply->serverAddress()));
            emit readRequestSuccess(addr, dataUnit.values().toList());
        }
        else if (err == QModbusDevice::ProtocolError)
        {
            auto exceptionCode = pReply->rawResult().exceptionCode();

            emit readRequestProtocolError(exceptionCode);
        }
        else
        {
            emit readRequestError(pReply->errorString(), pReply->error());
        }
    }
    else
    {
        // ignore data from reply
    }
}

/*!
 * \brief Prepare for opening a connection
 * \retval  true        when connection needs to be opened
 * \retval  false       when connection already opened
 */
bool ModbusConnection::prepareConnectionOpen()
{
    bool bRet;

    if (isConnected())
    {
        bRet = false;

        qCDebug(scopeCommConnection) << "Connection already open";

        // Already connected and ready
        emit connectionSuccess();
    }
    else
    {
        bRet = true;
    }

    return bRet;
}

RegisterType ModbusConnection::registerType(ObjectType type)
{
    switch (type)
    {
    case ObjectType::COIL:
        return RegisterType::Coils;
    case ObjectType::DISCRETE_INPUT:
        return RegisterType::DiscreteInputs;
    case ObjectType::INPUT_REGISTER:
        return RegisterType::InputRegisters;
    case ObjectType::HOLDING_REGISTER:
        return RegisterType::HoldingRegisters;
    case ObjectType::UNKNOWN:
        return RegisterType::HoldingRegisters;
    default:
        return RegisterType::HoldingRegisters;
    }
}

ObjectType ModbusConnection::objectType(RegisterType type)
{
    switch (type)
    {
    case RegisterType::Coils:
        return ObjectType::COIL;
    case RegisterType::DiscreteInputs:
        return ObjectType::DISCRETE_INPUT;
    case RegisterType::InputRegisters:
        return ObjectType::INPUT_REGISTER;
    case RegisterType::HoldingRegisters:
        return ObjectType::HOLDING_REGISTER;
    default:
        return ObjectType::UNKNOWN;
    }
}

/*!
 * General internal error handler
 * Should only be called for last connection in the list, the rest is stale
 * \param errMsg    Error message
 */
void ModbusConnection::handleConnectionError(QPointer<ConnectionData> connectionData, QString errMsg)
{
    qCDebug(scopeCommConnection) << "Connection error:" << errMsg;

    if (!connectionData->bConnectionErrorHandled)
    {
        connectionData->bConnectionErrorHandled = true;

        close();

        emit connectionError(QModbusDevice::ConnectionError, errMsg);
    }
}

/*!
 * Find specific ConnectionData instance in list based on QTimer or QModbusClient pointer
 * \param pTimer Pointer to QTimer object to find (nullptr when ignored)
 * \param pClient Pointer to QModbusClient object to find (nullptr when ignored)
 * \retval -1       Not found
 * \retval != -1    Index in list
 */
qint32 ModbusConnection::findConnectionData(QTimer* pTimer, QModbusClient* pClient)
{
    for (qint32 idx = 0; idx < _connectionList.size(); idx++)
    {
        if ((pTimer != nullptr) && (pTimer == &_connectionList[idx]->connectionTimeoutTimer))
        {
            return idx;
        }
        else if ((pClient != nullptr) && (pClient == _connectionList[idx]->pModbusClient.get()))
        {
            return idx;
        }
        else
        {
            // Check next
        }
    }

    return -1;
}

void ModbusConnection::openConnection(QPointer<ConnectionData> connectionData, quint32 timeout)
{
    connectionData->pModbusClient->setNumberOfRetries(0);
    connectionData->pModbusClient->setTimeout(static_cast<int>(timeout));

    connect(&connectionData->connectionTimeoutTimer, &QTimer::timeout, this, &ModbusConnection::connectionTimeOut);
    connect(connectionData->pModbusClient.get(), &QModbusClient::stateChanged, this,
            &ModbusConnection::handleConnectionStateChanged);
    connect(connectionData->pModbusClient.get(), &QModbusClient::errorOccurred, this,
            &ModbusConnection::handleConnectionErrorOccurred);

    _connectionList.append(connectionData);

    qCDebug(scopeCommConnection) << "Connection start: " << _connectionList.last();

    _connectionList.last()->connectionTimeoutTimer.start(static_cast<int>(timeout));
    _bWaitingForConnection = true;

    if (!_connectionList.last()->pModbusClient->connectDevice())
    {
        auto pClient = _connectionList.last()->pModbusClient.get();
        handleConnectionError(_connectionList.last(), QString("Connect failed: %0").arg(pClient->error()));
    }
}
