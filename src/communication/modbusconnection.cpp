
#include <QVariant>
#include <QLoggingCategory>
#include "modbusconnection.h"

/*!
 * Constructor for ModbusConnection module
 */
ModbusConnection::ModbusConnection(QObject *parent) : QObject(parent)
{
#if 0
    // Enable to have internal QModbus debug messages
    QLoggingCategory::setFilterRules(QStringLiteral("qt.modbus* = true"));
#endif
}

/*!
 * Start opening of connection
 * Emits signals (\ref connectionSuccess, \ref errorOccurred) when connection is ready or failed
 *
 * \param[in]   ip          IP address of server
 * \param[in]   port        Port on the server
 * \param[in]   timeout     Timeout of connection (in seconds)
 */
void ModbusConnection::openConnection(QString ip, qint32 port, quint32 timeout)
{
    if (state() == QModbusDevice::ConnectedState)
    {
        // Already connected and ready
        emit connectionSuccess();
    }
    else
    {
        auto connectionData = QPointer<ConnectionData>(new ConnectionData());

        connect(&connectionData->timeoutTimer, &QTimer::timeout, this, &ModbusConnection::connectionTimeOut);
        connect(&connectionData->modbusClient, &QModbusTcpClient::stateChanged, this, &ModbusConnection::handleConnectionStateChanged);
        connect(&connectionData->modbusClient, &QModbusTcpClient::errorOccurred, this, &ModbusConnection::handleConnectionErrorOccurred);

        connectionData->modbusClient.setNumberOfRetries(0);
        connectionData->modbusClient.setTimeout(static_cast<int>(timeout));

        connectionData->modbusClient.setConnectionParameter(QModbusDevice::NetworkAddressParameter, QVariant(ip));
        connectionData->modbusClient.setConnectionParameter(QModbusDevice::NetworkPortParameter, QVariant(port));

        _connectionList.append(QPointer<ConnectionData>(connectionData));

        if (_connectionList.last()->modbusClient.connectDevice())
        {
            _bWaitingForConnection = true;
            _connectionList.last()->timeoutTimer.start(static_cast<int>(timeout));
        }
        else
        {
            auto pClient = &_connectionList.last()->modbusClient;
            handleError(_connectionList.last(), QString("Connect failed: %0").arg(pClient->error()));
        }
    }
}

/*!
 *  Close connection
 */
void ModbusConnection::closeConnection(void)
{
    if (
            !_connectionList.isEmpty()
            && _connectionList.last()->modbusClient.state() != QModbusDevice::UnconnectedState
        )
    {
        _connectionList.last()->timeoutTimer.stop();
        _connectionList.last()->modbusClient.disconnectDevice();
    }
}

/*!
 * Send read request over connection
 *
 * \param read
 * \param serverAddress     slave address
 * \retval Pointer to QModbusReply object
 * \retval nullptr when connection is not valid
 */
QModbusReply * ModbusConnection::sendReadRequest(const QModbusDataUnit &read, int serverAddress)
{
    if (state() == QModbusDevice::ConnectedState)
    {
        return _connectionList.last()->modbusClient.sendReadRequest(read, serverAddress);
    }
    else
    {
        emit errorOccurred(QModbusDevice::ReadError, QString("Not connected"));
    }

    return nullptr;
}

/*!
 *  Get state of connection
 *
 * \return State of connection (\ref QModbusDevice::State)
 */
QModbusDevice::State ModbusConnection::state(void)
{
    if (_connectionList.isEmpty())
    {
        return QModbusDevice::UnconnectedState;
    }
    else
    {
        return _connectionList.last()->modbusClient.state();
    }
}

/*!
 * Hande change of internal connection object
 *
 * \param state New state of connection
 */
void ModbusConnection::handleConnectionStateChanged(QModbusDevice::State state)
{
    QModbusTcpClient * pClient = qobject_cast<QModbusTcpClient *>(QObject::sender());    
    const qint32 senderIdx = findConnectionData(nullptr, pClient);

    if (state == QModbusDevice::ConnectingState)
    {
        // Wait for connection or error
    }
    else if (state == QModbusDevice::ConnectedState)
    {

        if (senderIdx != -1)
        {
            // Stop timeout counter
            _connectionList[senderIdx]->timeoutTimer.stop();
        }

        if (senderIdx == _connectionList.size() - 1)
        {
            // Most recent connection is opened
            emit connectionSuccess();

            _bWaitingForConnection = false;
        }
        else if (senderIdx != -1)
        {
            // Stale connection is open, close it
            _connectionList[senderIdx]->modbusClient.disconnectDevice();
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
                handleError(_connectionList.last(), QString("Connection timeout"));
            }
        }
        else if (senderIdx != -1)
        {
            // Prepare to remove old connection
            _connectionList[senderIdx]->modbusClient.disconnect();
            _connectionList[senderIdx]->timeoutTimer.disconnect();

            connect(_connectionList[senderIdx], &ConnectionData::destroyed, this, &ModbusConnection::connectionDestroyed);

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
    QModbusTcpClient * pClient = qobject_cast<QModbusTcpClient *>(QObject::sender());
    const qint32 senderIdx = findConnectionData(nullptr, pClient);

    // Only handle error is latest connection, the rest is automaticaly closed on state change
    if (senderIdx == _connectionList.size() - 1)
    {
        handleError(_connectionList.last(), QString("Error: %0").arg(error));
    }
}

/*!
 * Handle time out of connection start
 */
void ModbusConnection::connectionTimeOut()
{
    QModbusTcpClient * pClient = qobject_cast<QModbusTcpClient *>(QObject::sender());
    const qint32 senderIdx = findConnectionData(nullptr, pClient);

    // Only handle error is latest connection, the rest is automaticaly closed on state change
    if (senderIdx == _connectionList.size() - 1)
    {
        handleError(_connectionList.last(), QString("Connection timeout"));
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
 * General internal error handler
 * Should only be called for last connection in the list, the rest is stale
 * \param errMsg    Error message
 */
void ModbusConnection::handleError(QPointer<ConnectionData> connectionData, QString errMsg)
{

    if (!connectionData->bErrorHandled)
    {
        connectionData->bErrorHandled = true;

        closeConnection();

        emit errorOccurred(QModbusDevice::ConnectionError, errMsg);
    }
}

/*!
 * Find specific ConnectionData instance in list based on QTimer or QModbusTcpClient pointer
 * \param pTimer Pointer to QTimer object to find (nullptr when ignored)
 * \param pClient Pointer to QModbusTcpClient oject to find (nullptr when ignored)
 * \retval -1       Not found
 * \retval != -1    Index in list
 */
qint32 ModbusConnection::findConnectionData(QTimer * pTimer, QModbusTcpClient * pClient)
{
    for(qint32 idx = 0; idx < _connectionList.size(); idx++)
    {
        if (
            (pTimer != nullptr)
            && (pTimer == &_connectionList[idx]->timeoutTimer)
        )
        {
            return idx;
        }
        else if (
                 (pClient != nullptr)
                 && (pClient == &_connectionList[idx]->modbusClient)
            )
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

