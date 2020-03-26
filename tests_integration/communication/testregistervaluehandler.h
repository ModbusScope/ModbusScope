
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

    void connectionCheck();
    void twoConnectionsCheck();

private:

    SettingsModel* _pSettingsModel;
    GraphDataModel* _pGraphDataModel;

};
