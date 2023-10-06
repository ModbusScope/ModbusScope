
#include <QObject>
#include <QPointer>
#include <QUrl>

#include "modbusconnection.h"

#include "testslavedata.h"
#include "testslavemodbus.h"

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

    ModbusConnection::TcpSettings constructTcpSettings(QString ip, qint32 port);

    TestSlaveModbus::ModbusDataMap _testSlaveData;
    QPointer<TestSlaveModbus> _pTestSlaveModbus;

    quint8 _slaveId;

    QUrl _serverConnectionData;
};
