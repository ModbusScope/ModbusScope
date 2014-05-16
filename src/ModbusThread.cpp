

#include "modbussettings.h"
#include "ModbusThread.h"
#include "QDebug"
#include "QThread"
#include "QElapsedTimer"
#include "QCoreApplication"

// Constructor
ModbusThread::ModbusThread() :
    bStop(false),
    bActive(false),
    thread(NULL),
    settings(NULL)
{

    // NEVER create object with new here
}

// Destructor
ModbusThread::~ModbusThread()
{

}

void ModbusThread::SetSettingsPointer(ModbusSettings * settings)
{
    this->settings = settings;
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

            if(conToUiTimer.elapsed() > 200)
            {
                if (modbusMaster.Open(settings->GetIpAddress().toStdString(), settings->GetPort()) != -1)   // Open modbus
                {

                    QList<u_int16_t> l;
                    QList<u_int16_t> registers;
                    QList<u_int16_t> result;

                    settings->GetRegisters(&registers);

                    qDebug() << "Start regs read (" << registers.size() << ")" << "\n";
                    for( int32_t i = 0; i < registers.size(); i++)
                    {

                        if (modbusMaster.ReadRegisters(registers[i]-40001, 1, &l) != -1)
                        {
                            result.append(l[0]);
                        }
                        else
                        {
                            result.append(0);
                        }
                        qDebug() << "reg " << registers[i] << ": " << result[i] << "\n";
                    }

                    emit modbusResults(result);

                    conToUiTimer.restart();

                    modbusMaster.Close(); // Close modbus
                }
            }
        }

    }

    qDebug("Byebye\n");

    thread->quit();
}
