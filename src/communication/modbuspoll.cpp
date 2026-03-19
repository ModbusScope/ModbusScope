
#include "communication/modbuspoll.h"

#include <QDateTime>

#include "models/settingsmodel.h"
#include "util/formatdatetime.h"
#include "util/scopelogging.h"

ModbusPoll::ModbusPoll(SettingsModel* pSettingsModel, QObject* parent) : QObject(parent), _bPollActive(false)
{
    _pPollTimer = new QTimer();
    _pSettingsModel = pSettingsModel;
    _lastPollStart = QDateTime::currentMSecsSinceEpoch();
}

ModbusPoll::~ModbusPoll()
{
    delete _pPollTimer;
}

void ModbusPoll::startCommunication(QList<ModbusRegister>& registerList)
{
    _registerList = registerList;

    // Trigger read immediately
    _pPollTimer->singleShot(1, this, &ModbusPoll::triggerRegisterRead);

    _bPollActive = true;

    qCInfo(scopeComm) << QString("Start logging: %1").arg(FormatDateTime::currentDateTime());

    resetCommunicationStats();
}

void ModbusPoll::resetCommunicationStats()
{
    _lastPollStart = QDateTime::currentMSecsSinceEpoch();
}

void ModbusPoll::stopCommunication()
{
    _bPollActive = false;
    _pPollTimer->stop();

    qCInfo(scopeComm) << QString("Stop logging: %1").arg(FormatDateTime::currentDateTime());
}

bool ModbusPoll::isActive()
{
    return _bPollActive;
}

void ModbusPoll::triggerRegisterRead()
{
    if (_bPollActive)
    {
        _lastPollStart = QDateTime::currentMSecsSinceEpoch();

        ResultDoubleList results;
        for (qsizetype i = 0; i < _registerList.size(); i++)
        {
            results.append(ResultDouble());
        }
        emit registerDataReady(results);

        // Reschedule timer
        uint waitInterval;
        const quint32 passedInterval = static_cast<quint32>(QDateTime::currentMSecsSinceEpoch() - _lastPollStart);

        if (passedInterval > _pSettingsModel->pollTime())
        {
            // Poll again immediately
            waitInterval = 1;
        }
        else
        {
            // Set waitInterval to remaining time
            waitInterval = _pSettingsModel->pollTime() - passedInterval;
        }

        _pPollTimer->singleShot(static_cast<int>(waitInterval), this, &ModbusPoll::triggerRegisterRead);
    }
}
