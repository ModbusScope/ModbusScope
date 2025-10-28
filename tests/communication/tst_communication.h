
#include "models/device.h"
#include "testdevice.h"

#include <QObject>

/* Forward declaration */
class SettingsModel;
class GraphDataModel;
class ModbusPoll;
class TestSlaveModbus;

class TestCommunication: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void singleSlaveSuccess();

    void typeFloat32();

    void constantExpression();

    void mixed_1();
    void mixed_fail();

    void readLargeRegisterAddress();

    void unknownConnection();
    void disabledConnection();

private:
    void doHandleRegisterData(QList<QVariant>& actRawData);

    SettingsModel * _pSettingsModel;
    GraphDataModel* _pGraphDataModel;
    QMap<deviceId_t, TestSlaveModbus*> _testSlaveMap;
};
