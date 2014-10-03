

#include "modbusmaster.h"
#include "modbussettings.h"
#include "scopegui.h"
#include "QTimer"
#include "QDebug"

#include "scopedata.h"

ScopeData::ScopeData(QObject *parent) :
    QObject(parent), _master(NULL), _active(false), _timer(new QTimer())
{

    qRegisterMetaType<QList<quint16> *>("QList<quint16> *");
    qRegisterMetaType<QList<quint16> >("QList<quint16>");

    /* Setup modbus master */
    _master = new ModbusMaster();

    connect(this, SIGNAL(requestStop()), _master, SLOT(stopThread()));

    connect(_master, SIGNAL(threadStopped()), this, SLOT(masterStopped()));
    connect(_master, SIGNAL(threadStopped()), _master, SLOT(deleteLater()));

    _master->startThread();

    connect(this, SIGNAL(registerRequest(ModbusSettings *, QList<quint16> *)), _master, SLOT(readRegisterList(ModbusSettings *, QList<quint16> *)));

    connect(_master, SIGNAL(readRegisterResult(bool, QList<quint16>)), this, SLOT(receiveNewData(bool, QList<quint16>)));
    //connect(_master, SIGNAL(readRegisterResult(bool, QList<quint16>)), _gui, SLOT(plotResults(bool, QList<quint16>)));
}

ScopeData::~ScopeData()
{
    emit requestStop();

#ifdef QT_DEBUG_OUTPUT
    qDebug() << "ScopeData::~ScopeData() before wait";
#endif

    if (_master)
    {
        _master->wait();
    }

#ifdef QT_DEBUG_OUTPUT
    qDebug() << "ScopeData::~ScopeData() after wait";
#endif

    delete _timer;
}

bool ScopeData::startCommunication(ModbusSettings * pSettings, QList<quint16> * pRegisterList)
{
    bool bResetted = false;

    if (!_active)
    {
        _settings.copy(pSettings);

        _registerlist.clear();

        for(qint32 i = 0; i < pRegisterList->size(); i++)
        {
            //TODO option
            _registerlist.append(pRegisterList->at(i) - 40001);
        }

        // Start timer
        _timer->singleShot(1000, this, SLOT(readData()));

        _active = true;
        bResetted = true;
    }

    return bResetted;
}

void ScopeData::masterStopped()
{
#ifdef QT_DEBUG_OUTPUT
    qDebug() << "ScopeData::MasterStopped";
#endif
    _master = NULL;
}

void ScopeData::stopCommunication()
{
#ifdef QT_DEBUG_OUTPUT
    qDebug() << "ScopeData::StopCommunication";
#endif
    _active = false;
}

void ScopeData::receiveNewData(bool bSuccess, QList<quint16> values)
{
    emit propagateNewData(bSuccess, values);
}

void ScopeData::readData()
{
    if(_active)
    {
        emit registerRequest(&_settings, &_registerlist);

        _timer->singleShot(1000, this, SLOT(readData()));
    }
}

