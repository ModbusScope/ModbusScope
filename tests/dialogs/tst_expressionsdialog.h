
#ifndef TST_EXPRESSIONSDIALOG_H
#define TST_EXPRESSIONSDIALOG_H

#include "ProtocolAdapter/adaptermanager.h"
#include "ProtocolAdapter/datapointservice.h"
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

    bool isAdapterReady() const override
    {
        return !mockIdle;
    }

    //! Simulate the adapter returning a description for the request with the given id (1 for the first call).
    void injectDescribeResult(int requestId, const QString& description)
    {
        QJsonObject result;
        result["valid"] = true;
        result["description"] = description;
        emit describeDataPointResult(requestId, result);
    }

    //! Simulate the adapter answering the request with the given id with an error.
    void injectDescribeFailure(int requestId, const QString& error)
    {
        emit dataPointRequestFailed(requestId, error);
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

class DataPointDescriptionCache;
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
    void describeDispatch_replyForRemovedReferenceIgnored();
    void describeDispatch_cachedAcrossExpressionChanges();
    void describeDispatch_failureKeepsParserDescription();
    void describeDispatch_duplicateReferences();

    void rowsFollowExpressionOrder();
    void duplicateReferencesShareExampleValue();
    void exampleValuesFollowDistinctDataPoints();

    void describeRoutedPerDeviceAdapter();
    void describeSkipsUnavailableAdapter();
    void describeSkipsIdleAdapter();
    void describeShowsInvalidDeviceMessage();
    void helpComboHiddenWithSingleDevice();
    void helpComboVisibleWithTwoDevices();
    void helpComboShowsNameProtocolAndId();
    void helpRoutedToSelectedDevice();
    void helpSwitchingBetweenDevicesOnSameAdapterSkipsRedundantRequest();

    void exampleInputDefaultsToOne();
    void exampleInputAcceptsHex();

    void acceptDisabledOnSyntaxError();
    void acceptDisabledOnEmptyExpression();
    void acceptDisabledOnUnknownDevice();
    void unknownDeviceShownWithInvalidExampleValue();
    void acceptEnabledOnDivisionByZeroExample();
    void acceptEnabledOnInvalidExampleValue();
    void acceptAppliesValidExpression();

    void buttonBoxHasAcceptAndCancel();
    void enterInExampleTableIgnored();
    void enterOnCancelCancels();
    void escapeCancels();
    void cancelLeavesExpressionUnchanged();
    void enterAcceptsValidExpression();
    void enterIgnoredOnSyntaxError();
    void shiftEnterIgnored();
    void expressionEditorWraps();

private:
    MockAdapterManager* addSimAdapter();
    void recreateDialog();

    GraphDataModel* _pGraphDataModel{ nullptr };
    SettingsModel* _pSettingsModel{ nullptr };
    MockAdapterHub* _pMockHub{ nullptr };
    DataPointService* _pDataPointService{ nullptr };
    DataPointDescriptionCache* _pDescriptionCache{ nullptr };
    MockAdapterManager* _pMockAdapterManager{ nullptr };
    ExpressionsDialog* _pDialog{ nullptr };
};

#endif // TST_EXPRESSIONSDIALOG_H
