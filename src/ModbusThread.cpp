

#include "ModbusThread.h"
#include "QDebug"
#include "QThread"
#include "QElapsedTimer"
#include "QCoreApplication"

// Constructor
ModbusThread::ModbusThread() :
    bStop(false),
    bActive(false),
    thread(NULL)
{
    // NEVER create object with new here
}

// Destructor
ModbusThread::~ModbusThread()
{

}


void ModbusThread::startCommunication(/*QList<unsigned short> registers*/)
{
    /*
    registerList.clear();
    registerList.append(registers);
    */
    bActive = true;
}

void ModbusThread::stopCommunication()
{
    bActive = false;
}


void ModbusThread::start()
{
    if(thread == NULL)
    {
        thread = new QThread();
        thread->start();
        bStop = false;
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        connect(thread, SIGNAL(finished()), this, SLOT(threadStopped()));

        connect(thread, SIGNAL(started()), this, SLOT(threadStarted()));

        moveToThread(thread);
    }
}

void ModbusThread::wait()
{
    if(thread)
    {
        thread->wait();
    }
}

void ModbusThread::stopThread()
{
    bStop = true;
}

void ModbusThread::threadStarted()
{
    process();
}

void ModbusThread::threadStopped()
{
    /* thread is deleted using a connection between thread->finished and thread->deleteLater */
    thread = NULL;

    emit done();
}

// Process
void ModbusThread::process()
{
    // Code here starts as soon as the thread starts
    QElapsedTimer conToUiTimer;
    conToUiTimer.start();

    qDebug("Start\n");

    while(!bStop)
    {

        QCoreApplication::processEvents();

        if (bActive)
        {

            if(conToUiTimer.elapsed() > 2000)
            {
                if (modbusMaster.Open("127.0.0.1", 1502) != -1)   // Open modbus
                {

                    QList<unsigned short> l;
                    if (modbusMaster.ReadRegisters(0, 2, &l) != -1)
                    {
                        emit modbusResults(l[0], l[1]);
                        /*
                        qDebug() << "reg 0: " << l[0] << "\n";
                        qDebug() << "reg 1: " << (unsigned short)l[1] << "\n";
                        */

                        // Read modbus vars
                        conToUiTimer.restart();
                    }

                    modbusMaster.Close(); // Close modbus
                }
            }
        }

    }

    qDebug("Byebye\n");

    thread->quit();
}
