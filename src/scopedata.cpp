

#include "modbusmaster.h"
#include "modbussettings.h"
#include "scopegui.h"
#include "QTimer"
#include "qcustomplot.h"
#include "QDebug"

#include "scopedata.h"

ScopeData::ScopeData(QCustomPlot * pGraph, QObject *parent) :
    QObject(parent), _master(NULL), _gui(NULL), _active(false), _timer(new QTimer())
{

    qRegisterMetaType<QList<quint16> *>("QList<quint16> *");
    qRegisterMetaType<QList<quint16> >("QList<quint16>");

    /* Setup modbus master */
    _master = new ModbusMaster();

    /* Setup GUI graph */
    _gui = new ScopeGui(pGraph, this);

    connect(this, SIGNAL(RequestStop()), _master, SLOT(StopThread()));

    connect(_master, SIGNAL(ThreadStopped()), this, SLOT(MasterStopped()));
    connect(_master, SIGNAL(ThreadStopped()), _master, SLOT(deleteLater()));

    _master->StartThread();

    connect(this, SIGNAL(RegisterRequest(ModbusSettings *, QList<quint16> *)), _master, SLOT(ReadRegisterList(ModbusSettings *, QList<quint16> *)));
    connect(_master, SIGNAL(ReadRegisterResult(bool, QList<quint16>)), _gui, SLOT(PlotResults(bool, QList<quint16>)));
}

ScopeData::~ScopeData()
{
    emit RequestStop();

#ifdef QT_DEBUG_OUTPUT
    qDebug() << "ScopeData::~ScopeData() before wait";
#endif

    if (_master)
    {
        _master->Wait();
    }

#ifdef QT_DEBUG_OUTPUT
    qDebug() << "ScopeData::~ScopeData() after wait";
#endif

    delete _timer;
}

void ScopeData::StartCommunication(ModbusSettings * pSettings, QList<quint16> * pRegisterList)
{
    if (!_active)
    {
        _settings.Copy(pSettings);

        _registerlist.clear();

        for(qint32 i = 0; i < pRegisterList->size(); i++)
        {
            //TODO option
            _registerlist.append(pRegisterList->at(i) - 40001);
        }

        _gui->ResetGraph(pRegisterList->size());

        // Start timer
        _timer->singleShot(1000, this, SLOT(ReadData()));

        _active = true;
    }
}

void ScopeData::MasterStopped()
{
#ifdef QT_DEBUG_OUTPUT
    qDebug() << "ScopeData::MasterStopped";
#endif
    _master = NULL;
}

void ScopeData::StopCommunication()
{
#ifdef QT_DEBUG_OUTPUT
    qDebug() << "ScopeData::StopCommunication";
#endif
    _active = false;
}

void ScopeData::ReadData()
{
    if(_active)
    {
        emit RegisterRequest(&_settings, &_registerlist);

        _timer->singleShot(1000, this, SLOT(ReadData()));
    }
}

