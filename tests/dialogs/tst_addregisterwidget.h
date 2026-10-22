
#ifndef TST_ADDREGISTERWIDGET_H
#define TST_ADDREGISTERWIDGET_H

#include "ProtocolAdapter/adaptermanager.h"
#include "ProtocolAdapter/datapointservice.h"
#include "mockadapterhub.h"
#include "models/graphdata.h"
#include "models/settingsmodel.h"

#include <QJsonObject>
#include <QObject>

class AddRegisterWidget;
class DeviceComboBox;
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

class TestAddRegisterWidget : public QObject
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
    void emptyNameAddsSignalWithoutLabel();
    void nameClearedAfterAdd();
    void formResetAfterAdd();
    void buildFailureShowsReasonInOpenMenu();
    void longBuildErrorWrapsInOpenMenu();
    void emptyBuildResultShowsReason();
    void buildFailureWithoutReasonShowsFallback();
    void buildErrorIsPlainText();
    void fieldChangeHidesBuildError();
    void deviceSwitchHidesBuildError();

    void deviceComboVisibleWithSingleDevice();
    void deviceComboListsDevices();
    void switchDeviceRebuildsSchema();
    void switchDeviceWhileMenuOpenResizesPopup();
    void switchToLargerSchemaWhileMenuOpenShowsAllFields();
    void buildExpressionRoutedToSelectedDevice();
    void btnAddDisabledWhenSelectedDeviceAdapterUnavailable();
    void deviceComboRefreshesLiveOnDeviceAdded();
    void deviceAddedToEmptyListAppliesConfiguredAdapter();
    void deviceComboDisablesAddWhenLastDeviceRemoved();
    void addRegisterClosesContainingMenu();
    void emptyExpressionResponseLeavesContainingMenuOpen();

private:
    void clickAdd();
    void addRegister(GraphData& graphData, const QString& expression);
    void setAddress(const QJsonObject& values);
    bool hasLabelWithText(const QString& text) const;
    deviceId_t addSimAdapter();
    static DeviceComboBox* deviceCombo(AddRegisterWidget* pWidget);
    static SchemaFormWidget* addressForm(AddRegisterWidget* pWidget);
    static QJsonObject buildAddressSchema();
    static QJsonObject buildTestRegisterSchema();
    static QJsonObject buildSimRegisterSchema();

    SettingsModel _settingsModel;
    MockAdapterHub* _pMockHub{ nullptr };
    DataPointService* _pDataPointService{ nullptr };
    MockAdapterManager* _pMockAdapterManager{ nullptr };
    MockAdapterManager* _pMockSimAdapterManager{ nullptr };
    AddRegisterWidget* _pRegWidget{ nullptr };
};

#endif // TST_ADDREGISTERWIDGET_H
