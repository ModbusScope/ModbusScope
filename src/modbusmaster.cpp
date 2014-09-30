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

void ModbusMaster::StartThread()
{
    if(_thread == NULL)
    {
        _thread = new QThread();
        _thread->start();
        connect(_thread, SIGNAL(finished()), _thread, SLOT(deleteLater()));
        connect(_thread, SIGNAL(finished()), this, SLOT(Stopped()));

#ifdef QT_DEBUG_OUTPUT
        qDebug() << "Thread start: " << _thread->currentThreadId();
#endif

        moveToThread(_thread);
    }
}

void ModbusMaster::Wait()
{
    if(_thread)
    {
        _thread->wait();
    }
}

void ModbusMaster::StopThread()
{
#ifdef QT_DEBUG_OUTPUT
    qDebug() << "Thread stop: " << _thread->currentThreadId();
#endif
    _thread->quit();
}

void ModbusMaster::Stopped()
{

#ifdef QT_DEBUG_OUTPUT
    qDebug() << "ModbusMaster::Stopped";
#endif
    /* thread is deleted using a connection between thread->finished and thread->deleteLater */
    _thread = NULL;

    emit ThreadStopped();
}

void ModbusMaster::ReadRegisterList(ModbusSettings * pSettings, QList<quint16> * pRegisterList)
{
    QList<quint16> globalResultList;
    QList<quint16> resultList;
    bool bSuccess = false;

#ifdef QT_DEBUG_OUTPUT
    qDebug() << "Register read: thread " << _thread->currentThreadId();
    qDebug() << "Register read: number " << pRegisterList->size();
#endif

    /* Open port */
    modbus_t * pCtx = Connect(pSettings->GetIpAddress(), pSettings->GetPort());
    if (pCtx)
    {

        /* Set modbus slave */
        modbus_set_slave(pCtx, pSettings->GetSlaveId());

        // TODO: optimize reads
        for (qint32 i = 0; i < pRegisterList->size(); i++)
        {

            /* handle failure correctly */
            if (ReadRegisters(pCtx, pRegisterList->at(i), 1, &resultList) == 0)
            {
                globalResultList.append(resultList[0]);
                bSuccess = true;
            }
            else
            {
                globalResultList.append(0);
                bSuccess = false;
            }
        }

        Close(pCtx); /* Close port */
    }
    else
    {
        bSuccess = false;
    }

    emit ReadRegisterResult(bSuccess, globalResultList);
}

void ModbusMaster::Close(modbus_t *connection)
{
#ifdef QT_DEBUG_OUTPUT
        qDebug() << "Close connection ";
#endif
    modbus_close(connection);
    modbus_free(connection);
}


modbus_t * ModbusMaster::Connect(QString ip, quint16 port)
{
    modbus_t * conn;

#ifdef QT_DEBUG_OUTPUT
        qDebug() << "Open connection (" << ip.toStdString().c_str() << ":" << port << ")";
#endif
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

qint32 ModbusMaster::ReadRegisters(modbus_t * pCtx, quint16 startReg, quint32 num, QList<quint16> * pResultList)
{
    qint32 rc = 0;

    quint16 * aRegister = (quint16 *)malloc(num * sizeof(quint16));

#ifdef QT_DEBUG_OUTPUT
    qDebug() << "Read registers: start (" << startReg << "), number (" << num << ")";
#endif
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
#ifdef QT_DEBUG_OUTPUT
            qDebug() << "Register result (" << i << "): " << aRegister[i];
#endif
            pResultList->append(aRegister[i]);
        }
    }

    free(aRegister);

    return rc;
}
