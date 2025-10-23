
#include "communication/modbusconnection.h"
#include "models/device.h"
#include "testslavemodbus.h"

#include <QObject>

class TestSlaveModbus;

class TestModbusConnection: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void connectionSuccess();
    void connectionFail();
    void connectionSuccesAfterFail();

    void readRequestSuccess();
    void readRequestProtocolError();
    void readRequestError();

private:
    ModbusConnection::tcpSettings_t constructTcpSettings(QString ip, qint32 port);

    TestSlaveModbus _testSlaveModbus;

    quint8 _slaveId;
    QString _ip;
    qint32 _port;
};
