
#include <QObject>
#include <QUrl>

#include "testslavedata.h"
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

    void readLargeRegisterAddress();
    void readVeryLargeRegisterAddress();

    void unknownConnection();
    void disabledConnection();

private:

    void doHandleRegisterData(QList<QVariant>& actRawData);

    SettingsModel * _pSettingsModel;
    GraphDataModel* _pGraphDataModel;

    QList<QUrl> _serverConnectionDataList;
    QList<TestSlaveData *> _testSlaveDataList;
    QList<TestSlaveModbus *> _testSlaveModbusList;
};
