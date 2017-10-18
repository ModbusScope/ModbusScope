#include <QApplication>
#include <QThread>
#include <QCoreApplication>
#include "modbusmaster.h"
#include "settingsmodel.h"
#include "guimodel.h"
#include "errorlogmodel.h"
#include <modbus.h>
#include <util.h>

#include "errno.h"

ModbusMaster::ModbusMaster(SettingsModel * pSettingsModel, GuiModel * pGuiModel, QObject *parent) :
    QObject(parent),
    _pThread(NULL)
{
    // NEVER create object with new here

    qRegisterMetaType<ModbusResult>("ModbusResult");
    qRegisterMetaType<QMap<quint16, ModbusResult> >("QMap<quint16, ModbusResult>");

    _pSettingsModel = pSettingsModel;
    _pGuiModel = pGuiModel;
}

ModbusMaster::~ModbusMaster()
{
    _pThread = NULL;
    _pSettingsModel = NULL;
    _pGuiModel = NULL;
}

void ModbusMaster::startThread()
{
    if(_pThread == NULL)
    {
        _pThread = new QThread();
        _pThread->start();
        connect(_pThread, SIGNAL(finished()), _pThread, SLOT(deleteLater()));
        connect(_pThread, SIGNAL(finished()), this, SLOT(stopped()));

        moveToThread(_pThread);
    }
}

void ModbusMaster::wait()
{
    if(_pThread)
    {
        _pThread->wait();
    }
}

void ModbusMaster::stopThread()
{
    if(_pThread)
    {
        _pThread->quit();
    }
}

void ModbusMaster::stopped()
{
    /* thread is deleted using a connection between thread->finished and thread->deleteLater */
    _pThread = NULL;

    emit threadStopped();
}

void ModbusMaster::readRegisterList(QList<quint16> registerList)
{

    _resultMap.clear();
    _success = 0;
    _error = 0;

    emit modbusLogInfo("Register list read: " + dumpToString(registerList));

    /* Open port */
    if (openPort())
    {

        while (registerList.size() > 0)
        {
            // Do optimized reads
            QList<quint16> readList = getSubsequentRegisterList(registerList);

            readPartialList(readList);

            // Remove registers from TODO list
            while(readList.size() > 0)
            {
                registerList.removeFirst();
                readList.removeFirst();
            }
        }
    }
    else
    {
        emit modbusLogError(QString("Open port failed"));
        addErrorResults(registerList);
    }

    emit modbusLogInfo("Result map: " + dumpToString(_resultMap));

    if (_bOpen)
    {
        closePort(_pModbusContext);
    }

    _pGuiModel->setCommunicationStats(_pGuiModel->communicationSuccessCount() + _success, _pGuiModel->communicationErrorCount() + _error);

    emit modbusPollDone(_resultMap);

}


void ModbusMaster::readPartialList(QList<quint16> registerList)
{
    emit modbusLogInfo("Partial list read:" + dumpToString(registerList));

    if (!_bOpen)
    {
        if (!openPort())
        {
            emit modbusLogError(QString("Open port failed"));
            addErrorResults(registerList);
            return;
        }
    }

    // Read registers
    QList<quint16> registerDataList;
    qint32 returnCode = readRegisters(_pModbusContext, registerList.first() - 40001, registerList.size(), &registerDataList);
    if (returnCode == 0)
    {
        addSuccessResults(registerList, registerDataList);
    }
    else
    {
        /* only split on specific modbus exception (invalid value and invalid address) */
        if (
                (returnCode == (MODBUS_ENOBASE + MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS))
                || (returnCode == (MODBUS_ENOBASE + MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE))
            )
        {

            emit modbusLogError(QString("Modbus Exception"));

            /* No need to close connection because we got exception */

            /* Read of one register failed */
            if (registerList.size() == 1)
            {
                /* Log error */
                addErrorResults(registerList);
            }
            else
            {
                _error++;

                /* More than one => read all separately */
                for (qint32 i = 0; i < registerList.size(); i++)
                {
                    QList<quint16> singleRegisterList;
                    singleRegisterList.append(registerList.at(i));

                    /* recursive function call */
                    readPartialList(singleRegisterList);
                }
            }
        }
        else
        {
            emit modbusLogError(QString("No response"));

            /* Close connection because we didn't get response */
            closePort(_pModbusContext);

            /* Log errors */
            addErrorResults(registerList);
        }
    }
}

