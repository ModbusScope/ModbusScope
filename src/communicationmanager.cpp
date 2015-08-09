#include <QTimer>
#include <QDateTime>

#include "modbusmaster.h"
#include "communicationmanager.h"
#include "guimodel.h"
#include "settingsmodel.h"


CommunicationManager::CommunicationManager(SettingsModel * pSettingsModel, GuiModel *pGuiModel, QObject *parent) :
    QObject(parent), _active(false)
{

    _pPollTimer = new QTimer();
    _pGuiModel = pGuiModel;
    _pSettingsModel = pSettingsModel;

    qRegisterMetaType<QList<quint16> >("QList<quint16>");
    qRegisterMetaType<QList<bool> >("QList<bool>");

    _registerlist.clear();

    /* Setup modbus master */
    _master = new ModbusMaster(_pSettingsModel, _pGuiModel);

    connect(this, SIGNAL(requestStop()), _master, SLOT(stopThread()));

    connect(_master, SIGNAL(threadStopped()), this, SLOT(masterStopped()));
    connect(_master, SIGNAL(threadStopped()), _master, SLOT(deleteLater()));

    _master->startThread();

    connect(this, SIGNAL(registerRequest(QList<quint16>)), _master, SLOT(readRegisterList(QList<quint16>)));
    connect(_master, SIGNAL(modbusPollDone(QList<bool>, QList<quint16>)), this, SLOT(handlePollDone(QList<bool>, QList<quint16>)));
}

CommunicationManager::~CommunicationManager()
{
    emit requestStop();

    if (_master)
    {
        _master->wait();
    }

    delete _pPollTimer;
}

bool CommunicationManager::startCommunication(QList<RegisterData> registers)
{
    bool bResetted = false;

    if (!_active)
    {
        _registerlist.clear();        
        _registerlist.append(registers);

        // Trigger read immediatly
        _pPollTimer->singleShot(1, this, SLOT(readData()));

        _active = true;
        bResetted = true;

        resetCommunicationStats();
    }

    return bResetted;
}

void CommunicationManager::resetCommunicationStats()
{
    if (_active)
    {
        _pGuiModel->setCommunicationStats(0, 0);

        _lastPollStart = QDateTime::currentMSecsSinceEpoch();
        _pGuiModel->setCommunicationStartTime(QDateTime::currentMSecsSinceEpoch());
    }
}

void CommunicationManager::handlePollDone(QList<bool> successList, QList<quint16> values)
{
    // Restart timer when previous request has been handled
    uint waitInterval;
    const uint passedInterval = QDateTime::currentMSecsSinceEpoch() - _lastPollStart;

    if (passedInterval > _pSettingsModel->pollTime())
    {
        // Poll again immediatly
        waitInterval = 1;
    }
    else
    {
        // Set waitInterval to remaining time
        waitInterval = _pSettingsModel->pollTime() - passedInterval;
    }

    _pPollTimer->singleShot(waitInterval, this, SLOT(readData()));

    // Process values
    QList<double> processedValue;
    for (qint32 i = 0; i < values.size(); i++)
    {
        if (_registerlist[i].isUnsigned())
        {
            processedValue.append(values[i]);
        }
        else
        {
            processedValue.append((qint16)values[i]);
        }

        // Apply bitmask
        if (_registerlist[i].isUnsigned())
        {
            processedValue[i] = (quint16)((quint16)processedValue[i] & _registerlist[i].bitmask());
        }
        else
        {
            processedValue[i] = (qint16)((quint16)processedValue[i] & _registerlist[i].bitmask());
        }

        // Apply shift
        if (_registerlist[i].shift() != 0)
        {
            if (_registerlist[i].shift() > 0)
            {
                processedValue[i] = (quint16)((quint16)processedValue[i] << _registerlist[i].shift());
            }
            else
            {
                processedValue[i] = (quint16)((quint16)processedValue[i] >> abs(_registerlist[i].shift()));
            }

            if (!_registerlist[i].isUnsigned())
            {
                processedValue[i] = (qint16)processedValue[i];
            }
        }

        // Apply scaleFactor
        processedValue[i] = processedValue[i] * _registerlist[i].scaleFactor();
    }

    // propagate processed data
    emit handleReceivedData(successList, processedValue);
}


void CommunicationManager::masterStopped()
{
    _master = NULL;
}

void CommunicationManager::stopCommunication()
{
    _active = false;
    _pPollTimer->stop();
    _pGuiModel->setCommunicationEndTime(QDateTime::currentMSecsSinceEpoch());
}

bool CommunicationManager::isActive()
{
    return _active;
}

void CommunicationManager::readData()
{
    if(_active)
    {
        _lastPollStart = QDateTime::currentMSecsSinceEpoch();

        QList<quint16> regAddrList;
        for (qint32 i = 0; i < _registerlist.size(); i++)
        {
            regAddrList.append(_registerlist[i].registerAddress());
        }
        emit registerRequest(regAddrList);
    }
}
