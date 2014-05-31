

#include "modbusmaster.h"
#include "modbussettings.h"
#include "QTimer"
#include "QDebug"

#include "modbusscope.h"

ModbusScope::ModbusScope(QObject *parent) :
    QObject(parent), _master(NULL), _active(false), _timer(new QTimer())
{

    qRegisterMetaType<QList<quint16> *>("QList<quint16> *");
    qRegisterMetaType<QList<quint16> >("QList<quint16>");

    /* Setup modbus master */
    _master = new ModbusMaster();

    connect(this, SIGNAL(RequestStop()), _master, SLOT(StopThread()));

    connect(_master, SIGNAL(ThreadStopped()), this, SLOT(MasterStopped()));
    connect(_master, SIGNAL(ThreadStopped()), _master, SLOT(deleteLater()));

    _master->StartThread();

    connect(this, SIGNAL(RegisterRequest(ModbusSettings *, QList<quint16> *)), _master, SLOT(ReadRegisterList(ModbusSettings *, QList<quint16> *)));
    connect(_master, SIGNAL(ReadRegisterResult(bool, QList<quint16>)), this, SLOT(RegisterProcess(bool, QList<quint16>)));
}

ModbusScope::~ModbusScope()
{
    emit RequestStop();

#ifdef QT_DEBUG_OUTPUT
    qDebug() << "ModbusScope::~ModbusScope() before wait";
#endif

    if (_master)
    {
        _master->Wait();
    }

#ifdef QT_DEBUG_OUTPUT
    qDebug() << "ModbusScope::~ModbusScope() after wait";
#endif

    delete _timer;
}

void ModbusScope::StartCommunication(ModbusSettings * settings, QList<quint16> * registerList)
{
    if (!_active)
    {
        _settings.Copy(settings);

        _registerlist.clear();

        for(qint32 i = 0; i < registerList->size(); i++)
        {
            //TODO option
            _registerlist.append(registerList->at(i) - 40001);
        }

        // Start timer
        _timer->singleShot(1000, this, SLOT(ReadData()));

        _active = true;
    }
}

void ModbusScope::MasterStopped()
{
#ifdef QT_DEBUG_OUTPUT
    qDebug() << "ModbusScope::MasterStopped";
#endif
    _master = NULL;
}

void ModbusScope::StopCommunication()
{
#ifdef QT_DEBUG_OUTPUT
    qDebug() << "ModbusScope::StopCommunication";
#endif
    _active = false;
}

void ModbusScope::RegisterProcess(bool success, QList<quint16> resultList)
{
    Q_UNUSED(success);
    Q_UNUSED(resultList);
}

void ModbusScope::ReadData()
{
    if(_active)
    {
        emit RegisterRequest(&_settings, &_registerlist);

        _timer->singleShot(1000, this, SLOT(ReadData()));
    }
}

