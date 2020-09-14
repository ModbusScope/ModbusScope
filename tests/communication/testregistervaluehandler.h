
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

    void read_16_default();
    void read_16_operation();

    void read_s16_default();
    void read_s16_operation();

    void read_32_default();
    void read_32_operation();

    void read_s32_default();
    void read_s32_operation();

    void read_32BitMixed_1();
    void read_32BitMixed_2();

    void multiRead();

    void connectionCheck();
    void twoConnectionsCheck();

    void graphList_1();
    void graphList_2();
    void graphList_3();
    void graphList_4();

    void bigEndian_32_1();
    void bigEndian_s32_2();

    void manyInactiveRegisters();


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
