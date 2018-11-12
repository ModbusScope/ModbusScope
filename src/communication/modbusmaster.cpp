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

    connect(_pModbusConnection, &ModbusConnection::connectionSuccess, this, &ModbusMaster::handleConnectionOpened);
    connect(_pModbusConnection, &ModbusConnection::errorOccurred, this, &ModbusMaster::handlerConnectionError);
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

void ModbusMaster::handleRequestFinished()
{
    qDebug() << "handleRequestFinished";

    QModbusReply * pReply = qobject_cast<QModbusReply *>(QObject::sender());
    auto err = pReply->error();

    // Start deletion of reply object before handling data (and closing connection)
    pReply->deleteLater();

    if (err == QModbusDevice::NoError)
    {
        emit modbusLogInfo(QString("Read success"));

        // Success
        QModbusDataUnit dataUnit = pReply->result();
        _pReadRegisters->addSuccess(dataUnit.startAddress() + 40001, dataUnit.values().toList());
    }
    else if (err == QModbusDevice::ProtocolError)
    {
        auto exceptionCode = pReply->rawResult().exceptionCode();

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
    }
    else
    {
        emit modbusLogError(QString("Request Failed:  %0 (%1)").arg(pReply->errorString()).arg(pReply->error()));

        // When we don't receive an exception, abort read and close connection
        _pReadRegisters->addAllErrors();
    }

    if (err == QModbusDevice::NoError)
    {
        _success++;
    }
    else
    {
        _error++;
    }

    // Start next read
    emit triggerNextRequest();
}

void ModbusMaster::handleRequestErrorOccurred(QModbusDevice::Error error)
{
    Q_UNUSED(error);
    qDebug() << "Request error occurred (" << QString(error) << "). Should handle?";
}

void ModbusMaster::handleTriggerNextRequest(void)
{
    if (_pReadRegisters->hasNext())
    {
        ModbusReadItem readItem = _pReadRegisters->next();

        emit modbusLogInfo("Partial list read: " + QString("Start address (%0) and count (%1)").arg(readItem.address()).arg(readItem.count()));

        // read registers
        QModbusDataUnit _dataUnit(QModbusDataUnit::HoldingRegisters, readItem.address() - 40001, readItem.count());
        QModbusReply* pReply = _pModbusConnection->sendReadRequest(_dataUnit, _pSettingsModel->slaveId());

        connect(pReply, SIGNAL(finished()), this, SLOT(handleRequestFinished()));
        connect(pReply, SIGNAL(errorOccurred(QModbusDevice::Error)), this, SLOT(handleRequestErrorOccurred(QModbusDevice::Error)));
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
