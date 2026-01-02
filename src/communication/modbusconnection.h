#ifndef MODBUSCONNECTION_H
#define MODBUSCONNECTION_H

#include "communication/modbusdataunit.h"
#include "models/connectiontypes.h"
#include <QModbusClient>
#include <QModbusDevice>
#include <QModbusReply>
#include <QPointer>
#include <QSerialPort>
#include <QTimer>

class ConnectionData : public QObject
{
    Q_OBJECT
public:
    explicit ConnectionData(std::unique_ptr<QModbusClient> pModbus)
        : connectionTimeoutTimer(this), bConnectionErrorHandled(false), pReply(nullptr)
    {
        pModbusClient = std::move(pModbus);
    }

    QTimer connectionTimeoutTimer;
    std::unique_ptr<QModbusClient> pModbusClient;
    bool bConnectionErrorHandled;

    QModbusReply * pReply;
};


class ModbusConnection : public QObject
{
    Q_OBJECT
public:
    explicit ModbusConnection(QObject *parent = nullptr);

    typedef struct
    {
        QString ip;
        qint32 port;
    } tcpSettings_t;

    typedef struct
    {
        QString portName;
        QSerialPort::Parity parity;
        QSerialPort::BaudRate baudrate;
        QSerialPort::DataBits databits;
        QSerialPort::StopBits stopbits;
    } serialSettings_t;

    virtual void configureTcpConnection(tcpSettings_t const& tcpSettings);
    virtual void configureSerialConnection(serialSettings_t const& serialSettings);

    virtual void open(quint32 timeout);
    virtual void close(void);

    virtual void sendReadRequest(ModbusDataUnit const& regAddress, quint16 size);

    virtual bool isConnected(void);

signals:
    void connectionSuccess(void);
    void connectionError(QModbusDevice::Error error, QString msg);

    void readRequestSuccess(ModbusDataUnit startRegister, QList<quint16> registerDataList);
    void readRequestProtocolError(QModbusPdu::ExceptionCode exceptionCode);
    void readRequestError(QString errorString, QModbusDevice::Error error);

private slots:
    void handleConnectionStateChanged(QModbusDevice::State connectionState);
    void handleConnectionErrorOccurred(QModbusDevice::Error error);

    void connectionTimeOut();
    void connectionDestroyed();

    void handleRequestFinished();

private:
    bool prepareConnectionOpen();
    void openConnection(QPointer<ConnectionData> connectionData, quint32 timeout);
    QModbusDataUnit::RegisterType registerType(ModbusAddress::ObjectType type);
    ModbusAddress::ObjectType objectType(QModbusDataUnit::RegisterType type);
    void handleConnectionError(QPointer<ConnectionData> connectionData, QString errMsg);
    qint32 findConnectionData(QTimer * pTimer, QModbusClient * pClient);

    ConnectionTypes::type_t _connectionType;
    tcpSettings_t _tcpSettings;
    serialSettings_t _serialSettings;

    QList<QPointer<ConnectionData>> _connectionList;
    bool _bWaitingForConnection;

};

#endif // MODBUSCONNECTION_H
