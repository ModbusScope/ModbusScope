#ifndef MOCKDATAPOINTMANAGER_H
#define MOCKDATAPOINTMANAGER_H

#include "ProtocolAdapter/adaptermanager.h"

#include <QJsonObject>
#include <QList>
#include <QStringList>

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

#endif // MOCKDATAPOINTMANAGER_H
