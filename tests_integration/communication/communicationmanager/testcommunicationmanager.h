
#include <QObject>
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
    void singleSlaveFail();

    void multiSlaveSuccess();

private:

    void verifyReceivedDataSignal(QList<QVariant> arguments, QList<bool> expResultList, QList<double> expValueList);

    SettingsModel * _pSettingsModel;
    GuiModel * _pGuiModel;
    ErrorLogModel * _pErrorLogModel;

    QList<QUrl> _serverConnectionDataList;
    QList<TestSlaveData *> _testSlaveDataList;
    QList<TestSlaveModbus *> _testSlaveModbusList;
};
