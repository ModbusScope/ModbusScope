
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
    void singleSlaveFail();

private:

    void verifyReceivedDataSignal(QList<QVariant> arguments, QList<bool> expResultList, QList<double> expValueList);

    QPointer<TestSlaveData> _pTestSlaveData;
    QPointer<TestSlaveModbus> _pTestSlaveModbus;

    SettingsModel * _pSettingsModel;
    GuiModel * _pGuiModel;
    ErrorLogModel * _pErrorLogModel;

    QUrl * _pServerConnectionData;

};
