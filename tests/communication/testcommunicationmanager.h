
#include <QObject>
#include <QUrl>
#include "communicationmanager.h"

#include "settingsmodel.h"
#include "guimodel.h"
#include "graphdatamodel.h"
#include "diagnosticmodel.h"

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
    void singleSlaveCheckProcessing();

    void multiSlaveSuccess();
    void multiSlaveSuccess_2();
    void multiSlaveSuccess_3();
    void multiSlaveSingleFail();
    void multiSlaveAllFail();
    void multiSlaveDisabledConnection();

private:

    void verifyReceivedDataSignal(QList<QVariant> arguments, QList<bool> expResultList, QList<double> expValueList);

    SettingsModel * _pSettingsModel;
    GuiModel * _pGuiModel;

    QList<QUrl> _serverConnectionDataList;
    QList<TestSlaveData *> _testSlaveDataList;
    QList<TestSlaveModbus *> _testSlaveModbusList;
};
