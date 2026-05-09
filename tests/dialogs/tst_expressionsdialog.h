
#ifndef TST_EXPRESSIONSDIALOG_H
#define TST_EXPRESSIONSDIALOG_H

#include "ProtocolAdapter/adaptermanager.h"

#include <QJsonObject>
#include <QObject>

/*!
 * \brief Test double for AdapterManager.
 *
 * Captures describeDataPoint() calls and provides an inject helper to simulate
 * the adapter.describeDataPoint response without a real adapter process.
 */
class MockAdapterManager : public AdapterManager
{
    Q_OBJECT

public:
    explicit MockAdapterManager(QObject* parent = nullptr) : AdapterManager(nullptr, parent)
    {
    }

    void describeDataPoint(const QString& address) override
    {
        describeCalls.append(address);
    }

    void requestExpressionHelp() override
    {
    }

    bool isAdapterIdle() const override
    {
        return false;
    }

    //! Simulate the adapter returning a description for the last requested data point.
    void injectDescribeResult(const QString& description)
    {
        QJsonObject result;
        result["description"] = description;
        emit describeDataPointResult(result);
    }

    QStringList describeCalls;
};

class GraphDataModel;
class ExpressionsDialog;

class TestExpressionsDialog : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void describeDispatch_singleAddress();
    void describeDispatch_multipleAddresses();
    void describeDispatch_abortsOnExpressionChange();

private:
    GraphDataModel* _pGraphDataModel{ nullptr };
    MockAdapterManager* _pMockAdapterManager{ nullptr };
    ExpressionsDialog* _pDialog{ nullptr };
};

#endif // TST_EXPRESSIONSDIALOG_H
