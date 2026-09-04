
#ifndef TST_REGISTERDIALOG_H
#define TST_REGISTERDIALOG_H

#include "ProtocolAdapter/adaptermanager.h"
#include "mockadapterhub.h"
#include "models/settingsmodel.h"

#include <QJsonObject>
#include <QObject>

class GraphDataModel;
class RegisterDialog;

/*!
 * \brief Test double for AdapterManager.
 *
 * Captures buildExpression() calls and provides an inject helper to simulate
 * the adapter session starting, without a real adapter process.
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

    void buildExpression(const QJsonObject& fields, const QString& dataType, deviceId_t deviceId) override
    {
        buildCalls.append({ fields, dataType, deviceId });
    }

    bool isAdapterActive() const override
    {
        return mockActive;
    }

    bool isAdapterReady() const override
    {
        return mockReady;
    }

    //! Simulate the adapter session becoming ready, which RegisterDialog waits for
    //! before requesting its default expression.
    void injectSessionStarted()
    {
        emit sessionStarted();
    }

    QList<BuildCall> buildCalls;
    bool mockActive{ false };
    bool mockReady{ false };
};

class TestRegisterDialog : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void defaultRegisterUsesLowestConfiguredDeviceId();
    void defaultRegisterUsesDeviceOneWhenPresent();
    void defaultRegisterSkipsBuildExpressionWhenNoDevices();
    void defaultRegisterRequestedImmediatelyWhenSessionAlreadyActive();
    void defaultRegisterRequestedImmediatelyWhenAdapterAlreadyReady();
    void defaultRegisterRequestedAgainWhenSessionRestartsAfterAlreadyActive();

    void dataPointLimitWarningHiddenWhenWithinLimit();
    void dataPointLimitWarningShownWhenExceeded();
    void dataPointLimitWarningHiddenWhenSignalDeactivated();
    void dataPointLimitWarningHiddenWhenAdapterReportsNoLimit();

private:
    static QJsonObject buildTestRegisterSchema();
    static QJsonObject buildDescribeWithMaxRegisters(int maxRegisters);
    void seedSignals(int count);

    SettingsModel _settingsModel;
    GraphDataModel* _pGraphDataModel{ nullptr };
    MockAdapterHub* _pMockHub{ nullptr };
    MockAdapterManager* _pMockAdapterManager{ nullptr };
    RegisterDialog* _pDialog{ nullptr };
};

#endif // TST_REGISTERDIALOG_H
