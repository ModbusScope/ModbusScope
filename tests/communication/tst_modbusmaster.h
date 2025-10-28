
#include "models/settingsmodel.h"
#include "testdevice.h"

#include <QObject>

class TestSlaveModbus;

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
    QMap<deviceId_t, TestSlaveModbus*> _testSlaveMap;

    SettingsModel* _pSettingsModel;

    const uint _cReadCount = 3;
};
