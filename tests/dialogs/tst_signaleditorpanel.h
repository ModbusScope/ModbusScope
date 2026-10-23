
#ifndef TST_SIGNALEDITORPANEL_H
#define TST_SIGNALEDITORPANEL_H

#include "../mocks/mockadapterhub.h"
#include "../mocks/mockdatapointmanager.h"
#include "ProtocolAdapter/datapointservice.h"
#include "models/settingsmodel.h"

#include <QJsonObject>
#include <QObject>

class DataPointDescriptionCache;
class DeviceComboBox;
class GraphDataModel;
class SignalEditorPanel;
class SchemaFormWidget;

class TestSignalEditorPanel : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void registerDefault();
    void registerType();
    void registerObjectType();
    void registerDevice();
    void registerValueAxis();
    void addDisabledUntilFirstBuildReply();
    void addDisabledWhileBuildPending();
    void addModeShowsBuiltExpression();
    void buildExpressionFailureKeepsFormUsable();
    void emptyNameAddsSignalWithDefaultLabel();
    void nameClearedAfterAdd();
    void addKeepsDeviceFieldsAndAxis();
    void consecutiveAddsWithoutReopening();
    void startAddClearsNameAndFocusesIt();
    void enterAddsSignal();
    void enterInAddressFieldAddsSignal();
    void enterIgnoredWhileBuildPending();

    void statusNamesAddedSignal();
    void buildErrorReplacesAddedStatus();
    void fieldChangeClearsAddedStatus();
    void emptyBuildResultShowsReason();
    void buildFailureWithoutReasonShowsFallback();
    void statusIsPlainText();
    void fieldChangeHidesBuildError();
    void deviceSwitchHidesBuildError();
    void noDeviceDisablesAddWithHint();

    void buildExpressionRoutedToSelectedDevice();
    void btnAddDisabledWhenSelectedDeviceAdapterUnavailable();
    void deviceAddedToEmptyListAppliesConfiguredAdapter();
    void deviceComboDisablesAddWhenLastDeviceRemoved();

    void editModeLoadsReferenceWhenDescribed();
    void editAddressOfSingleReference();
    void editKeepsArithmeticAroundReference();
    void editSecondOfTwoIdenticalReferences();
    void selectingDataPointLoadsIt();
    void syntaxErrorShowsOnlyErrorAndEditExpression();
    void editExpressionRequestsDialogForRow();
    void nameAndAxisEditRow();
    void emptyNameRestoresLabel();
    void enterInEditModeAddsNothing();
    void deviceSwitchReplacesReferenceWithDefault();
    void removedRowReturnsToAddMode();
    void outsideExpressionChangeReloadsPanel();
    void showAddModeClearsNameAndShowsAdd();
    void descriptionAfterOwnWriteKeepsForm();
    void expressionLabelIsEscaped();
    void expressionWithoutDataPointShowsHint();

private:
    void clickAdd();
    void addSignal(const QString& expression);
    void setAddress(const QJsonObject& values);
    void recreatePanel();
    void editNewSignal(const QString& expression);
    static QJsonObject holdingRegisterFields(int address);
    void describeCall(qsizetype callIndex, const QJsonObject& fields);
    bool hasLabelWithText(const QString& text) const;
    deviceId_t addSimAdapter();
    static DeviceComboBox* deviceCombo(SignalEditorPanel* pPanel);
    static SchemaFormWidget* addressForm(SignalEditorPanel* pPanel);
    static QJsonObject buildAddressSchema();
    static QJsonObject buildTestRegisterSchema();
    static QJsonObject buildSimRegisterSchema();

    SettingsModel _settingsModel;
    GraphDataModel* _pGraphDataModel{ nullptr };
    MockAdapterHub* _pMockHub{ nullptr };
    DataPointService* _pDataPointService{ nullptr };
    DataPointDescriptionCache* _pDescriptionCache{ nullptr };
    MockDataPointManager* _pMockDataPointManager{ nullptr };
    MockDataPointManager* _pMockSimAdapterManager{ nullptr };
    SignalEditorPanel* _pPanel{ nullptr };
};

#endif // TST_SIGNALEDITORPANEL_H
