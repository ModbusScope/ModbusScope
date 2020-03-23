
#include <QDateTime>

#include "modbusmaster.h"
#include "guimodel.h"
#include "settingsmodel.h"
#include "graphdatamodel.h"
#include "errorlogmodel.h"

#include "communicationmanager.h"

CommunicationManager::CommunicationManager(SettingsModel * pSettingsModel, GuiModel *pGuiModel, GraphDataModel *pGraphDataModel, ErrorLogModel *pErrorLogModel, QObject *parent) :
    QObject(parent), _active(false)
{

    _pPollTimer = new QTimer();
    _pGuiModel = pGuiModel;
    _pSettingsModel = pSettingsModel;
    _pGraphDataModel = pGraphDataModel;
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
    QMapIterator<quint16, ModbusResult> i(partialResultMap);
    while (i.hasNext())
    {
        i.next();

        for(quint16 listIdx = 0; listIdx < _activeIndexList.size(); listIdx++)
        {
            const quint16 activeIndex = _activeIndexList[listIdx];
            if (_pGraphDataModel->connectionId(activeIndex) == connectionId)
            {
                if (_pGraphDataModel->registerAddress(activeIndex) == i.key())
                {
                    bool bSuccess = i.value().isSuccess();
                    uint16_t value = static_cast<uint16_t>(i.value().value());
                    double processedResult = 0;

                    if (_pGraphDataModel->isBit32(activeIndex))
                    {

                        ModbusResult nextResult = i.peekNext().value();

                        /* TODO: use connection endiannes settings */
                        uint32_t combinedValue = static_cast<uint32_t>(nextResult.value()) | value;

                        if (nextResult.isSuccess())
                        {
                            processedResult = processValue(activeIndex, combinedValue);
                        }
                        else
                        {
                            bSuccess = false;
                            processedResult = 0;
                        }
                    }
                    else
                    {
                        processedResult = processValue(activeIndex, value);
                    }

                    _processedValues[listIdx] = processedResult;
                    _successList[listIdx] = bSuccess;
                }
            }
        }
    }

    if (lastResult)
    {
        // propagate processed data
        emit handleReceivedData(_successList, _processedValues);
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

        /* Prepare result lists */
        _processedValues.clear();
        _successList.clear();
        _pGraphDataModel->activeGraphIndexList(&_activeIndexList);

        for(int idx = 0; idx < _activeIndexList.size(); idx++)
        {
            _processedValues.append(0);
            _successList.append(false);
        }

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

            _pGraphDataModel->activeGraphAddresList(&regAddrList.last(), i);

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

double CommunicationManager::processValue(quint32 graphIndex, quint16 value)
{

    /* TODO: Rework and unit test fully !!! */
    /* 16 bit vs 32 bit */

    double processedValue = 0;

    if (_pGraphDataModel->isUnsigned(graphIndex))
    {
        processedValue = value;
    }
    else
    {
        processedValue = static_cast<qint16>(value);
    }

    // Apply bitmask
    if (_pGraphDataModel->isUnsigned(graphIndex))
    {
        processedValue = static_cast<quint16>(processedValue) & _pGraphDataModel->bitmask(graphIndex);
    }
    else
    {
        processedValue = static_cast<qint16>(static_cast<qint16>(processedValue) & _pGraphDataModel->bitmask(graphIndex));
    }

    // Apply shift
    if (_pGraphDataModel->shift(graphIndex) != 0)
    {
        if (_pGraphDataModel->shift(graphIndex) > 0)
        {
            processedValue = static_cast<qint16>(processedValue) << _pGraphDataModel->shift(graphIndex);
        }
        else
        {
            processedValue = static_cast<qint16>(processedValue) >> abs(_pGraphDataModel->shift(graphIndex));
        }

        if (!_pGraphDataModel->isUnsigned(graphIndex))
        {
            processedValue = static_cast<qint16>(processedValue);
        }
    }

    // Apply multiplyFactor
    processedValue *= _pGraphDataModel->multiplyFactor(graphIndex);

    // Apply divideFactor
    processedValue /= _pGraphDataModel->divideFactor(graphIndex);

    return processedValue;
}
