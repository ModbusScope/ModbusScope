

#include "modbusmaster.h"
#include "modbussettings.h"
#include "scopegui.h"
#include "QTimer"
#include "QDateTime"
#include "scopedata.h"

ScopeData::ScopeData(QObject *parent) :
    QObject(parent), _master(NULL), _active(false), _timer(new QTimer()), _successCount(0), _errorCount(0)
{

    qRegisterMetaType<QList<quint16> *>("QList<quint16> *");
    qRegisterMetaType<QList<quint16> >("QList<quint16>");
    qRegisterMetaType<QList<bool> >("QList<bool>");

    _registerlist.clear();

    /* Setup modbus master */
    _master = new ModbusMaster();

    connect(this, SIGNAL(requestStop()), _master, SLOT(stopThread()));

    connect(_master, SIGNAL(threadStopped()), this, SLOT(masterStopped()));
    connect(_master, SIGNAL(threadStopped()), _master, SLOT(deleteLater()));

    _master->startThread();

    connect(this, SIGNAL(registerRequest(ModbusSettings *, QList<quint16> *)), _master, SLOT(readRegisterList(ModbusSettings *, QList<quint16> *)));
    connect(_master, SIGNAL(modbusPollDone(QList<bool>, QList<quint16>)), this, SLOT(handlePollDone(QList<bool>, QList<quint16>)));
    connect(_master, SIGNAL(modbusCommDone(quint32, quint32)), this, SLOT(processCommStats(quint32, quint32)));
}

ScopeData::~ScopeData()
{
    emit requestStop();

    if (_master)
    {
        _master->wait();
    }

    delete _timer;
}

bool ScopeData::startCommunication(ModbusSettings * pSettings, QList<quint16> registers)
{
    bool bResetted = false;

    if (!_active)
    {
        _settings.copy(pSettings);

        _registerlist.clear();
        _registerlist.append(registers);

        _successCount = 0;
        _errorCount = 0;

        // Trigger read immediatly
        _timer->singleShot(1, this, SLOT(readData()));

        _active = true;
        bResetted = true;
    }

    return bResetted;
}

void ScopeData::processCommStats(quint32 success,quint32 error)
{
    _successCount += success;
    _errorCount += error;

    emit triggerStatUpdate(_successCount, _errorCount);
}

void ScopeData::handlePollDone(QList<bool> successList, QList<quint16> values)
{
    // Restart timer when previous request has been handled
    uint waitInterval;
    const uint passedInterval = QDateTime::currentMSecsSinceEpoch() - _lastPollStart;

    if (passedInterval > _settings.getPollTime())
    {
        // Poll again immediatly
        waitInterval = 1;
    }
    else
    {
        // Set waitInterval to remaining time
        waitInterval = _settings.getPollTime() - passedInterval;
    }

    _timer->singleShot(waitInterval, this, SLOT(readData()));

    // propagate data
    emit handleReceivedData(successList, values);
}


void ScopeData::masterStopped()
{
    _master = NULL;
}

void ScopeData::stopCommunication()
{
    _active = false;
}

void ScopeData::readData()
{
    if(_active)
    {
        _lastPollStart = QDateTime::currentMSecsSinceEpoch();
        emit registerRequest(&_settings, &_registerlist);
    }
}

