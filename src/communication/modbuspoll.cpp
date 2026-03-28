
#include "communication/modbuspoll.h"

#include "models/connection.h"
#include "models/connectiontypes.h"
#include "models/device.h"
#include "models/settingsmodel.h"
#include "util/formatdatetime.h"
#include "util/modbusdatatype.h"
#include "util/scopelogging.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QSerialPort>

ModbusPoll::ModbusPoll(SettingsModel* pSettingsModel, QObject* parent) : QObject(parent), _bPollActive(false)
{
    _pPollTimer = new QTimer(this);
    _pPollTimer->setSingleShot(true);
    connect(_pPollTimer, &QTimer::timeout, this, &ModbusPoll::triggerRegisterRead);

    _pSettingsModel = pSettingsModel;
    _lastPollStart = QDateTime::currentMSecsSinceEpoch();

    _pAdapterProcess = new AdapterProcess(this);
    _pAdapterClient = new AdapterClient(_pAdapterProcess, this);

    connect(_pAdapterClient, &AdapterClient::sessionStarted, this, &ModbusPoll::triggerRegisterRead);
    connect(_pAdapterClient, &AdapterClient::readDataResult, this, &ModbusPoll::onReadDataResult);
    connect(_pAdapterClient, &AdapterClient::describeResult, this, &ModbusPoll::onDescribeResult);
    connect(_pAdapterClient, &AdapterClient::sessionError, this, [this](QString message) {
        qCWarning(scopeComm) << "AdapterClient error:" << message;
        _bPollActive = false;
    });
    connect(_pAdapterClient, &AdapterClient::sessionStopped, this, &ModbusPoll::initAdapter);
}

ModbusPoll::~ModbusPoll() = default;

/*! \brief Prepare the protocol adapter subprocess for use.
 *
 * Resolves the adapter binary relative to the running executable so the path
 * is correct in the build tree, AppImage, and installed layouts alike.
 * Calls prepareAdapter() on the client, which triggers the adapter.describe
 * handshake.
 */
void ModbusPoll::initAdapter()
{
    const QString adapterPath = QCoreApplication::applicationDirPath() + "/modbusadapter";
    _pAdapterClient->prepareAdapter(adapterPath);
}

void ModbusPoll::startCommunication(QList<ModbusRegister>& registerList)
{
    _registerList = registerList;
    _bPollActive = true;

    qCInfo(scopeComm) << QString("Start logging: %1").arg(FormatDateTime::currentDateTime());

    resetCommunicationStats();

    QStringList expressions = buildRegisterExpressions(_registerList);

    const AdapterData* data = _pSettingsModel->adapterData("modbus");
    QJsonObject config = data->hasStoredConfig() ? data->currentConfig() : buildAdapterConfig();

    _pAdapterClient->provideConfig(config, expressions);
}

void ModbusPoll::resetCommunicationStats()
{
    _lastPollStart = QDateTime::currentMSecsSinceEpoch();
}

void ModbusPoll::stopCommunication()
{
    _bPollActive = false;
    _pPollTimer->stop();
    _pAdapterClient->stopSession();

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
        _pAdapterClient->requestReadData();
    }
}

void ModbusPoll::onReadDataResult(ResultDoubleList results)
{
    emit registerDataReady(results);

    if (_bPollActive)
    {
        const quint32 passedInterval = static_cast<quint32>(QDateTime::currentMSecsSinceEpoch() - _lastPollStart);
        uint waitInterval;

        if (passedInterval > _pSettingsModel->pollTime())
        {
            waitInterval = 1;
        }
        else
        {
            waitInterval = _pSettingsModel->pollTime() - passedInterval;
        }

        _pPollTimer->start(static_cast<int>(waitInterval));
    }
}

void ModbusPoll::onDescribeResult(const QJsonObject& description)
{
    _pSettingsModel->updateAdapterFromDescribe("modbus", description);
}

QJsonObject ModbusPoll::buildAdapterConfig()
{
    QJsonArray connections;
    const QList<ConnectionTypes::connectionId_t> connectionList = _pSettingsModel->connectionList();
    for (const ConnectionTypes::connectionId_t id : connectionList)
    {
        if (!_pSettingsModel->connectionState(id))
        {
            continue;
        }

        Connection* pConn = _pSettingsModel->connectionSettings(id);
        QJsonObject connObj;
        connObj["id"] = static_cast<int>(id);
        connObj["timeout"] = static_cast<int>(pConn->timeout());
        connObj["persistent"] = pConn->persistentConnection();

        if (pConn->connectionType() == ConnectionTypes::TYPE_TCP)
        {
            connObj["type"] = "tcp";
            connObj["ip"] = pConn->ipAddress();
            connObj["port"] = pConn->port();
        }
        else
        {
            QString parityStr;
            switch (pConn->parity())
            {
            case QSerialPort::EvenParity:
                parityStr = "E";
                break;
            case QSerialPort::OddParity:
                parityStr = "O";
                break;
            default:
                parityStr = "N";
                break;
            }

            connObj["type"] = "serial";
            connObj["portName"] = pConn->portName();
            connObj["baudrate"] = static_cast<int>(pConn->baudrate());
            connObj["parity"] = parityStr;
            connObj["databits"] = static_cast<int>(pConn->databits());
            connObj["stopbits"] = static_cast<int>(pConn->stopbits());
        }

        connections.append(connObj);
    }

    QJsonArray devices;
    const QList<deviceId_t> deviceList = _pSettingsModel->deviceListForAdapter("modbus");
    for (const deviceId_t devId : deviceList)
    {
        Device* pDev = _pSettingsModel->deviceSettings(devId);
        QJsonObject devObj;
        devObj["id"] = static_cast<int>(devId);
        devObj["connectionId"] = static_cast<int>(pDev->connectionId());
        devObj["slaveId"] = static_cast<int>(pDev->slaveId());
        devObj["consecutiveMax"] = static_cast<int>(pDev->consecutiveMax());
        devObj["int32LittleEndian"] = pDev->int32LittleEndian();
        devObj["name"] = pDev->name();
        devices.append(devObj);
    }

    QJsonObject config;
    config["version"] = 1;
    config["connections"] = connections;
    config["devices"] = devices;
    return config;
}

QStringList ModbusPoll::buildRegisterExpressions(const QList<ModbusRegister>& registerList)
{
    QStringList expressions;
    for (const ModbusRegister& reg : registerList)
    {
        QString expr = QString("${%1 @ %2 : %3}")
                         .arg(reg.address().fullAddress())
                         .arg(reg.deviceId())
                         .arg(ModbusDataType::typeString(reg.type()));
        expressions.append(expr);
    }
    return expressions;
}
