#include "modbusmaster.h"
#include "settingsmodel.h"
#include "errorlogmodel.h"
#include <util.h>

typedef QMap<quint16,ModbusResult> ModbusResultMap;
Q_DECLARE_METATYPE(ModbusResultMap);
Q_DECLARE_METATYPE(ModbusResult);

ModbusMaster::ModbusMaster(SettingsModel * pSettingsModel) :
    QObject(nullptr)
{

    qMetaTypeId<ModbusResult>();
    qMetaTypeId<QMap<quint16, ModbusResult> >();

    _pSettingsModel = pSettingsModel;
    _pModbusConnection = new ModbusConnection();
    _pReadRegisters = new ReadRegisters();

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

    emit modbusLogInfo("Register list read: " + dumpToString(registerList));

    _pReadRegisters->resetRead(registerList, _pSettingsModel->consecutiveMax());

    /* Open connection */
    _pModbusConnection->openConnection(_pSettingsModel->ipAddress(), _pSettingsModel->port(), _pSettingsModel->timeout());
}

void ModbusMaster::handleConnectionOpened()
{
    emit modbusLogInfo("Connection opened");

    emit triggerNextRequest();
}

void ModbusMaster::handlerConnectionError(QModbusDevice::Error error, QString msg)
{
    Q_UNUSED(error);

    _error++;

    emit modbusLogError(QString("Connection error (fatal):") + msg);

    _pReadRegisters->addAllErrors();

    finishRead();
}

void ModbusMaster::handleRequestSuccess(quint16 startRegister, QList<quint16> registerDataList)
{
    emit modbusLogInfo(QString("Read success"));

    // Success
    _pReadRegisters->addSuccess(startRegister, registerDataList);

    _success++;

    // Start next read
    emit triggerNextRequest();
}

void ModbusMaster::handleRequestProtocolError(QModbusPdu::ExceptionCode exceptionCode)
{
    emit modbusLogError(QString("Modbus Exception: %0").arg(exceptionCode));

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

        emit modbusLogInfo("Partial list read: " + QString("Start address (%0) and count (%1)").arg(readItem.address()).arg(readItem.count()));

        _pModbusConnection->sendReadRequest(readItem.address(), readItem.count(), _pSettingsModel->slaveId());
    }
    else
    {
        // Done reading
        finishRead();
    }
}

void ModbusMaster::finishRead()
{
    QMap<quint16, ModbusResult> results = _pReadRegisters->resultMap();

    emit modbusLogInfo("Result map: " + dumpToString(results));
    emit modbusAddToStats(_success, _error);
    emit modbusPollDone(results);
    emit modbusLogInfo("Connection closed");

    _pModbusConnection->closeConnection();
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
