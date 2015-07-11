#include <QApplication>
#include <QThread>
#include <QCoreApplication>
#include <QDebug>
#include "modbusmaster.h"
#include "connectionmodel.h"
#include "guimodel.h"
#include <modbus.h>

#include "errno.h"

ModbusMaster::ModbusMaster(ConnectionModel *pConnectionModel, GuiModel * pGuiModel, QObject *parent) :
    QObject(parent),
    _pThread(NULL)
{
    // NEVER create object with new here

    _pConnectionModel = pConnectionModel;
    _pGuiModel = pGuiModel;
}

ModbusMaster::~ModbusMaster()
{

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
    _pThread->quit();
}

void ModbusMaster::stopped()
{
    /* thread is deleted using a connection between thread->finished and thread->deleteLater */
    _pThread = NULL;

    emit threadStopped();
}

void ModbusMaster::readRegisterList(QList<quint16> registerList)
{
    QList<quint16> resultList;
    QList<bool> resultStateList;
    quint32 success = 0;
    quint32 error = 0;

    /* Open port */
    modbus_t * pCtx = openPort(_pConnectionModel->ipAddress(), _pConnectionModel->port());
    if (pCtx)
    {
        /* Set modbus slave */
        modbus_set_slave(pCtx, _pConnectionModel->slaveId());

        // Disable byte time-out
        uint32_t sec = -1;
        uint32_t usec = 0;
        modbus_set_byte_timeout(pCtx, sec, usec);

        // Set response timeout
        sec = _pConnectionModel->timeout() / 1000;
        usec = (_pConnectionModel->timeout() % 1000) * 1000uL;
        modbus_set_response_timeout(pCtx, sec, usec);

        // Do optimized reads
        qint32 regIndex = 0;
        while (regIndex < registerList.size())
        {
            quint32 count = 0;

            // get number of subsequent registers
            if ((registerList.size() - regIndex) > 1)
            {
                bool bSubsequent;
                do
                {
                    bSubsequent = false;

                    // if next is current + 1, dan subsequent = true
                    if (registerList.at(regIndex + count + 1) == registerList.at(regIndex + count) + 1)
                    {
                        bSubsequent = true;
                        count++;
                    }

                    // Break loop when end of list
                    if ((regIndex + count) >= ((uint)registerList.size() - 1))
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
            if (readRegisters(pCtx, registerList.at(regIndex) - 40001, count, &registerDataList) == 0)
            {
                success++;
                resultList.append(registerDataList);
                for (uint i = 0; i < count; i++)
                {
                    resultStateList.append(true);
                }
            }
            else
            {
                error++;
                for (uint i = 0; i < count; i++)
                {
                    resultList.append(0);
                    resultStateList.append(false);
                }
            }


            // Set register index to next register
            regIndex += count;
        }

        closePort(pCtx); /* Close port */
    }
    else
    {
        for (qint32 i = 0; i < registerList.size(); i++)
        {
            error++;
            resultList.append(0);
            resultStateList.append(false);
        }
    }

    _pGuiModel->setCommunicationStats(_pGuiModel->communicationSuccessCount() + success, _pGuiModel->communicationErrorCount() + error);

    emit modbusPollDone(resultStateList, resultList);
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
