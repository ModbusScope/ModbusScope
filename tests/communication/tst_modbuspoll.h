
#include <QObject>
#include <QUrl>

#include "settingsmodel.h"

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

    void singleSlaveCoil();

    void verifyRestartAfterStop();

    void multiSlaveSuccess();
    void multiSlaveSuccess_2();
    void multiSlaveSuccess_3();
    void multiSlaveSingleFail();
    void multiSlaveAllFail();
    void multiSlaveDisabledConnection();

private:

    TestSlaveData* dataMap(uint32_t connId, QModbusDataUnit::RegisterType type);

    SettingsModel * _pSettingsModel;

    QList<QUrl> _serverConnectionDataList;
    TestSlaveModbus::ModbusDataMap _testSlaveData;

    QList<TestSlaveModbus::ModbusDataMap *> _testSlaveDataList;
    QList<TestSlaveModbus *> _testSlaveModbusList;
};
