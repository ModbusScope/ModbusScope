
#include <QDateTime>

#include "modbusconnection.h"
#include "readregisters.h"

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

    /* Setup modbus master */
    for (quint8 i = 0u; i < SettingsModel::CONNECTION_ID_CNT; i++) // TODO: Set max modbus master count
    {
        auto modbusData = new ModbusMasterData(new ModbusMaster(_pSettingsModel, i));
        _modbusMasters.append(modbusData);

        connect(_modbusMasters.last()->pModbusMaster, &ModbusMaster::modbusPollDone, this, &CommunicationManager::handlePollDone);

        connect(_modbusMasters.last()->pModbusMaster, SIGNAL(modbusLogError(QString)), this, SLOT(handleModbusError(QString)));
        connect(_modbusMasters.last()->pModbusMaster, SIGNAL(modbusLogInfo(QString)), this, SLOT(handleModbusInfo(QString)));

        connect(_modbusMasters.last()->pModbusMaster, QOverload<quint32, quint32>::of(&ModbusMaster::modbusAddToStats),
            [=](quint32 successes, quint32 errors){
                _pGuiModel->setCommunicationStats(_pGuiModel->communicationSuccessCount() + successes, _pGuiModel->communicationErrorCount() + errors);
            });
    }
}

CommunicationManager::~CommunicationManager()
{
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

void CommunicationManager::handlePollDone(QMap<quint16, ModbusResult> partialResultMap, quint8 connectionId)
{
    bool firstResult = false;
    bool lastResult = false;

    quint8 activeCnt = 0;
    for(quint8 i = 0; i < SettingsModel::CONNECTION_ID_CNT; i++)
    {
        if (_modbusMasters[i]->bActive)
        {
            activeCnt++;
        }
    }

    if (activeCnt == SettingsModel::CONNECTION_ID_CNT)
    {
        /* First results */
        firstResult = true;
    }
    else if (activeCnt == 1)
    {
        /* Last result */
        lastResult = true;
    }
    else
    {
        qDebug() << "Oops: unexpected results";
    }


    if (lastResult)
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

    }

    if (firstResult)
    {
        // Clear result list
        _resultMap.clear();
    }

    // Always add data to result map
    QMapIterator<quint16, ModbusResult> i(partialResultMap);
    while (i.hasNext())
    {
        i.next();
        _resultMap.insert(i.key(), i.value());
    }

    if (lastResult)
    {
        /* Check all register have values and propagate result if correct */

        bool bOk = true;
        QList<quint16> activeIndexList;
        _pGraphDataModel->activeGraphIndexList(&activeIndexList);

        // Process values
        QList<double> processedValues;
        QList<bool> successList;

        // Process values
        if (activeIndexList.count() == _resultMap.count())
        {
            foreach(quint16 graphIndex, activeIndexList)
            {
                const quint16 registerAddress = _pGraphDataModel->registerAddress(graphIndex);
                if (_resultMap.contains(registerAddress))
                {
                    processedValues.append(processValue(graphIndex, _resultMap[registerAddress].value()));
                    successList.append(_resultMap[registerAddress].isSuccess());
                }
                else
                {
                    bOk = false;
                    break;
                }
            }
        }
        else
        {
            bOk = false;
        }

        if (bOk)
        {
            // propagate processed data
            emit handleReceivedData(successList, processedValues);
        }
    }

    // Set master as inactive
    _modbusMasters[connectionId]->bActive = false;
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

        _pGraphDataModel->activeGraphAddresList(&regAddrList, 0);
        _modbusMasters[0]->pModbusMaster->readRegisterList(regAddrList);
        _modbusMasters[0]->bActive = true;

        _pGraphDataModel->activeGraphAddresList(&regAddrList, 1);
        _modbusMasters[1]->pModbusMaster->readRegisterList(regAddrList);
        _modbusMasters[1]->bActive = true;
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
