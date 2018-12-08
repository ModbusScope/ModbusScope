#ifndef MODBUSCONNECTION_H
#define MODBUSCONNECTION_H

#include <QObject>
#include <QTimer>

#include <QModbusDevice>
#include <QModbusReply>
#include <QModbusTcpClient>
#include <QPointer>

class ConnectionData : public QObject
{
    Q_OBJECT
public:

    explicit ConnectionData():
        connectionTimeoutTimer(this), modbusClient(this), bConnectionErrorHandled(false)
    {

    }

    QTimer connectionTimeoutTimer;
    QModbusTcpClient modbusClient;
    bool bConnectionErrorHandled;
};


class ModbusConnection : public QObject
{
    Q_OBJECT
public:
    explicit ModbusConnection(QObject *parent = nullptr);

    void openConnection(QString ip, qint32 port, quint32 timeout);
    void closeConnection(void);

    QModbusReply *sendReadRequest(const QModbusDataUnit &read, int serverAddress);

    QModbusDevice::State connectionState(void);

signals:
    void connectionSuccess(void);
    void connectionError(QModbusDevice::Error error, QString msg);

private slots:
    void handleConnectionStateChanged(QModbusDevice::State connectionState);
    void handleConnectionErrorOccurred(QModbusDevice::Error error);

    void connectionTimeOut();
    void connectionDestroyed();

private:

    void handleConnectionError(QPointer<ConnectionData> connectionData, QString errMsg);
    qint32 findConnectionData(QTimer * pTimer, QModbusTcpClient * pClient);

    QList<QPointer<ConnectionData>> _connectionList;
    bool _bWaitingForConnection;

};

#endif // MODBUSCONNECTION_H
