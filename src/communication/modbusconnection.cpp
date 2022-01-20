
#include <QVariant>
#include <QtSerialPort/QSerialPort>
#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>

#include "scopelogging.h"
#include "modbusconnection.h"

/*!
 * Constructor for ModbusConnection module
 */
ModbusConnection::ModbusConnection(QObject *parent) : QObject(parent)
{
    _bWaitingForConnection = false;
}

/*!
 * Start opening of TCP connection
 * Emits signals (\ref connectionSuccess, \ref errorOccurred) when connection is ready or failed
 *
 * \param[in]   tcpSettings     TCP setting for server
 * \param[in]   timeout         Timeout of connection (in seconds)
 */
void ModbusConnection::openTcpConnection(struct TcpSettings tcpSettings, quint32 timeout)
{
    if (prepareConnectionOpen())
    {
        auto connectionData = QPointer<ConnectionData>(new ConnectionData(new QModbusTcpClient()));

        connectionData->pModbusClient->setConnectionParameter(QModbusDevice::NetworkAddressParameter, QVariant(tcpSettings.ip));
        connectionData->pModbusClient->setConnectionParameter(QModbusDevice::NetworkPortParameter, QVariant(tcpSettings.port));

        openConnection(connectionData, timeout);
    }
}

/*!
 * Start opening of serial connection
 * Emits signals (\ref connectionSuccess, \ref errorOccurred) when connection is ready or failed
 *
 * \param[in]   tcpSettings     Serial setting for server
 * \param[in]   timeout         Timeout of connection (in seconds)
 */
void ModbusConnection::openSerialConnection(struct SerialSettings serialSettings, quint32 timeout)
{
    if (prepareConnectionOpen())
    {
        auto connectionData = QPointer<ConnectionData>(new ConnectionData(new QModbusRtuSerialMaster()));

        connectionData->pModbusClient->setConnectionParameter(QModbusDevice::SerialPortNameParameter, QVariant(serialSettings.portName));
        connectionData->pModbusClient->setConnectionParameter(QModbusDevice::SerialParityParameter, QVariant(serialSettings.parity));
        connectionData->pModbusClient->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, QVariant(serialSettings.baudrate));
        connectionData->pModbusClient->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, QVariant(serialSettings.databits));
        connectionData->pModbusClient->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, QVariant(serialSettings.stopbits));

        openConnection(connectionData, timeout);
    }
}



/*!
 *  Close connection
 */
void ModbusConnection::closeConnection(void)
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
void ModbusConnection::sendReadRequest(quint32 regAddress, quint16 size, int serverAddress)
{
    if (isConnected())
    {
        QModbusDataUnit _dataUnit(QModbusDataUnit::HoldingRegisters, static_cast<int>(regAddress - 40001), size);
        _connectionList.last()->pReply = _connectionList.last()->pModbusClient->sendReadRequest(_dataUnit, serverAddress);

        connect(_connectionList.last()->pReply, &QModbusReply::finished, this, &ModbusConnection::handleRequestFinished);
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
 * Hande change of internal connection object
 *
 * \param state New state of connection
 */
void ModbusConnection::handleConnectionStateChanged(QModbusDevice::State state)
{
    QModbusClient * pClient = qobject_cast<QModbusClient *>(QObject::sender());
    const qint32 senderIdx = findConnectionData(nullptr, pClient);

    if (senderIdx != -1)
    {
        qCDebug(scopeCommConnection) << "Connection state change: " << _connectionList[senderIdx] << ", state: " << state;
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
    QModbusClient * pClient = qobject_cast<QModbusClient *>(QObject::sender());
    const qint32 senderIdx = findConnectionData(nullptr, pClient);

    // Only handle error is latest connection, the rest is automaticaly closed on state change
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
    QTimer * pTimeoutTimer = qobject_cast<QTimer *>(QObject::sender());
    const qint32 senderIdx = findConnectionData(pTimeoutTimer, nullptr);

    /* Stop timer */
    pTimeoutTimer->stop();

    // Only handle error is latest connection, the rest is automaticaly closed on state change
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
    QModbusReply * pReply = qobject_cast<QModbusReply *>(QObject::sender());
     auto err = pReply->error();

     // Start deletion of reply object before handling data (and closing connection)
     pReply->deleteLater();

     /* Check if reply is for valid connection (the last) */
     if (pReply == _connectionList.last()->pReply)
     {
         if (err == QModbusDevice::NoError)
         {
             // Success
             QModbusDataUnit dataUnit = pReply->result();
             emit readRequestSuccess(static_cast<quint32>(dataUnit.startAddress()) + 40001, dataUnit.values().toList());
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

        closeConnection();

        emit connectionError(QModbusDevice::ConnectionError, errMsg);
    }
}

/*!
 * Find specific ConnectionData instance in list based on QTimer or QModbusClient pointer
 * \param pTimer Pointer to QTimer object to find (nullptr when ignored)
 * \param pClient Pointer to QModbusClient oject to find (nullptr when ignored)
 * \retval -1       Not found
 * \retval != -1    Index in list
 */
qint32 ModbusConnection::findConnectionData(QTimer * pTimer, QModbusClient * pClient)
{
    for(qint32 idx = 0; idx < _connectionList.size(); idx++)
    {
        if (
            (pTimer != nullptr)
            && (pTimer == &_connectionList[idx]->connectionTimeoutTimer)
        )
        {
            return idx;
        }
        else if (
                 (pClient != nullptr)
                 && (pClient == _connectionList[idx]->pModbusClient)
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

void ModbusConnection::openConnection(QPointer<ConnectionData> connectionData, quint32 timeout)
{
    connectionData->pModbusClient->setNumberOfRetries(0);
    connectionData->pModbusClient->setTimeout(static_cast<int>(timeout));

    connect(&connectionData->connectionTimeoutTimer, &QTimer::timeout, this, &ModbusConnection::connectionTimeOut);
    connect(connectionData->pModbusClient, &QModbusClient::stateChanged, this, &ModbusConnection::handleConnectionStateChanged);
    connect(connectionData->pModbusClient, &QModbusClient::errorOccurred, this, &ModbusConnection::handleConnectionErrorOccurred);

    _connectionList.append(connectionData);

    qCDebug(scopeCommConnection) << "Connection start: " << _connectionList.last();

    _connectionList.last()->connectionTimeoutTimer.start(static_cast<int>(timeout));
    _bWaitingForConnection = true;

    if (!_connectionList.last()->pModbusClient->connectDevice())
    {
        auto pClient = _connectionList.last()->pModbusClient;
        handleConnectionError(_connectionList.last(), QString("Connect failed: %0").arg(pClient->error()));
    }
}

