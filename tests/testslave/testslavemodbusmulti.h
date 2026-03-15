#ifndef TESTSLAVEMODBUSMULTI_H
#define TESTSLAVEMODBUSMULTI_H

#include "testdevice.h"

#include <QTcpServer>
#include <QTcpSocket>

/*!
 * Minimal Modbus TCP server that serves multiple slave IDs on a single port.
 * QModbusTcpServer only handles one slave ID, making it impossible to bind two
 * instances to the same port. This class implements enough of the Modbus TCP
 * protocol (Read Holding Registers, FC 0x03) to support multi-slave tests.
 */
class TestSlaveMultiModbus : public QTcpServer
{
    Q_OBJECT
public:
    explicit TestSlaveMultiModbus(QObject* parent = nullptr);
    ~TestSlaveMultiModbus();

    bool listenOnPort(quint16 port);
    void stopListening();

    TestDevice* testDevice(quint8 slaveId);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void handleReadyRead();

private:
    void processFrame(QTcpSocket* socket, const QByteArray& frame);

    QMap<quint8, TestDevice*> _devices;
    QList<QTcpSocket*> _sockets;
};

#endif // TESTSLAVEMODBUSMULTI_H
