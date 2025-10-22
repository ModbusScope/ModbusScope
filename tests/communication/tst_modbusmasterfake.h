
#include <QObject>

#include "models/settingsmodel.h"
#include "testdevice.h"

class TestModbusMasterFake : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void singleRequestSuccess();
    void requestToDevicesOnSameConnection();

private:
    QMap<deviceId_t, TestDevice*> _testDeviceMap;
    SettingsModel* _pSettingsModel;
};
