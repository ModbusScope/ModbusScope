#ifndef MODBUSMASTER_H
#define MODBUSMASTER_H

#include <QObject>
#include <QMap>
#include <QModbusDevice>
#include <QModbusReply>

#include "result.h"

/* Forward declaration */
class SettingsModel;
class ReadRegisters;
class ModbusConnection;


class ModbusMaster : public QObject
{
    Q_OBJECT
public:
    explicit ModbusMaster(SettingsModel * pSettingsModel, quint8 connectionId);
    virtual ~ModbusMaster();

    void readRegisterList(QList<quint32> registerList);

    void cleanUp();

signals:
    void modbusPollDone(QMap<quint32, Result<quint16> > modbusResults, quint8 connectionId);
    void modbusLogError(QString msg);
    void modbusLogInfo(QString msg);
    void triggerNextRequest();

private slots:
    void handleConnectionOpened();
    void handlerConnectionError(QModbusDevice::Error error, QString msg);

    void handleRequestSuccess(quint32 startRegister, QList<quint16> registerDataList);
    void handleRequestProtocolError(QModbusPdu::ExceptionCode exceptionCode);
    void handleRequestError(QString errorString, QModbusDevice::Error error);

    void handleTriggerNextRequest(void);

private:
    void finishRead(bool bError);
    QString dumpToString(QMap<quint32, Result<quint16> > map);
    QString dumpToString(QList<quint32> list);

    void logResults(QMap<quint32, Result<quint16> > &results);

    void logInfo(QString msg);
    void logError(QString msg);

    quint8 _connectionId;

    SettingsModel * _pSettingsModel;
    ModbusConnection * _pModbusConnection;
    ReadRegisters * _pReadRegisters;

};

#endif // MODBUSMASTER_H
