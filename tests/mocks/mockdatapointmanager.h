#ifndef MOCKDATAPOINTMANAGER_H
#define MOCKDATAPOINTMANAGER_H

#include "ProtocolAdapter/adaptermanager.h"

#include <QJsonObject>
#include <QList>
#include <QStringList>

class SettingsModel;

/*!
 * \brief Test double for AdapterManager.
 *
 * Records describeDataPoint(), buildExpression() and requestExpressionHelp() calls, numbers the
 * requests from 1 like a fresh adapter process would, and lets tests answer them without a real
 * adapter process. Replies are addressed by call, so tests do not depend on the request ids.
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
        int requestId;
    };

    explicit MockDataPointManager(const QString& adapterId, QObject* parent = nullptr)
        : AdapterManager(adapterId, QString(), nullptr, parent)
    {
    }

    MockDataPointManager(const QString& adapterId, SettingsModel* pSettingsModel, QObject* parent = nullptr)
        : AdapterManager(adapterId, QString(), pSettingsModel, parent)
    {
    }

    int buildExpression(const QJsonObject& fields, const QString& dataType, deviceId_t deviceId) override
    {
        const int requestId = nextRequestId();
        buildCalls.append({ fields, dataType, deviceId, requestId });
        return requestId;
    }

    int describeDataPoint(const QString& expression) override
    {
        const int requestId = nextRequestId();
        describeCalls.append(expression);
        describeRequestIds.append(requestId);
        return requestId;
    }

    void requestExpressionHelp() override
    {
        helpRequestCount++;
    }

    bool isAdapterReady() const override
    {
        return mockReady;
    }

    bool isAdapterIdle() const override
    {
        return mockIdle;
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

    //! Simulate the adapter returning an expression for the last buildExpression() call.
    void injectBuildResult(const QString& expression)
    {
        emit buildExpressionResult(buildCalls.last().requestId, expression);
    }

    //! Simulate the adapter answering the last buildExpression() call with an error.
    void injectBuildFailure(const QString& error)
    {
        emit dataPointRequestFailed(buildCalls.last().requestId, error);
    }

    //! Simulate the adapter describing the data point of a describeDataPoint() call (0 for the first).
    void injectDescribeResult(qsizetype callIndex, const QString& description, const QJsonObject& fields = {})
    {
        QJsonObject result;
        result["valid"] = true;
        result["description"] = description;
        if (!fields.isEmpty())
        {
            result["fields"] = fields;
        }
        emit describeDataPointResult(describeRequestIds.at(callIndex), result);
    }

    //! Simulate the adapter rejecting the data point of a describeDataPoint() call (0 for the first).
    void injectDescribeInvalid(qsizetype callIndex, const QString& error)
    {
        QJsonObject result;
        result["valid"] = false;
        result["error"] = error;
        emit describeDataPointResult(describeRequestIds.at(callIndex), result);
    }

    //! Simulate the adapter answering a describeDataPoint() call (0 for the first) with an error.
    void injectDescribeFailure(qsizetype callIndex, const QString& error)
    {
        emit dataPointRequestFailed(describeRequestIds.at(callIndex), error);
    }

    //! Simulate the adapter returning expression help text.
    void injectExpressionHelp(const QString& helpText)
    {
        emit expressionHelpResult(helpText);
    }

    QList<BuildCall> buildCalls;
    QStringList describeCalls;
    QList<int> describeRequestIds;
    int helpRequestCount{ 0 };
    bool mockReady{ true };
    bool mockIdle{ false };
    bool refuseRequests{ false };
    int lastRequestId{ 0 };
};

#endif // MOCKDATAPOINTMANAGER_H
