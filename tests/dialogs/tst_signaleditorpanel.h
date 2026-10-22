
#ifndef TST_SIGNALEDITORPANEL_H
#define TST_SIGNALEDITORPANEL_H

#include "ProtocolAdapter/adaptermanager.h"
#include "ProtocolAdapter/datapointservice.h"
#include "mockadapterhub.h"
#include "models/settingsmodel.h"

#include <QJsonObject>
#include <QObject>

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

    QList<BuildCall> buildCalls;
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

private:
    void clickAdd();
    void addSignal(const QString& expression);
    void setAddress(const QJsonObject& values);
    void recreatePanel();
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
    MockAdapterManager* _pMockAdapterManager{ nullptr };
    MockAdapterManager* _pMockSimAdapterManager{ nullptr };
    SignalEditorPanel* _pPanel{ nullptr };
};

#endif // TST_SIGNALEDITORPANEL_H
