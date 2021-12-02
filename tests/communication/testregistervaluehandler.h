
#include <QObject>
#include "registervaluehandler.h"

/* Forward declaration */
class SettingsModel;

class TestRegisterValueHandler: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void addressList_16();
    void addressList_16_multiple();
    void addressList_32();
    void addressList_32_multiple();
    void addressListCombinations();
    void addressListMultipleConnections();

    void read_16();
    void read_32();

    void readBigEndian_32();
    void readBigEndian_s32();

    void readConnections();
    void readFail();

private:

    void verifyRegisterResult(QList<ModbusRegister>& regList,
                            QMap<quint16, ModbusResult> &regData,
                            QList<ModbusResult> expResults);

    void addToResultMap(QMap<quint16, ModbusResult> &resultMap,
            quint16 addr,
            bool b32bit,
            qint64 value,
            bool result
            );

    SettingsModel* _pSettingsModel;

};
