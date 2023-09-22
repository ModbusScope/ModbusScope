#ifndef MODBUSCONNECTION_H
#define MODBUSCONNECTION_H

#include "modbusaddress.h"
#include <QObject>
#include <QTimer>
#include <QSerialPort>
#include <QModbusDevice>
#include <QModbusReply>
#include <QModbusClient>
#include <QPointer>

class ConnectionData : public QObject
{
    Q_OBJECT
public:

    explicit ConnectionData(QModbusClient* pModbus):
        connectionTimeoutTimer(this), bConnectionErrorHandled(false), pReply(nullptr)
    {
        pModbusClient = pModbus;
    }

    ~ConnectionData()
    {
        delete pModbusClient;
    }

    QTimer connectionTimeoutTimer;
    QModbusClient* pModbusClient;
    bool bConnectionErrorHandled;

    QModbusReply * pReply;
};


class ModbusConnection : public QObject
{
    Q_OBJECT
public:
    explicit ModbusConnection(QObject *parent = nullptr);

    struct TcpSettings
    {
        QString ip;
        qint32 port;
    };

    struct SerialSettings
    {
        QString portName;
        QSerialPort::Parity parity;
        QSerialPort::BaudRate baudrate;
        QSerialPort::DataBits databits;
        QSerialPort::StopBits stopbits;
    };

    void openTcpConnection(struct TcpSettings tcpSettings, quint32 timeout);
    void openSerialConnection(struct SerialSettings serialSettings, quint32 timeout);
    void closeConnection(void);

    void sendReadRequest(ModbusAddress regAddress, quint16 size, int serverAddress);

    bool isConnected(void);

signals:
    void connectionSuccess(void);
    void connectionError(QModbusDevice::Error error, QString msg);

    void readRequestSuccess(ModbusAddress startRegister, QList<quint16> registerDataList);
    void readRequestProtocolError(QModbusPdu::ExceptionCode exceptionCode);
    void readRequestError(QString errorString, QModbusDevice::Error error);

private slots:
    void handleConnectionStateChanged(QModbusDevice::State connectionState);
    void handleConnectionErrorOccurred(QModbusDevice::Error error);

    void connectionTimeOut();
    void connectionDestroyed();

    void handleRequestFinished();

    bool prepareConnectionOpen();
    void openConnection(QPointer<ConnectionData> connectionData, quint32 timeout);

private:

    QModbusDataUnit::RegisterType registerType(ModbusAddress::ObjectType type);
    ModbusAddress::ObjectType objectType(QModbusDataUnit::RegisterType type);
    void handleConnectionError(QPointer<ConnectionData> connectionData, QString errMsg);
    qint32 findConnectionData(QTimer * pTimer, QModbusClient * pClient);

    QList<QPointer<ConnectionData>> _connectionList;
    bool _bWaitingForConnection;

};

#endif // MODBUSCONNECTION_H
