
#include <QObject>
#include <QPointer>
#include <QUrl>
#include "settingsmodel.h"

#include "testslavedata.h"
#include "testslavemodbus.h"

class TestModbusMaster: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void singleRequestSuccess();
    void singleRequestGatewayNotAvailable();
    void singleRequestNoResponse();
    void singleRequestInvalidAddressOnce();
    void singleRequestInvalidAddressPersistent();

    void multiRequestSuccess();
    void multiRequestGatewayNotAvailable();
    void multiRequestNoResponse();
    void multiRequestInvalidAddress();

private:

    QPointer<TestSlaveData> _pTestSlaveData;
    QPointer<TestSlaveModbus> _pTestSlaveModbus;

    SettingsModel _settingsModel;
    QUrl _serverConnectionData;

    const uint _cReadCount = 3;
};
