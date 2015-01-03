

#include "modbusmaster.h"
#include "modbussettings.h"
#include "scopegui.h"
#include "QTimer"
#include "QDateTime"
#include "scopedata.h"

ScopeData::ScopeData(QObject *parent) :
    QObject(parent), _master(NULL), _active(false), _timer(new QTimer()), _successCount(0), _errorCount(0)
{

    qRegisterMetaType<QList<quint16> >("QList<quint16>");
    qRegisterMetaType<QList<bool> >("QList<bool>");
    qRegisterMetaType<ModbusSettings>("ModbusSettings");

    _registerlist.clear();

    /* Setup modbus master */
    _master = new ModbusMaster();

    connect(this, SIGNAL(requestStop()), _master, SLOT(stopThread()));

    connect(_master, SIGNAL(threadStopped()), this, SLOT(masterStopped()));
    connect(_master, SIGNAL(threadStopped()), _master, SLOT(deleteLater()));

    _master->startThread();

    connect(this, SIGNAL(registerRequest(ModbusSettings, QList<quint16>)), _master, SLOT(readRegisterList(ModbusSettings, QList<quint16>)));
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

bool ScopeData::startCommunication(ModbusSettings * pSettings, QList<RegisterData> registers)
{
    bool bResetted = false;

    if (!_active)
    {
        _settings.copy(pSettings);

        _registerlist.clear();        
        _registerlist.append(registers);

        _successCount = 0;
        _errorCount = 0;

        _startTime = QDateTime::currentMSecsSinceEpoch();

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

    // Process values
    QList<qint32> processedValue;
    for (qint32 i = 0; i < values.size(); i++)
    {
        if (_registerlist[i].bUnsigned)
        {
            processedValue.append(values[i]);
        }
        else
        {
            processedValue.append((qint16)values[i]);
        }
    }

    // propagate processed data
    emit handleReceivedData(successList, processedValue);
}


void ScopeData::masterStopped()
{
    _master = NULL;
}

void ScopeData::stopCommunication()
{
    _active = false;
    _endTime = QDateTime::currentMSecsSinceEpoch();
}

qint64 ScopeData::getCommunicationStartTime()
{
    return _startTime;
}

qint64 ScopeData::getCommunicationEndTime()
{
    return _endTime;
}

void ScopeData::getCommunicationSettings(quint32 * successCount, quint32 * errorCount)
{
    *successCount = _successCount;
    *errorCount = _errorCount;
}

void ScopeData::getSettings(ModbusSettings * pSettings)
{
    pSettings->copy(&_settings);
}

bool ScopeData::isActive()
{
    return _active;
}

void ScopeData::readData()
{
    if(_active)
    {
        _lastPollStart = QDateTime::currentMSecsSinceEpoch();

        QList<quint16> regAddrList;
        for (qint32 i = 0; i < _registerlist.size(); i++)
        {
            regAddrList.append(_registerlist[i].reg);
        }
        emit registerRequest(_settings, regAddrList);
    }
}


bool ScopeData::sortRegisterDataList(const RegisterData& s1, const RegisterData& s2)
{
    return s1.reg < s2.reg;
}