void ModbusMaster::closePort(modbus_t *connection)
{
    modbus_close(connection);
    modbus_free(connection);

    _bOpen = false;

    emit modbusLogInfo("Connection closed");
}

/* Open and configure port */
bool ModbusMaster::openPort()
{       
    _pModbusContext = modbus_new_tcp(_pSettingsModel->ipAddress().toStdString().c_str(), _pSettingsModel->port());
    if (_pModbusContext)
    {
        if (modbus_connect(_pModbusContext) == -1)
        {
            emit modbusLogError("Connection failed: " + QString(modbus_strerror(errno)));

            modbus_free(_pModbusContext);
            _pModbusContext = 0;
        }
    }
    else
    {
        emit modbusLogError("New TCP failed: " + QString(modbus_strerror(errno)));
        _pModbusContext = 0;
    }

    if (_pModbusContext)
    {
        /* Set modbus slave */
        modbus_set_slave(_pModbusContext, _pSettingsModel->slaveId());

        // Disable byte time-out
        uint32_t sec = -1;
        uint32_t usec = 0;
        modbus_set_byte_timeout(_pModbusContext, sec, usec);

        // Set response timeout
        sec = _pSettingsModel->timeout() / 1000;
        usec = (_pSettingsModel->timeout() % 1000) * 1000uL;
        modbus_set_response_timeout(_pModbusContext, sec, usec);

        _bOpen = true;

        emit modbusLogInfo("Connection opened");

        return true;
    }

    emit modbusLogError("Connection open failed");

    return false;
}

qint32 ModbusMaster::readRegisters(modbus_t * pCtx, quint16 startReg, quint32 num, QList<quint16> * pResultList)
{
    qint32 rc = 0;

    quint16 * aRegister = (quint16 *)malloc(num * sizeof(quint16));

    if (modbus_read_registers(pCtx, startReg, num, aRegister) == -1)
    {
        rc = errno;

        emit modbusLogError("MB: Read failed: " + QString(modbus_strerror(errno)));
    }
    else
    {
        pResultList->clear();
        for (quint32 i = 0; i < num; i++)
        {
            pResultList->append(aRegister[i]);
        }
    }

    free(aRegister);

    return rc;
}

QList<quint16> ModbusMaster::getSubsequentRegisterList(QList<quint16> registerList)
{
    QList<quint16> retList;
    quint32 currentIdx = 0;

    if (registerList.size() > 0)
    {
        retList.append(registerList.first());
    }

    if ((registerList.size() > 1)
        && (_pSettingsModel->consecutiveMax() > 1)
        )
    {
        bool bSubsequent;
        do
        {
            bSubsequent = false;

            // if next is current + 1, dan subsequent = true
            if (registerList.at(currentIdx) + 1 == registerList.at(currentIdx + 1))
            {
                bSubsequent = true;
                retList.append(registerList.at(currentIdx + 1));
                currentIdx++;
            }

            // Break loop when end of list
            if (currentIdx >= ((uint)registerList.size() - 1))
            {
                break;
            }

            // Limit number of register in 1 read
            if (retList.size() >= _pSettingsModel->consecutiveMax())
            {
                break;
            }

        } while(bSubsequent == true);
    }

    return retList;
}

void ModbusMaster::addSuccessResults(QList<quint16> registerList, QList<quint16> registerDataList)
{
    _success++;

    for (qint32 i = 0; i < registerList.size(); i++)
    {
        const quint16 registerAddr = registerList.at(i);
        const ModbusResult result = ModbusResult(registerDataList[i], true);

        _resultMap.insert(registerAddr,result);
    }
}

void ModbusMaster::addErrorResults(QList<quint16> registerList)
{
    _error++;

    for (qint32 i = 0; i < registerList.size(); i++)
    {
        const quint16 registerAddr = registerList.at(i);
        const ModbusResult result = ModbusResult(0, false);

        _resultMap.insert(registerAddr,result);
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
