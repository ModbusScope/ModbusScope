#include <QTimer>
#include <QDateTime>

#include "modbusmaster.h"
#include "communicationmanager.h"
#include "guimodel.h"
#include "settingsmodel.h"
#include "graphdatamodel.h"
#include "errorlogmodel.h"


CommunicationManager::CommunicationManager(SettingsModel * pSettingsModel, GuiModel *pGuiModel, GraphDataModel *pGraphDataModel, ErrorLogModel *pErrorLogModel, QObject *parent) :
    QObject(parent), _active(false)
{

    _pPollTimer = new QTimer();
    _pGuiModel = pGuiModel;
    _pSettingsModel = pSettingsModel;
    _pGraphDataModel = pGraphDataModel;
    _pErrorLogModel = pErrorLogModel;

    qRegisterMetaType<QList<quint16> >("QList<quint16>");

    /* Setup modbus master */
    _master = new ModbusMaster(_pSettingsModel, _pGuiModel);

    connect(this, SIGNAL(requestStop()), _master, SLOT(stopThread()));

    connect(_master, SIGNAL(threadStopped()), this, SLOT(masterStopped()));
    connect(_master, SIGNAL(threadStopped()), _master, SLOT(deleteLater()));

    _master->startThread();

    connect(this, SIGNAL(registerRequest(QList<quint16>)), _master, SLOT(readRegisterList(QList<quint16>)));
    connect(_master, SIGNAL(modbusPollDone(QMap<quint16, ModbusResult>)), this, SLOT(handlePollDone(QMap<quint16, ModbusResult>)));

    connect(_master, SIGNAL(modbusLogError(QString)), this, SLOT(handleModbusError(QString)));
    connect(_master, SIGNAL(modbusLogInfo(QString)), this, SLOT(handleModbusInfo(QString)));
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

bool CommunicationManager::startCommunication()
{
    bool bResetted = false;

    if (!_active)
    {
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

void CommunicationManager::handlePollDone(QMap<quint16, ModbusResult> resultMap)
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


    /* Check response with current active registers, because it is possible that the graphs have changed during request */


    // Get list of active graph indexes
    bool bOk = true;
    QList<quint16> activeIndexList;
    _pGraphDataModel->activeGraphIndexList(&activeIndexList);

    // Process values
    QList<double> processedValues;
    QList<bool> successList;

    foreach(quint16 graphIndex, activeIndexList)
    {
        const quint16 registerAddress = _pGraphDataModel->registerAddress(graphIndex);
        if (resultMap.contains(registerAddress))
        {
            processedValues.append(processValue(graphIndex, resultMap[registerAddress].value()));
            successList.append(resultMap[registerAddress].isSuccess());
        }
        else
        {
            bOk = false;
            break;
        }
    }

    if (bOk)
    {
        // propagate processed data
        emit handleReceivedData(successList, processedValues);
    }
}

void CommunicationManager::handleModbusError(QString msg)
{
    ErrorLog log = ErrorLog(ErrorLog::LOG_ERROR, QDateTime::currentDateTime(), msg);
    _pErrorLogModel->addItem(log);
}

void CommunicationManager::handleModbusInfo(QString msg)
{
    ErrorLog log = ErrorLog(ErrorLog::LOG_INFO, QDateTime::currentDateTime(), msg);
    _pErrorLogModel->addItem(log);
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
        _pGraphDataModel->activeGraphAddresList(&regAddrList);

        emit registerRequest(regAddrList);
    }
}

double CommunicationManager::processValue(quint32 graphIndex, quint16 value)
{
    double processedValue = 0;

    if (_pGraphDataModel->isUnsigned(graphIndex))
    {
        processedValue = value;
    }
    else
    {
        processedValue = (qint16)value;
    }

    // Apply bitmask
    if (_pGraphDataModel->isUnsigned(graphIndex))
    {
        processedValue = (quint16)((quint16)processedValue & _pGraphDataModel->bitmask(graphIndex));
    }
    else
    {
        processedValue = (qint16)((quint16)processedValue & _pGraphDataModel->bitmask(graphIndex));
    }

    // Apply shift
    if (_pGraphDataModel->shift(graphIndex) != 0)
    {
        if (_pGraphDataModel->shift(graphIndex) > 0)
        {
            processedValue = (quint16)((quint16)processedValue << _pGraphDataModel->shift(graphIndex));
        }
        else
        {
            processedValue = (quint16)((quint16)processedValue >> abs(_pGraphDataModel->shift(graphIndex)));
        }

        if (!_pGraphDataModel->isUnsigned(graphIndex))
        {
            processedValue = (qint16)processedValue;
        }
    }

    // Apply multiplyFactor
    processedValue *= _pGraphDataModel->multiplyFactor(graphIndex);

    // Apply divideFactor
    processedValue /= _pGraphDataModel->divideFactor(graphIndex);

    return processedValue;
}
