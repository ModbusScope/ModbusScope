
#include <QObject>
#include <QPointer>
#include <QUrl>

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

    QPointer<TestSlaveData> _pTestSlaveData;
    QPointer<TestSlaveModbus> _pTestSlaveModbus;

    quint8 _slaveId;

    QUrl _serverConnectionData;
};
