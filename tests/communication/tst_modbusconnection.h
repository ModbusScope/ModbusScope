
#include "communication/modbusconnection.h"

#include "testslavemodbus.h"

#include <QObject>
#include <QPointer>
#include <QUrl>

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

    TestSlaveModbus::ModbusDataMap _testSlaveData;
    QPointer<TestSlaveModbus> _pTestSlaveModbus;

    quint8 _slaveId;

    QUrl _serverConnectionData;
};
