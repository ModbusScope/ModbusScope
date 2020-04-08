
#include <QDateTime>

#include "modbusmaster.h"
#include "guimodel.h"
#include "settingsmodel.h"
#include "graphdatamodel.h"
#include "errorlogmodel.h"

#include "communicationmanager.h"

CommunicationManager::CommunicationManager(SettingsModel * pSettingsModel, GuiModel *pGuiModel, GraphDataModel *pGraphDataModel, ErrorLogModel *pErrorLogModel, QObject *parent) :
    QObject(parent), _bPollActive(false), _registerValueHandler(pGraphDataModel, pSettingsModel)
{

    _pPollTimer = new QTimer();
    _pGuiModel = pGuiModel;
    _pSettingsModel = pSettingsModel;
    _pErrorLogModel = pErrorLogModel;

    /* Setup modbus master */
    for (quint8 i = 0u; i < SettingsModel::CONNECTION_ID_CNT; i++)
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

    _activeMastersCount = 0;
    _lastPollStart = QDateTime::currentMSecsSinceEpoch();
}

CommunicationManager::~CommunicationManager()
{
    delete _pPollTimer;
}

bool CommunicationManager::startCommunication()
{
    bool bResetted = false;

    if (!_bPollActive)
    {
        // Trigger read immediatly
        _pPollTimer->singleShot(1, this, SLOT(readData()));

        _bPollActive = true;
        bResetted = true;

        resetCommunicationStats();
    }

    return bResetted;
}

void CommunicationManager::resetCommunicationStats()
{
    if (_bPollActive)
    {
        _pGuiModel->setCommunicationStats(0, 0);

        _lastPollStart = QDateTime::currentMSecsSinceEpoch();
        _pGuiModel->setCommunicationStartTime(QDateTime::currentMSecsSinceEpoch());
    }
}

void CommunicationManager::handlePollDone(QMap<quint16, ModbusResult> partialResultMap, quint8 connectionId)
{
    bool lastResult = false;

    quint8 activeCnt = 0;
    for(quint8 i = 0; i < SettingsModel::CONNECTION_ID_CNT; i++)
    {
        if (_modbusMasters[i]->bActive)
        {
            activeCnt++;
        }
    }

    /* Last active modbus master has returned its result */
    if (activeCnt == 1)
    {
        /* Last result */
        lastResult = true;
    }

    // Always add data to result map
    _registerValueHandler.processPartialResult(partialResultMap, connectionId);

    if (lastResult)
    {
        // propagate processed data
        emit handleReceivedData(_registerValueHandler.successList(), _registerValueHandler.processedValues());
    }

    // Set master as inactive
    _modbusMasters[connectionId]->bActive = false;

    if (lastResult)
    {
        // Restart timer when previous request has been handled
        uint waitInterval;
        const quint32 passedInterval = static_cast<quint32>(QDateTime::currentMSecsSinceEpoch() - _lastPollStart);

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

        _pPollTimer->singleShot(static_cast<int>(waitInterval), this, SLOT(readData()));
    }
}

void CommunicationManager::handleModbusError(QString msg)
{
    qDebug() << msg;
    ErrorLog log = ErrorLog(ErrorLog::LOG_ERROR, QDateTime::currentDateTime(), msg);
    _pErrorLogModel->addItem(log);
}

void CommunicationManager::handleModbusInfo(QString msg)
{
    qDebug() << msg;
    ErrorLog log = ErrorLog(ErrorLog::LOG_INFO, QDateTime::currentDateTime(), msg);
    _pErrorLogModel->addItem(log);
}

void CommunicationManager::stopCommunication()
{
    _bPollActive = false;
    _pPollTimer->stop();
    _pGuiModel->setCommunicationEndTime(QDateTime::currentMSecsSinceEpoch());

    for(quint8 i = 0; i < SettingsModel::CONNECTION_ID_CNT; i++)
    {
        _modbusMasters[i]->pModbusMaster->cleanUp();
    }
}

bool CommunicationManager::isActive()
{
    return _bPollActive;
}

void CommunicationManager::readData()
{
    if(_bPollActive)
    {
        _lastPollStart = QDateTime::currentMSecsSinceEpoch();

        _registerValueHandler.startRead();

        /* Strange construction is required to avoid race condition:
         *
         * First set _activeMastersCount to correct value
         * And only then activate masters (readRegisterList)
         *
         * readRegisterList can return immediatly and this will give race condition otherwise
         */

        _activeMastersCount = 0;

        QList<QList<quint16> > regAddrList;

        for (quint8 i = 0u; i < SettingsModel::CONNECTION_ID_CNT; i++)
        {
            regAddrList.append(QList<quint16>());

            _registerValueHandler.activeGraphAddresList(&regAddrList.last(), i);

            if (regAddrList.last().count() > 0)
            {
                _activeMastersCount++;
            }
        }

        for (quint8 i = 0u; i < SettingsModel::CONNECTION_ID_CNT; i++)
        {
            if (regAddrList.at(i).count() > 0)
            {
                _modbusMasters[i]->bActive = true;
                _modbusMasters[i]->pModbusMaster->readRegisterList(regAddrList.at(i));
            }
        }
    }
}


