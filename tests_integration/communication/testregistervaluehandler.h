
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

    void read16();
    void read32();

    void connectionCheck();
    void twoConnectionsCheck();

private:

    QMap<quint16, ModbusResult> createResultMap(
            quint16 addr,
            bool b32bit,
            qint64 value,
            bool result
            );

    void addRegister();

    SettingsModel* _pSettingsModel;
    GraphDataModel* _pGraphDataModel;

};
