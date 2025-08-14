
#include "communication/modbuspoll.h"

#include <QDateTime>

#include "communication/modbusmaster.h"
#include "communication/registervaluehandler.h"
#include "models/settingsmodel.h"
#include "util/formatdatetime.h"
#include "util/scopelogging.h"

ModbusPoll::ModbusPoll(SettingsModel * pSettingsModel, QObject *parent) :
    QObject(parent), _bPollActive(false)
{

    _pPollTimer = new QTimer();
    _pSettingsModel = pSettingsModel;

    _pRegisterValueHandler = new RegisterValueHandler(_pSettingsModel);
    connect(_pRegisterValueHandler, &RegisterValueHandler::registerDataReady, this, &ModbusPoll::registerDataReady);

    /* Setup modbus master */
    for (quint8 i = 0u; i < ConnectionId::ID_CNT; i++)
    {
        auto modbusData = new ModbusMasterData(new ModbusMaster(_pSettingsModel, i));
        _modbusMasters.append(modbusData);

        connect(_modbusMasters.last()->pModbusMaster, &ModbusMaster::modbusPollDone, this, &ModbusPoll::handlePollDone);
        connect(_modbusMasters.last()->pModbusMaster, &ModbusMaster::modbusLogError, this, &ModbusPoll::handleModbusError);
        connect(_modbusMasters.last()->pModbusMaster, &ModbusMaster::modbusLogInfo, this, &ModbusPoll::handleModbusInfo);
    }

    _activeMastersCount = 0;
    _lastPollStart = QDateTime::currentMSecsSinceEpoch();
}

ModbusPoll::~ModbusPoll()
{
    for (quint8 i = 0u; i < _modbusMasters.size(); i++)
    {
        _modbusMasters[i]->pModbusMaster->disconnect();

        delete _modbusMasters[i]->pModbusMaster;
        delete _modbusMasters[i];
    }

    delete _pPollTimer;
}

void ModbusPoll::startCommunication(QList<ModbusRegister>& registerList)
{
    _pRegisterValueHandler->setRegisters(registerList);

    // Trigger read immediately
    _pPollTimer->singleShot(1, this, &ModbusPoll::triggerRegisterRead);

    _bPollActive = true;

    qCInfo(scopeComm) << QString("Start logging: %1").arg(FormatDateTime::currentDateTime());

    for (quint8 i = 0u; i < ConnectionId::ID_CNT; i++)
    {
        auto connData = _pSettingsModel->connectionSettings(i);
        if (_pSettingsModel->connectionState(i))
        {
            QString str;
            if (connData->connectionType() == Connection::TYPE_TCP)
            {
                str = QString("[Conn %0] %1:%2").arg(i + 1).arg(connData->ipAddress()).arg(connData->port());
            }
            else
            {
                QString strParity;
                QString strDataBits;
                QString strStopBits;
                connData->serialConnectionStrings(strParity, strDataBits, strStopBits);

                str = QString("[Conn %0] %1, %2, %3, %4, %5")
                        .arg(i + 1)
                        .arg(connData->portName())
                        .arg(connData->baudrate())
                        .arg(strParity, strDataBits, strStopBits);
            }
            qCInfo(scopeCommConnection) << str;
        }
    }

    resetCommunicationStats();
}

void ModbusPoll::resetCommunicationStats()
{
    _lastPollStart = QDateTime::currentMSecsSinceEpoch();
}

void ModbusPoll::handlePollDone(ModbusResultMap partialResultMap, connectionId_t connectionId)
{
    bool lastResult = false;

    quint8 activeCnt = 0;
    for (quint8 i = 0; i < ConnectionId::ID_CNT; i++)
    {
        if (_modbusMasters[i]->bActive)
        {
            activeCnt++;
        }
    }

    /* Last active modbus master has returned its result */
    if (activeCnt == 1 || activeCnt == 0)
    {
        /* Last result */
        lastResult = true;
    }

    // Always add data to result map
    _pRegisterValueHandler->processPartialResult(partialResultMap, connectionId);

    // Set master as inactive
    if (connectionId < ConnectionId::ID_CNT)
    {
        _modbusMasters[connectionId]->bActive = false;
    }

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

        _pPollTimer->singleShot(static_cast<int>(waitInterval), this, &ModbusPoll::triggerRegisterRead);
    }
}

void ModbusPoll::handleModbusError(QString msg)
{
    qCWarning(scopeCommConnection) << msg;
}

void ModbusPoll::handleModbusInfo(QString msg)
{
    qCDebug(scopeCommConnection) << msg;
}

void ModbusPoll::stopCommunication()
{
    _bPollActive = false;
    _pPollTimer->stop();

    qCInfo(scopeComm) << QString("Stop logging: %1").arg(FormatDateTime::currentDateTime());

    for (quint8 i = 0; i < ConnectionId::ID_CNT; i++)
    {
        _modbusMasters[i]->pModbusMaster->cleanUp();
    }
}

bool ModbusPoll::isActive()
{
    return _bPollActive;
}

void ModbusPoll::triggerRegisterRead()
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

        QList<QList<ModbusAddress> > regAddrList;

        for (connectionId_t i = 0u; i < ConnectionId::ID_CNT; i++)
        {
            QList<deviceId_t> devList = _pSettingsModel->deviceList(i);

            regAddrList.append(QList<ModbusAddress>());

                        _pRegisterValueHandler->registerAddresListForConnection(regAddrList.last(), i);

                        if (regAddrList.last().count() > 0)
                        {
                            _activeMastersCount++;
                        }
        }

        // TODO: use lowest consecutiveMax from relevant devices to avoid buffer issues in device
        quint8 consecutiveMax = 128;

        for (connectionId_t i = 0u; i < ConnectionId::ID_CNT; i++)
        {
            if (regAddrList.at(i).count() > 0)
            {
                _modbusMasters[i]->bActive = true;
                _modbusMasters[i]->pModbusMaster->readRegisterList(regAddrList.at(i), consecutiveMax);
            }
        }

        if (_activeMastersCount == 0)
        {
            ModbusResultMap emptyResultMap;
            handlePollDone(emptyResultMap, ConnectionId::ID_1);
        }
    }
}

