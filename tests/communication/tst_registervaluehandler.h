
#include <QObject>
#include "modbusregister.h"
#include "result.h"

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

    void read_16();
    void read_32();

    void readBigEndian_32();
    void readBigEndian_s32();

    void readConnections();
    void readFail();

private:

    void verifyRegisterResult(QList<ModbusRegister>& regList,
                            QMap<quint32, Result<quint16> > &regData,
                            QList<Result<qint64> > expResults);

    void addToResultMap(QMap<quint32, Result<quint16>> &resultMap,
            quint32 addr,
            bool b32bit,
            qint64 value,
            bool result
            );

    SettingsModel* _pSettingsModel;

};
