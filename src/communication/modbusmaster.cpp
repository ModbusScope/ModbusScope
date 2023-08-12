#include "modbusmaster.h"
#include "modbusresultmap.h"
#include "settingsmodel.h"
#include "modbusconnection.h"
#include "readregisters.h"

#include <util.h>

Q_DECLARE_METATYPE(Result<quint16>);

using State = ResultState::State;

ModbusMaster::ModbusMaster(SettingsModel * pSettingsModel, quint8 connectionId) : QObject(nullptr), _connectionId(connectionId), _pSettingsModel(pSettingsModel)
{
    qMetaTypeId<Result<quint16> >();

    // Use queued connection to make sure reply is deleted before closing connection
    connect(this, &ModbusMaster::triggerNextRequest, this, &ModbusMaster::handleTriggerNextRequest, Qt::QueuedConnection);

    connect(&_modbusConnection, &ModbusConnection::connectionSuccess, this, &ModbusMaster::handleConnectionOpened);
    connect(&_modbusConnection, &ModbusConnection::connectionError, this, &ModbusMaster::handlerConnectionError);
    connect(&_modbusConnection, &ModbusConnection::readRequestSuccess, this, &ModbusMaster::handleRequestSuccess);
    connect(&_modbusConnection, &ModbusConnection::readRequestProtocolError, this, &ModbusMaster::handleRequestProtocolError);
    connect(&_modbusConnection, &ModbusConnection::readRequestError, this, &ModbusMaster::handleRequestError);
}

ModbusMaster::~ModbusMaster()
{
    _modbusConnection.disconnect();
    _modbusConnection.closeConnection();
}

void ModbusMaster::readRegisterList(QList<ModbusAddress> registerList)
{
    if (_pSettingsModel->connectionState(_connectionId) == false)
    {
        ModbusResultMap errMap;

        for (int i = 0; i < registerList.size(); i++)
        {
            const auto result = Result<quint16>(0, State::INVALID);
            errMap.insert(registerList.at(i), result);
        }

        logError(QStringLiteral("Read failed because connection is disabled"));
        logResults(errMap);
    }
    else if (registerList.size() > 0)
    {
        logInfo("Register list read: " + dumpToString(registerList));

        _readRegisters.resetRead(registerList, _pSettingsModel->consecutiveMax(_connectionId));

        /* Open connection */
        if (_pSettingsModel->connectionType(_connectionId) == Connection::TYPE_SERIAL)
        {
            struct ModbusConnection::SerialSettings serialSettings =
            {
                .portName = _pSettingsModel->portName(_connectionId),
                .parity = _pSettingsModel->parity(_connectionId),
                .baudrate = _pSettingsModel->baudrate(_connectionId),
                .databits = _pSettingsModel->databits(_connectionId),
                .stopbits = _pSettingsModel->stopbits(_connectionId),
            };
            _modbusConnection.openSerialConnection(serialSettings, _pSettingsModel->timeout(_connectionId));
        }
        else
        {
            struct ModbusConnection::TcpSettings tcpSettings =
            {
                .ip = _pSettingsModel->ipAddress(_connectionId),
                .port = _pSettingsModel->port(_connectionId),
            };
            _modbusConnection.openTcpConnection(tcpSettings, _pSettingsModel->timeout(_connectionId));
        }
    }
    else
    {
        ModbusResultMap emptyResults;
        emit modbusPollDone(emptyResults, _connectionId);
    }
}

void ModbusMaster::cleanUp()
{
    /* Close connection when not closing automatically */
    if (_pSettingsModel->persistentConnection(_connectionId))
    {
        _modbusConnection.closeConnection();
    }
}

void ModbusMaster::handleConnectionOpened()
{
    emit triggerNextRequest();
}

void ModbusMaster::handlerConnectionError(QModbusDevice::Error error, QString msg)
{
    Q_UNUSED(error);

    logError(QString("Connection error: ") + msg);

    _readRegisters.addAllErrors();

    finishRead(true);
}

void ModbusMaster::handleRequestSuccess(ModbusAddress startRegister, QList<quint16> registerDataList)
{
    logInfo(QString("Read success"));

    // Success
    _readRegisters.addSuccess(startRegister, registerDataList);

    // Start next read
    emit triggerNextRequest();
}

void ModbusMaster::handleRequestProtocolError(QModbusPdu::ExceptionCode exceptionCode)
{
    logError(QString("Modbus Exception: %0").arg(exceptionCode));

    if (
        (exceptionCode == QModbusPdu::IllegalDataAddress)
        || (exceptionCode == QModbusPdu::IllegalDataValue)
        )
    {
        if (_readRegisters.next().count() > 1)
        {
            // Split read into separate reads on specific exception code and count is more than 1
            _readRegisters.splitNextToSingleReads();
        }
        else
        {
            // Add error to results
            _readRegisters.addError();
        }
    }
    else if (exceptionCode == QModbusPdu::IllegalFunction)
    {
        // No need to continue this read
        _readRegisters.addAllErrors();
    }
    else
    {
        _readRegisters.addError();
    }

    // Start next read
    emit triggerNextRequest();
}

void ModbusMaster::handleRequestError(QString errorString, QModbusDevice::Error error)
{
    logError(QString("Request Failed:  %0 (%1)").arg(errorString).arg(error));

    // When we don't receive an exception, abort read and close connection
    _readRegisters.addAllErrors();

    // Start next read
    emit triggerNextRequest();
}

void ModbusMaster::handleTriggerNextRequest(void)
{
    if (_readRegisters.hasNext())
    {
        ModbusReadItem readItem = _readRegisters.next();

        logInfo("Partial list read: " + QString("Start address (%0) and count (%1)").arg(readItem.address()).arg(readItem.count()));

        _modbusConnection.sendReadRequest(readItem.address(), readItem.count(), _pSettingsModel->slaveId(_connectionId));
    }
    else
    {
        finishRead(false);
    }
}

void ModbusMaster::finishRead(bool bError)
{
    ModbusResultMap results = _readRegisters.resultMap();

    logResults(results);

    bool bcloseConnection;

    if (bError)
    {
        /* Always close connection on error */
        bcloseConnection = true;
    }
    else
    {
        bcloseConnection = !_pSettingsModel->persistentConnection(_connectionId);
    }

    if (bcloseConnection)
    {
        _modbusConnection.closeConnection();
    }
}

QString ModbusMaster::dumpToString(ModbusResultMap map) const
{
    QString str;
    QDebug dStream(&str);

    dStream << map;

    return str;
}

QString ModbusMaster::dumpToString(QList<ModbusAddress> list) const
{
    QString str;
    QDebug dStream(&str);

    dStream << list;

    return str;
}

void ModbusMaster::logResults(ModbusResultMap &results)
{
    logInfo("Result map: " + dumpToString(results));
    emit modbusPollDone(results, _connectionId);
}

void ModbusMaster::logInfo(QString msg)
{
    emit modbusLogInfo(QString("[Conn %0] %1").arg(_connectionId + 1).arg(msg));
}

void ModbusMaster::logError(QString msg)
{
    emit modbusLogError(QString("[Conn %0] %1").arg(_connectionId + 1).arg(msg));
}
