
#include <QObject>
#include "modbusregister.h"
#include "modbusresultmap.h"

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
    void addressList_float32();
    void addressList_float32_multiple();
    void addressListCombinations();
    void addressListMultipleConnections();
    void addressListMixedObjects();
    void addressListSameRegisterDifferentType();

    void read_16();
    void read_32();

    void readBigEndian_32();
    void readBigEndian_s32();

    void readSameRegisterDifferentType();
    void readConnections();
    void readFail();

private:

    void verifyRegisterResult(QList<ModbusRegister>& regList,
                            ModbusResultMap &regData,
                            ResultDoubleList expResults);

    void addToResultMap(ModbusResultMap &resultMap,
            quint32 addr,
            bool b32bit,
            qint64 value,
            ResultState::State resultState
            );

    SettingsModel* _pSettingsModel;

};
