#ifndef TST_DATAPOINTSERVICE_H
#define TST_DATAPOINTSERVICE_H

#include "../mocks/mockdatapointmanager.h"
#include "models/settingsmodel.h"

#include <QJsonObject>
#include <QObject>

class DataPointService;
class MockAdapterHub;

class TestDataPointService : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void requestsRoutedToDeviceAdapter();
    void describeResultCarriesTicket();
    void interleavedRepliesMatchTheirTickets();
    void sameRequestIdOnTwoAdaptersKeptApart();
    void unknownDeviceFailsOnNextEventLoopTurn();
    void adapterNotRunningFails();
    void refusedRequestFails();
    void adapterErrorFails();
    void failedTicketNotFailedAgainByTimeout();
    void queuedFailuresKeepTicketOrder();
    void timeoutFails();
    void replyAfterTimeoutIgnored();
    void destroyedManagerFailsPendingRequests();
    void buildExpressionSplitsFields();
    void addressSchemaStripsDeviceId();
    void defaultsReturned();
    void unknownAdapterAddsNoEntry();

private:
    static QJsonObject buildSchemaWithDeviceId();

    SettingsModel* _pSettingsModel{ nullptr };
    MockAdapterHub* _pMockHub{ nullptr };
    MockDataPointManager* _pModbusManager{ nullptr };
    MockDataPointManager* _pSimManager{ nullptr };
    DataPointService* _pService{ nullptr };
};

#endif // TST_DATAPOINTSERVICE_H
