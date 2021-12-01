
#include <QDateTime>

#include "modbusmaster.h"
#include "guimodel.h"
#include "settingsmodel.h"
#include "graphdatamodel.h"
#include "diagnosticmodel.h"
#include "scopelogging.h"
#include "formatdatetime.h"
#include "registervaluehandler.h"

#include "communicationmanager.h"

CommunicationManager::CommunicationManager(SettingsModel * pSettingsModel, QObject *parent) :
    QObject(parent), _bPollActive(false)
{

    _pPollTimer = new QTimer();
    _pSettingsModel = pSettingsModel;

    _pRegisterValueHandler = new RegisterValueHandler(_pSettingsModel);
    connect(_pRegisterValueHandler, &RegisterValueHandler::registerDataReady, this, &CommunicationManager::registerDataReady);

    /* Setup modbus master */
    for (quint8 i = 0u; i < SettingsModel::CONNECTION_ID_CNT; i++)
    {
        auto modbusData = new ModbusMasterData(new ModbusMaster(_pSettingsModel, i));
        _modbusMasters.append(modbusData);

        connect(_modbusMasters.last()->pModbusMaster, &ModbusMaster::modbusPollDone, this, &CommunicationManager::handlePollDone);
        connect(_modbusMasters.last()->pModbusMaster, &ModbusMaster::modbusLogError, this, &CommunicationManager::handleModbusError);
        connect(_modbusMasters.last()->pModbusMaster, &ModbusMaster::modbusLogInfo, this, &CommunicationManager::handleModbusInfo);
    }

    _activeMastersCount = 0;
    _lastPollStart = QDateTime::currentMSecsSinceEpoch();
}

CommunicationManager::~CommunicationManager()
{
    for (quint8 i = 0u; i < _modbusMasters.size(); i++)
    {
        _modbusMasters[i]->pModbusMaster->disconnect();

        delete _modbusMasters[i]->pModbusMaster;
        delete _modbusMasters[i];
    }

    delete _pPollTimer;
}

void CommunicationManager::startCommunication(QList<ModbusRegister>& registerList)
{
    _pRegisterValueHandler->setRegisters(registerList);

    // Trigger read immediately
    _pPollTimer->singleShot(1, this, &CommunicationManager::triggerRegisterRead);

    _bPollActive = true;

    qCInfo(scopeCommConnection) << QString("Start logging: %1").arg(FormatDateTime::currentDateTime());

    for (quint8 i = 0u; i < SettingsModel::CONNECTION_ID_CNT; i++)
    {
        if (_pSettingsModel->connectionState(i))
        {
            QString str;
            if (_pSettingsModel->connectionType(i) == SettingsModel::CONNECTION_TYPE_TCP)
            {
                str = QString("[Conn %0] %1:%2 - slave id %3")
                                .arg(i + 1)
                                .arg(_pSettingsModel->ipAddress(i))
                                .arg(_pSettingsModel->port(i))
                                .arg(_pSettingsModel->slaveId(i))
                                ;
            }
            else
            {
                QString strParity;
                QString strDataBits;
                QString strStopBits;
                _pSettingsModel->serialConnectionStrings(i, strParity, strDataBits, strStopBits);

                str = QString("[Conn %0] %1, %2, %3, %4, %5 - slave id %6")
                                .arg(i + 1)
                                .arg(_pSettingsModel->portName(i))
                                .arg(_pSettingsModel->baudrate(i))
                                .arg(strParity, strDataBits, strStopBits)
                                .arg(_pSettingsModel->slaveId(i))
                                ;
            }

            qCInfo(scopeCommConnection) << str;
        }
    }

    resetCommunicationStats();
}

void CommunicationManager::resetCommunicationStats()
{
    _lastPollStart = QDateTime::currentMSecsSinceEpoch();
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
    _pRegisterValueHandler->processPartialResult(partialResultMap, connectionId);

    // Set master as inactive
    _modbusMasters[connectionId]->bActive = false;

    if (lastResult)
    {
        _pRegisterValueHandler->finishRead();

        // Restart timer when previous request has been handled
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

        _pPollTimer->singleShot(static_cast<int>(waitInterval), this, &CommunicationManager::triggerRegisterRead);
    }
}

void CommunicationManager::handleModbusError(QString msg)
{
    qCWarning(scopeCommConnection) << msg;
}

void CommunicationManager::handleModbusInfo(QString msg)
{
    qCDebug(scopeCommConnection) << msg;
}

void CommunicationManager::stopCommunication()
{
    _bPollActive = false;
    _pPollTimer->stop();

    qCInfo(scopeCommConnection) << QString("Stop logging: %1").arg(FormatDateTime::currentDateTime());

    for(quint8 i = 0; i < SettingsModel::CONNECTION_ID_CNT; i++)
    {
        _modbusMasters[i]->pModbusMaster->cleanUp();
    }
}

bool CommunicationManager::isActive()
{
    return _bPollActive;
}

void CommunicationManager::triggerRegisterRead()
{
    if(_bPollActive)
    {
        _lastPollStart = QDateTime::currentMSecsSinceEpoch();

        _pRegisterValueHandler->startRead();

        /* Strange construction is required to avoid race condition:
         *
         * First set _activeMastersCount to correct value
         * And only then activate masters (readRegisterList)
         *
         * readRegisterList can return immediately and this will give race condition otherwise
         */

        _activeMastersCount = 0;

        QList<QList<quint16> > regAddrList;

        for (quint8 i = 0u; i < SettingsModel::CONNECTION_ID_CNT; i++)
        {
            regAddrList.append(QList<quint16>());

            _pRegisterValueHandler->registerAddresList(regAddrList.last(), i);

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

