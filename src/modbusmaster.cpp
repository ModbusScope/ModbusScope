#include <QApplication>
#include <QThread>
#include <QCoreApplication>
#include <QDebug>
#include "modbusmaster.h"
#include <modbus.h>
#include "modbussettings.h"

#include "errno.h"

ModbusMaster::ModbusMaster(QObject *parent) :
    QObject(parent),
    _thread(NULL)
{
    // NEVER create object with new here
}

ModbusMaster::~ModbusMaster()
{

}

void ModbusMaster::startThread()
{
    if(_thread == NULL)
    {
        _thread = new QThread();
        _thread->start();
        connect(_thread, SIGNAL(finished()), _thread, SLOT(deleteLater()));
        connect(_thread, SIGNAL(finished()), this, SLOT(stopped()));

        moveToThread(_thread);
    }
}

void ModbusMaster::wait()
{
    if(_thread)
    {
        _thread->wait();
    }
}

void ModbusMaster::stopThread()
{
    _thread->quit();
}

void ModbusMaster::stopped()
{
    /* thread is deleted using a connection between thread->finished and thread->deleteLater */
    _thread = NULL;

    emit threadStopped();
}

void ModbusMaster::readRegisterList(ModbusSettings * pSettings, QList<quint16> * pRegisterList)
{
    QList<quint16> registerList;
    QList<bool> resultList;
    quint32 success = 0;
    quint32 error = 0;

    /* Open port */
    modbus_t * pCtx = openPort(pSettings->getIpAddress(), pSettings->getPort());
    if (pCtx)
    {

        /* Set modbus slave */
        modbus_set_slave(pCtx, pSettings->getSlaveId());

        // Do optimized reads
        qint32 regIndex = 0;
        while (regIndex < pRegisterList->size())
        {
            quint32 count = 0;

            // get number of subsequent registers
            if ((pRegisterList->size() - regIndex) > 1)
            {
                bool bSubsequent;
                do
                {
                    bSubsequent = false;

                    // if next is current + 1, dan subsequent = true
                    if (pRegisterList->at(regIndex + count + 1) == pRegisterList->at(regIndex + count) + 1)
                    {
                        bSubsequent = true;
                        count++;
                    }

                    // Break loop when end of list
                    if ((regIndex + count) >= ((uint)pRegisterList->size() - 1))
                    {
                        break;
                    }

                    // Limit number of register in 1 read
                    if (count > (_cMaxRegistersInOneRead - 1 - 1))
                    {
                        break;
                    }

                } while(bSubsequent == true);
            }

            // At least one register
            count++;

            // Read registers
            QList<quint16> registerDataList;
            if (readRegisters(pCtx, pRegisterList->at(regIndex) - 40001, count, &registerDataList) == 0)
            {
                success++;
                registerList.append(registerDataList);
                for (uint i = 0; i < count; i++)
                {
                    resultList.append(true);
                }
            }
            else
            {
                error++;
                for (uint i = 0; i < count; i++)
                {
                    registerList.append(0);
                    resultList.append(false);
                }
            }


            // Set register index to next register
            regIndex += count;
        }

        closePort(pCtx); /* Close port */
    }
    else
    {
        for (qint32 i = 0; i < pRegisterList->size(); i++)
        {
            error++;
            registerList.append(0);
            resultList.append(false);
        }
    }

    emit modbusPollDone(resultList, registerList);
    emit modbusCommDone(success, error);
}

void ModbusMaster::closePort(modbus_t *connection)
{
    modbus_close(connection);
    modbus_free(connection);
}


modbus_t * ModbusMaster::openPort(QString ip, quint16 port)
{
    modbus_t * conn;

    conn = modbus_new_tcp(ip.toStdString().c_str(), port);
    if (conn)
    {
        if (modbus_connect(conn) == -1)
        {
#ifdef QT_DEBUG_OUTPUT
            qDebug() << "Connection failed: " << modbus_strerror(errno) << endl;
#endif
            modbus_free(conn);
            conn = 0;
        }
    }
    else
    {
#ifdef QT_DEBUG_OUTPUT
        qDebug() << "New TCP failed: " << modbus_strerror(errno) << endl;
#endif
        conn = 0;
    }

    return conn;
}

qint32 ModbusMaster::readRegisters(modbus_t * pCtx, quint16 startReg, quint32 num, QList<quint16> * pResultList)
{
    qint32 rc = 0;

    quint16 * aRegister = (quint16 *)malloc(num * sizeof(quint16));

    if (modbus_read_registers(pCtx, startReg, num, aRegister) == -1)
    {
        qDebug() << "MB: Read failed: " << modbus_strerror(errno) << endl;
        rc = -1;
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
