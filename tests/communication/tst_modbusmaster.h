
#include "models/settingsmodel.h"

#include "testslavemodbus.h"

#include <QObject>
#include <QPointer>
#include <QUrl>

class TestModbusMaster: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void singleRequestSuccess();
    void singleRequestEmpty();
    void singleRequestGatewayNotAvailable();
    void singleRequestNoResponse();
    void singleRequestInvalidAddressOnce();
    void singleRequestInvalidAddressPersistent();

    void multiRequestSuccess();
    void multiRequestGatewayNotAvailable();
    void multiRequestNoResponse();
    void multiRequestInvalidAddress();

private:

    TestSlaveModbus::ModbusDataMap _testSlaveData;
    QPointer<TestSlaveModbus> _pTestSlaveModbus;

    SettingsModel _settingsModel;
    QUrl _serverConnectionData;

    const uint _cReadCount = 3;
};
