
#include "models/settingsmodel.h"

#include "testslavemodbus.h"

#include <QObject>

class TestModbusPoll : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void singleSlaveSuccess();
    void singleSlaveFail();
    void singleOnlyConstantDataPoll();

    void singleSlaveCoil();
    void singleSlaveMixedObjects();

    void verifyRestartAfterStop();

    void multiSlaveSuccess();
    void multiSlaveSuccess_2();
    void multiSlaveSuccess_3();
    void multiSlaveSingleFail();
    void multiSlaveAllFail();

    void multiSlaveDisabledConnection();
    void notExistingdevice();

private:
    SettingsModel* _pSettingsModel;

    QMap<deviceId_t, TestSlaveModbus*> _testSlaveMap;
};
