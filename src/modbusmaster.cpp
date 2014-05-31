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

#ifdef QT_DEBUG_OUTPUT
    qDebug() << "Register read: thread " << _thread->currentThreadId();
#endif
    for (qint32 i = 0; i < pRegisterList->size(); i++)
    {

        // TODO: optimize reads
        if (ReadRegisters(pSettings, pRegisterList->at(i), 1, &resultList) == 0)
        {
            globalResultList.append(resultList[0]);
        }
        else
        {
            globalResultList.append(0);
        }
    }

    emit ReadRegisterResult(true, globalResultList);
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

qint32 ModbusMaster::ReadRegisters(ModbusSettings * pSettings, quint16 startReg, quint32 num, QList<quint16> * pResultList)
{
    qint32 rc = 0;

    /* Open port */
    modbus_t *pCtx = Connect(pSettings->GetIpAddress(), pSettings->GetPort());
    if (pCtx)
    {
        quint16 * aRegister = (quint16 *)malloc(num * sizeof(quint16));

#ifdef QT_DEBUG_OUTPUT
        qDebug() << "Read registers: start (" << startReg << "), number (" << num << ")";
#endif
        if (modbus_read_registers(pCtx, startReg, num, aRegister) == -1)
        {
            qDebug() << "MB: Read failed: " << modbus_strerror(errno) << endl;
            rc = -2;
        }
        else
        {
            pResultList->clear();
            for (u_int32_t i = 0; i < num; i++)
            {
#ifdef QT_DEBUG_OUTPUT
                qDebug() << "Register result (" << i << "): " << aRegister[i];
#endif
                pResultList->append(aRegister[i]);
            }
        }

        free(aRegister);
        Close(pCtx); /* Close port */
    }
    else
    {
        rc = -1;
    }

    return rc;
}
