#ifndef MODBUSMASTER_H
#define MODBUSMASTER_H

#include <QMap>
#include <QModbusDevice>
#include <QModbusReply>

#include "communication/modbusconnection.h"
#include "communication/readregisters.h"
#include "models/connectiontypes.h"
#include "util/modbusresultmap.h"

/* Forward declaration */
class SettingsModel;

class ModbusMaster : public QObject
{
    Q_OBJECT
public:
    explicit ModbusMaster(ModbusConnection* pModbusConnection,
                          SettingsModel* pSettingsModel,
                          ConnectionTypes::connectionId_t connectionId);
    virtual ~ModbusMaster();

    void readRegisterList(QList<ModbusDataUnit> registerList, quint8 consecutiveMax);

    void cleanUp();

signals:
    void modbusPollDone(ModbusResultMap modbusResults, ConnectionTypes::connectionId_t connectionId);
    void modbusLogError(QString msg);
    void modbusLogInfo(QString msg);
    void triggerNextRequest();

private slots:
    void handleConnectionOpened();
    void handlerConnectionError(QModbusDevice::Error error, QString msg);

    void handleRequestSuccess(ModbusDataUnit const& startRegister, QList<quint16> registerDataList);
    void handleRequestProtocolError(QModbusPdu::ExceptionCode exceptionCode);
    void handleRequestError(QString errorString, QModbusDevice::Error error);

    void handleTriggerNextRequest(void);

private:
    void finishRead(bool bError);
    QString dumpToString(ModbusResultMap map) const;
    QString dumpToString(QList<ModbusDataUnit> list) const;

    void logResults(const ModbusResultMap &results);

    void logInfo(QString msg);
    void logError(QString msg);

    ConnectionTypes::connectionId_t _connectionId{};

    std::unique_ptr<ModbusConnection> _pModbusConnection;
    SettingsModel* _pSettingsModel{};
    ReadRegisters _readRegisters{};
};

#endif // MODBUSMASTER_H
