
#include <QObject>
#include <QPointer>
#include <QUrl>
#include "settingsmodel.h"

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

    SettingsModel _settingsModel;
    QUrl _serverConnectionData;
};
