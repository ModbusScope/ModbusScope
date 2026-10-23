
#ifndef TST_EXPRESSIONSDIALOG_H
#define TST_EXPRESSIONSDIALOG_H

#include "../mocks/mockadapterhub.h"
#include "../mocks/mockdatapointmanager.h"
#include "ProtocolAdapter/datapointservice.h"

#include <QJsonObject>
#include <QObject>

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
    void exampleInputAcceptsDecimalComma();

    void acceptDisabledOnSyntaxError();
    void acceptDisabledOnEmptyExpression();
    void acceptDisabledOnUnknownDevice();
    void unknownDeviceShownWithInvalidExampleValue();
    void acceptEnabledOnDivisionByZeroExample();
    void acceptEnabledOnInvalidExampleValue();
    void acceptAppliesValidExpression();
    void acceptDisabledOnInvalidDataPoint();
    void invalidDataPointShownOverExampleError();
    void adapterErrorShownAsPlainText();
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
    MockDataPointManager* addSimAdapter();
    void recreateDialog();

    GraphDataModel* _pGraphDataModel{ nullptr };
    SettingsModel* _pSettingsModel{ nullptr };
    MockAdapterHub* _pMockHub{ nullptr };
    DataPointService* _pDataPointService{ nullptr };
    DataPointDescriptionCache* _pDescriptionCache{ nullptr };
    MockDataPointManager* _pMockDataPointManager{ nullptr };
    ExpressionsDialog* _pDialog{ nullptr };
};

#endif // TST_EXPRESSIONSDIALOG_H
