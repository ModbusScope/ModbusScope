#include "modbusmaster.h"
#include "settingsmodel.h"
#include "errorlogmodel.h"
#include "modbusconnection.h"
#include "readregisters.h"

#include <util.h>

typedef QMap<quint16,ModbusResult> ModbusResultMap;
Q_DECLARE_METATYPE(ModbusResultMap);
Q_DECLARE_METATYPE(ModbusResult);

ModbusMaster::ModbusMaster(SettingsModel * pSettingsModel, quint8 connectionId) :
    QObject(nullptr)
{

    qMetaTypeId<ModbusResult>();
    qMetaTypeId<QMap<quint16, ModbusResult> >();

    _pSettingsModel = pSettingsModel;
    _pModbusConnection = new ModbusConnection();
    _pReadRegisters = new ReadRegisters();

    _connectionId = connectionId;
    _success = 0;
    _error = 0;

    // Use queued connection to make sure reply is deleted before closing connection
    connect(this, &ModbusMaster::triggerNextRequest, this, &ModbusMaster::handleTriggerNextRequest, Qt::QueuedConnection);

    // Connection signals/slots
    connect(_pModbusConnection, &ModbusConnection::connectionSuccess, this, &ModbusMaster::handleConnectionOpened);
    connect(_pModbusConnection, &ModbusConnection::connectionError, this, &ModbusMaster::handlerConnectionError);

    // Read request signals/slots
    connect(_pModbusConnection, &ModbusConnection::readRequestSuccess, this, &ModbusMaster::handleRequestSuccess);
    connect(_pModbusConnection, &ModbusConnection::readRequestProtocolError, this, &ModbusMaster::handleRequestProtocolError);
    connect(_pModbusConnection, &ModbusConnection::readRequestError, this, &ModbusMaster::handleRequestError);
}

ModbusMaster::~ModbusMaster()
{
    delete _pModbusConnection;
    delete _pReadRegisters;
}

void ModbusMaster::readRegisterList(QList<quint16> registerList)
{
    _success = 0;
    _error = 0;

    if (registerList.count() > 0)
    {
        logInfo("Register list read: " + dumpToString(registerList));

        _pReadRegisters->resetRead(registerList, _pSettingsModel->consecutiveMax(_connectionId));

        /* Open connection */
        _pModbusConnection->openConnection(_pSettingsModel->ipAddress(_connectionId), _pSettingsModel->port(_connectionId), _pSettingsModel->timeout(_connectionId));
    }
    else
    {
        QMap<quint16, ModbusResult> emptyResults;
        emit modbusPollDone(emptyResults, _connectionId);
    }
}

void ModbusMaster::cleanUp()
{
    /* Close connection when not closing automatically */
    if (_pSettingsModel->reuseConnection(_connectionId))
    {
        _pModbusConnection->closeConnection();
    }
}

void ModbusMaster::handleConnectionOpened()
{
    emit triggerNextRequest();
}

void ModbusMaster::handlerConnectionError(QModbusDevice::Error error, QString msg)
{
    Q_UNUSED(error);

    _error++;

    logError(QString("Connection error (fatal):") + msg);

    _pReadRegisters->addAllErrors();

    finishRead(true);
}

void ModbusMaster::handleRequestSuccess(quint16 startRegister, QList<quint16> registerDataList)
{
    logInfo(QString("Read success"));

    // Success
    _pReadRegisters->addSuccess(startRegister, registerDataList);

    _success++;

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
        if (_pReadRegisters->next().count() > 1)
        {
            // Split read into separate reads on specific exception code and count is more than 1
            _pReadRegisters->splitNextToSingleReads();
        }
        else
        {
            // Add error to results
            _pReadRegisters->addError();
        }
    }
    else if (exceptionCode == QModbusPdu::IllegalFunction)
    {
        // No need to continue this read
        _pReadRegisters->addAllErrors();
    }
    else
    {
        _pReadRegisters->addError();
    }

    _error++;

    // Start next read
    emit triggerNextRequest();
}

void ModbusMaster::handleRequestError(QString errorString, QModbusDevice::Error error)
{
    emit modbusLogError(QString("Request Failed:  %0 (%1)").arg(errorString).arg(error));

    // When we don't receive an exception, abort read and close connection
    _pReadRegisters->addAllErrors();

    _error++;

    // Start next read
    emit triggerNextRequest();
}

void ModbusMaster::handleTriggerNextRequest(void)
{
    if (_pReadRegisters->hasNext())
    {
        ModbusReadItem readItem = _pReadRegisters->next();

        logInfo("Partial list read: " + QString("Start address (%0) and count (%1)").arg(readItem.address()).arg(readItem.count()));

        _pModbusConnection->sendReadRequest(readItem.address(), readItem.count(), _pSettingsModel->slaveId(_connectionId));
    }
    else
    {
        // Done reading
        finishRead(false);
    }
}

void ModbusMaster::finishRead(bool bError)
{
    QMap<quint16, ModbusResult> results = _pReadRegisters->resultMap();

    logInfo("Result map: " + dumpToString(results));
    emit modbusAddToStats(_success, _error);
    emit modbusPollDone(results, _connectionId);

    bool bcloseConnection;

    if (bError)
    {
        /* Always close connection on error */
        bcloseConnection = true;
    }
    else
    {
        bcloseConnection = !_pSettingsModel->reuseConnection(_connectionId);
    }

    if (bcloseConnection)
    {
        _pModbusConnection->closeConnection();
    }
}

QString ModbusMaster::dumpToString(QMap<quint16, ModbusResult> map)
{
    QString str;
    QDebug dStream(&str);

    dStream << map;

    return str;
}

QString ModbusMaster::dumpToString(QList<quint16> list)
{
    QString str;
    QDebug dStream(&str);

    dStream << list;

    return str;
}

void ModbusMaster::logInfo(QString msg)
{
    emit modbusLogInfo(QString("[Conn %0] %1").arg(_connectionId).arg(msg));
}

void ModbusMaster::logError(QString msg)
{
    emit modbusLogError(QString("[Conn %0] %1").arg(_connectionId).arg(msg));
}
