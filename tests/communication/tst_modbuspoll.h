
#include <QObject>
#include <QUrl>

#include "modbuspoll.h"
#include "settingsmodel.h"
#include "modbusregister.h"

#include "testslavedata.h"
#include "testslavemodbus.h"

class TestModbusPoll: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void singleSlaveSuccess();
    void singleSlaveFail();
    void singleOnlyConstantDataPoll();

    void verifyRestartAfterStop();

    void multiSlaveSuccess();
    void multiSlaveSuccess_2();
    void multiSlaveSuccess_3();
    void multiSlaveSingleFail();
    void multiSlaveAllFail();
    void multiSlaveDisabledConnection();

private:

    void verifyReceivedDataSignal(QList<QVariant> arguments, ResultDoubleList expResultList);

    SettingsModel * _pSettingsModel;

    QList<QUrl> _serverConnectionDataList;
    QList<TestSlaveData *> _testSlaveDataList;
    QList<TestSlaveModbus *> _testSlaveModbusList;
};
