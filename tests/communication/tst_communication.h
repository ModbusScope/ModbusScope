
#include <QObject>
#include <QUrl>

#include "testslavemodbus.h"

/* Forward declaration */
class SettingsModel;
class GraphDataModel;
class ModbusPoll;

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
#if 0
    void readLargeRegisterAddress();
    void readVeryLargeRegisterAddress();
#endif
    void unknownConnection();
    void disabledConnection();

private:

    void doHandleRegisterData(QList<QVariant>& actRawData);
    TestSlaveData* dataMap(uint32_t connId, QModbusDataUnit::RegisterType type);

    SettingsModel * _pSettingsModel;
    GraphDataModel* _pGraphDataModel;

    QList<QUrl> _serverConnectionDataList;
    QList<TestSlaveModbus::ModbusDataMap *> _testSlaveDataList;
    QList<TestSlaveModbus *> _testSlaveModbusList;
};
