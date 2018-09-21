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
        timeoutTimer(this), modbusClient(this)
    {

    }

    QTimer timeoutTimer;
    QModbusTcpClient modbusClient;
};


class ModbusConnection : public QObject
{
    Q_OBJECT
public:
    explicit ModbusConnection(QObject *parent = nullptr);

    void openConnection(QString ip, qint32 port, quint32 timeout);
    void closeConnection(void);

    QModbusReply *sendReadRequest(const QModbusDataUnit &read, int serverAddress);

    QModbusDevice::State state(void);

signals:
    void connectionSuccess(void);
    void errorOccurred(QModbusDevice::Error error, QString msg);

private slots:
    void handleConnectionStateChanged(QModbusDevice::State state);
    void handleConnectionErrorOccurred(QModbusDevice::Error error);

    void connectionTimeOut();
    void connectionDestroyed();

private:

    void handleError(QString errMsg);
    qint32 findConnectionData(QTimer * pTimer, QModbusTcpClient * pClient);

    QList<QPointer<ConnectionData>> _connectionList;

};

#endif // MODBUSCONNECTION_H
