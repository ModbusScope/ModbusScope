
#ifndef TST_EXPRESSIONSDIALOG_H
#define TST_EXPRESSIONSDIALOG_H

#include "ProtocolAdapter/adaptermanager.h"
#include "mockadapterhub.h"

#include <QJsonObject>
#include <QObject>

/*!
 * \brief Test double for AdapterManager.
 *
 * Captures describeDataPoint() and requestExpressionHelp() calls and provides
 * inject helpers to simulate the adapter responses without a real adapter process.
 */
class MockAdapterManager : public AdapterManager
{
    Q_OBJECT

public:
    explicit MockAdapterManager(const QString& adapterId = QStringLiteral("modbus"), QObject* parent = nullptr)
        : AdapterManager(adapterId, QString(), nullptr, parent)
    {
    }

    int describeDataPoint(const QString& address) override
    {
        describeCalls.append(address);
        return static_cast<int>(describeCalls.size());
    }

    void requestExpressionHelp() override
    {
        helpRequestCount++;
    }

    bool isAdapterIdle() const override
    {
        return mockIdle;
    }

    //! Simulate the adapter returning a description for the last requested data point.
    void injectDescribeResult(const QString& description)
    {
        QJsonObject result;
        result["description"] = description;
        emit describeDataPointResult(static_cast<int>(describeCalls.size()), result);
    }

    //! Simulate the adapter returning expression help text.
    void injectExpressionHelp(const QString& helpText)
    {
        emit expressionHelpResult(helpText);
    }

    QStringList describeCalls;
    int helpRequestCount{ 0 };
    bool mockIdle{ false };
};

class GraphDataModel;
class ExpressionsDialog;
class SettingsModel;

class TestExpressionsDialog : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void describeDispatch_singleAddress();
    void describeDispatch_multipleAddresses();
    void describeDispatch_abortsOnExpressionChange();

    void describeRoutedPerDeviceAdapter();
    void describeSkipsUnavailableAdapter();
    void describeSkipsIdleAdapter();
    void describeShowsInvalidDeviceMessage();
    void helpComboHiddenWithSingleDevice();
    void helpComboVisibleWithTwoDevices();
    void helpComboShowsNameProtocolAndId();
    void helpRoutedToSelectedDevice();
    void helpSwitchingBetweenDevicesOnSameAdapterSkipsRedundantRequest();

private:
    MockAdapterManager* addSimAdapter();
    void recreateDialog();

    GraphDataModel* _pGraphDataModel{ nullptr };
    SettingsModel* _pSettingsModel{ nullptr };
    MockAdapterHub* _pMockHub{ nullptr };
    MockAdapterManager* _pMockAdapterManager{ nullptr };
    ExpressionsDialog* _pDialog{ nullptr };
};

#endif // TST_EXPRESSIONSDIALOG_H
