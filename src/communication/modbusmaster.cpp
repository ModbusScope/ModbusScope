#include "modbusmaster.h"

#include "communication/modbusconnection.h"
#include "communication/readregisters.h"
#include "models/settingsmodel.h"
#include "util/modbusresultmap.h"

Q_DECLARE_METATYPE(Result<quint16>);

using State = ResultState::State;

ModbusMaster::ModbusMaster(SettingsModel* pSettingsModel, connectionId_t connectionId)
    : QObject(nullptr), _connectionId(connectionId), _pSettingsModel(pSettingsModel)
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

void ModbusMaster::readRegisterList(QList<ModbusAddress> registerList, quint8 consecutiveMax)
{
    auto connData = _pSettingsModel->connectionSettings(_connectionId);
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

        _readRegisters.resetRead(registerList, consecutiveMax);

        /* Open connection */
        if (connData->connectionType() == Connection::TYPE_SERIAL)
        {
            struct ModbusConnection::SerialSettings serialSettings = {
                .portName = connData->portName(),
                .parity = connData->parity(),
                .baudrate = connData->baudrate(),
                .databits = connData->databits(),
                .stopbits = connData->stopbits(),
            };
            _modbusConnection.openSerialConnection(serialSettings, connData->timeout());
        }
        else
        {
            struct ModbusConnection::TcpSettings tcpSettings = {
                .ip = connData->ipAddress(),
                .port = connData->port(),
            };
            _modbusConnection.openTcpConnection(tcpSettings, connData->timeout());
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
    if (_pSettingsModel->connectionSettings(_connectionId)->persistentConnection())
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

        logInfo("Partial list read: " + QString("Start address (%0) and count (%1)").arg(readItem.address().toString()).arg(readItem.count()));

        _modbusConnection.sendReadRequest(readItem.address(), readItem.count());
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
        bcloseConnection = !_pSettingsModel->connectionSettings(_connectionId)->persistentConnection();
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

    dStream << &list;

    return str;
}

void ModbusMaster::logResults(ModbusResultMap const &results)
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
