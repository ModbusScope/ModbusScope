
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

    void read16_1();
    void read16_2();
    void read16_3();
    void read16_4();
    void read16_5();
    void read16_6();
    void read16_7();
    void read16_8();

    void read32_1();

    void read32BitMixed_1();
    void read32BitMixed_2();

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
