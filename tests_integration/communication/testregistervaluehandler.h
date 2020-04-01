
#include <QObject>
#include "registervaluehandler.h"

#include "settingsmodel.h"
#include "graphdatamodel.h"

class TestRegisterValueHandler: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    /*
     * _1: default (read)
     * _2: multiply
     * _3: divide
     * _4: multiply and divide
     * _5: Shift
     * _6: Signed shift
     * _7: Bitmask
     * _8: Full option (without bitmask, to test combinations)
     * */

    void read_16_1();
    void read_16_2();
    void read_16_3();
    void read_16_4();
    void read_16_5();
    void read_16_6();
    void read_16_7();
    void read_16_8();

    void read_s16_1();
    void read_s16_2();
    void read_s16_3();
    void read_s16_4();
    void read_s16_5();
    void read_s16_6();
    void read_s16_7();
    void read_s16_8();

    void read_32_1();
    void read_32_2();
    void read_32_3();
    void read_32_4();
    void read_32_5();
    void read_32_6();
    void read_32_7();
    void read_32_8();

    void read_s32_1();
    void read_s32_2();
    void read_s32_3();
    void read_s32_4();
    void read_s32_5();
    void read_s32_6();
    void read_s32_7();
    void read_s32_8();

    void read_32BitMixed_1();
    void read_32BitMixed_2();

    void multiRead();

    void connectionCheck();
    void twoConnectionsCheck();

private:

    QMap<quint16, ModbusResult> createResultMap(
            quint16 addr,
            bool b32bit,
            qint64 value,
            bool result
            );

    void addToResultMap(QMap<quint16, ModbusResult> &resultMap,
            quint16 addr,
            bool b32bit,
            qint64 value,
            bool result
            );

    void addRegisterToModel();

    SettingsModel* _pSettingsModel;
    GraphDataModel* _pGraphDataModel;

};
