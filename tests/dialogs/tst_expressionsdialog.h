
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

    int buildExpression(const QJsonObject& fields, const QString& dataType, deviceId_t deviceId) override
    {
        Q_UNUSED(fields);
        Q_UNUSED(dataType);
        buildCalls.append(deviceId);
        return lastBuildRequestId();
    }

    //! Request id of the last buildExpression() call; kept apart from the describe ids.
    int lastBuildRequestId() const
    {
        return cFirstBuildRequestId + static_cast<int>(buildCalls.size()) - 1;
    }

    //! Simulate the adapter returning an expression for the last buildExpression() request.
    void injectBuildResult(const QString& expression)
    {
        emit buildExpressionResult(lastBuildRequestId(), expression);
    }

    //! Simulate the adapter answering the last buildExpression() request with an error.
    void injectBuildFailure(const QString& error)
    {
        emit dataPointRequestFailed(lastBuildRequestId(), error);
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

    //! Simulate the adapter rejecting the data point of the request with the given id.
    void injectDescribeInvalid(int requestId, const QString& error)
    {
        QJsonObject result;
        result["valid"] = false;
        result["error"] = error;
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

    static constexpr int cFirstBuildRequestId = 1000;

    QStringList describeCalls;
    QList<deviceId_t> buildCalls;
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
    void acceptDisabledOnInvalidDataPoint();
    void invalidDataPointShownOverExampleError();
    void acceptEnabledAfterInvalidDataPointRemoved();
    void acceptEnabledOnFailedDescribe();

    void insertFormCreatedWhenShown();
    void insertFormStartsOnFirstDataPointDevice();
    void insertAtCursor();
    void insertReplacesSelection();
    void insertDisabledUntilBuilt();
    void insertFailureShowsError();
    void enterInInsertFormInserts();

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
