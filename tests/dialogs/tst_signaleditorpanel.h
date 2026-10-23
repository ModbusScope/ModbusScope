
#ifndef TST_SIGNALEDITORPANEL_H
#define TST_SIGNALEDITORPANEL_H

#include "ProtocolAdapter/adaptermanager.h"
#include "ProtocolAdapter/datapointservice.h"
#include "mockadapterhub.h"
#include "models/settingsmodel.h"

#include <QJsonObject>
#include <QObject>

class DataPointDescriptionCache;
class DeviceComboBox;
class GraphDataModel;
class SignalEditorPanel;
class SchemaFormWidget;

/*!
 * \brief Test double for AdapterManager.
 *
 * Captures buildExpression() calls and provides an inject helper to simulate
 * the adapter.buildExpression response without a real adapter process.
 */
class MockAdapterManager : public AdapterManager
{
    Q_OBJECT

public:
    struct BuildCall
    {
        QJsonObject fields;
        QString dataType;
        deviceId_t deviceId;
    };

    explicit MockAdapterManager(SettingsModel* pSettingsModel,
                                const QString& adapterId = QStringLiteral("modbus"),
                                QObject* parent = nullptr)
        : AdapterManager(adapterId, QString(), pSettingsModel, parent)
    {
    }

    int buildExpression(const QJsonObject& fields, const QString& dataType, deviceId_t deviceId) override
    {
        buildCalls.append({ fields, dataType, deviceId });
        return buildCalls.size();
    }

    int describeDataPoint(const QString& address) override
    {
        describeCalls.append(address);
        return cFirstDescribeRequestId + static_cast<int>(describeCalls.size()) - 1;
    }

    void requestExpressionHelp() override
    {
    }

    bool isAdapterReady() const override
    {
        return mockReady;
    }

    //! Simulate the adapter returning an expression string for the last buildExpression() request.
    void injectBuildExpressionResult(const QString& expression)
    {
        emit buildExpressionResult(static_cast<int>(buildCalls.size()), expression);
    }

    //! Simulate the adapter answering the last buildExpression() request with an error.
    void injectBuildExpressionFailure(const QString& error)
    {
        emit dataPointRequestFailed(static_cast<int>(buildCalls.size()), error);
    }

    //! Simulate the adapter describing the data point of the given describeDataPoint() call (0 for the first).
    void injectDescribeResult(int callIndex, const QJsonObject& fields)
    {
        QJsonObject result;
        result["valid"] = true;
        result["description"] = QStringLiteral("described %1").arg(describeCalls.at(callIndex));
        result["fields"] = fields;
        emit describeDataPointResult(cFirstDescribeRequestId + callIndex, result);
    }

    //! Describe request ids, kept apart from the build request ids.
    static constexpr int cFirstDescribeRequestId = 1000;

    QList<BuildCall> buildCalls;
    QStringList describeCalls;
    bool mockReady{ true };
};

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
    MockAdapterManager* _pMockAdapterManager{ nullptr };
    MockAdapterManager* _pMockSimAdapterManager{ nullptr };
    SignalEditorPanel* _pPanel{ nullptr };
};

#endif // TST_SIGNALEDITORPANEL_H
