#ifndef TST_DATAPOINTSERVICE_H
#define TST_DATAPOINTSERVICE_H

#include "ProtocolAdapter/adaptermanager.h"
#include "models/settingsmodel.h"

#include <QJsonObject>
#include <QObject>

class DataPointService;
class MockAdapterHub;

/*!
 * \brief Test double for AdapterManager.
 *
 * Records describeDataPoint() and buildExpression() calls, numbers them from 1 like a fresh
 * adapter process would, and lets tests answer them without a real adapter process.
 */
class MockDataPointManager : public AdapterManager
{
    Q_OBJECT

public:
    struct BuildCall
    {
        QJsonObject fields;
        QString dataType;
        deviceId_t deviceId;
    };

    explicit MockDataPointManager(const QString& adapterId, QObject* parent = nullptr)
        : AdapterManager(adapterId, QString(), nullptr, parent)
    {
    }

    int buildExpression(const QJsonObject& fields, const QString& dataType, deviceId_t deviceId) override
    {
        buildCalls.append({ fields, dataType, deviceId });
        return nextRequestId();
    }

    int describeDataPoint(const QString& expression) override
    {
        describeCalls.append(expression);
        return nextRequestId();
    }

    bool isAdapterReady() const override
    {
        return mockReady;
    }

    bool isAdapterActive() const override
    {
        return false;
    }

    //! Returns the id of the next request, or -1 while requests are refused.
    int nextRequestId()
    {
        if (refuseRequests)
        {
            return -1;
        }
        return ++lastRequestId;
    }

    QList<BuildCall> buildCalls;
    QStringList describeCalls;
    bool mockReady{ true };
    bool refuseRequests{ false };
    int lastRequestId{ 0 };
};

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
