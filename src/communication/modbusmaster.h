#ifndef MODBUSMASTER_H
#define MODBUSMASTER_H

#include <QObject>
#include <QMap>
#include <QModbusDevice>
#include <QModbusReply>

#include "modbusresultmap.h"
#include "modbusconnection.h"
#include "readregisters.h"

/* Forward declaration */
class SettingsModel;

class ModbusMaster : public QObject
{
    Q_OBJECT
public:
    explicit ModbusMaster(SettingsModel * pSettingsModel, quint8 connectionId);
    virtual ~ModbusMaster();

    void readRegisterList(QList<ModbusAddress> registerList);

    void cleanUp();

signals:
    void modbusPollDone(ModbusResultMap modbusResults, quint8 connectionId);
    void modbusLogError(QString msg);
    void modbusLogInfo(QString msg);
    void triggerNextRequest();

private slots:
    void handleConnectionOpened();
    void handlerConnectionError(QModbusDevice::Error error, QString msg);

    void handleRequestSuccess(ModbusAddress startRegister, QList<quint16> registerDataList);
    void handleRequestProtocolError(QModbusPdu::ExceptionCode exceptionCode);
    void handleRequestError(QString errorString, QModbusDevice::Error error);

    void handleTriggerNextRequest(void);

private:
    void finishRead(bool bError);
    QString dumpToString(ModbusResultMap map) const;
    QString dumpToString(QList<ModbusAddress> list) const;

    void logResults(ModbusResultMap &results);

    void logInfo(QString msg);
    void logError(QString msg);

    quint8 _connectionId{};

    SettingsModel * _pSettingsModel{};
    ModbusConnection _modbusConnection{};
    ReadRegisters _readRegisters{};
};

#endif // MODBUSMASTER_H
