
#include <QObject>
#include <QPointer>
#include <QUrl>
#include "communicationmanager.h"

#include "settingsmodel.h"
#include "guimodel.h"
#include "graphdatamodel.h"
#include "errorlogmodel.h"

#include "testslavedata.h"
#include "testslavemodbus.h"

class TestCommunicationManager: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void singleSlaveSuccess();

private:

    QPointer<TestSlaveData> _pTestSlaveData;
    QPointer<TestSlaveModbus> _pTestSlaveModbus;

    SettingsModel _settingsModel;
    GuiModel _guiModel;
    GraphDataModel _graphDataModel;
    ErrorLogModel _errorLogModel;

    QUrl _serverConnectionData;

};
