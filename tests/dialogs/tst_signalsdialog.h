
#ifndef TST_SIGNALSDIALOG_H
#define TST_SIGNALSDIALOG_H

#include "ProtocolAdapter/adaptermanager.h"
#include "mockadapterhub.h"
#include "models/settingsmodel.h"

#include <QJsonObject>
#include <QObject>

class GraphDataModel;
class SignalEditorPanel;
class SignalsDialog;

/*!
 * \brief Test double for AdapterManager.
 *
 * Captures buildExpression() calls and provides an inject helper to answer
 * them, without a real adapter process.
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

    bool isAdapterReady() const override
    {
        return mockReady;
    }

    //! Simulate the adapter returning an expression string for the last buildExpression() request.
    void injectBuildExpressionResult(const QString& expression)
    {
        emit buildExpressionResult(static_cast<int>(buildCalls.size()), expression);
    }

    QList<BuildCall> buildCalls;
    bool mockReady{ false };
};

class TestSignalsDialog : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void dataPointLimitWarningHiddenWhenWithinLimit();
    void dataPointLimitWarningShownWhenExceeded();
    void dataPointLimitWarningHiddenWhenSignalDeactivated();
    void dataPointLimitWarningHiddenWhenAdapterReportsNoLimit();

    void buttonBoxHasOkAndCancel();
    void tableEditsDraftNotSource();
    void cancelDiscardsChanges();
    void escapeDiscardsChanges();
    void okAppliesChanges();
    void okWithoutChangesLeavesSourceUntouched();

    void addButtonClearsSelectionAndFocusesName();
    void addedSignalScrolledIntoViewNotSelected();
    void enterInPanelDoesNotCloseDialog();
    void deleteInNameFieldKeepsRows();
    void cancelDiscardsPanelAdds();
    void minimumSizeKeepsTableAndPanelUsable();

private:
    static QJsonObject buildTestRegisterSchema();
    static QJsonObject buildDescribeWithMaxRegisters(int maxRegisters);
    void seedSignals(int count);
    void seedSamples();
    void editDraft();
    GraphDataModel* draftModel() const;
    SignalEditorPanel* panel() const;
    void openWithReadyAdapter(int signalCount);
    void addSignalThroughPanel(const QString& expression);

    SettingsModel _settingsModel;
    GraphDataModel* _pGraphDataModel{ nullptr };
    MockAdapterHub* _pMockHub{ nullptr };
    MockAdapterManager* _pMockAdapterManager{ nullptr };
    SignalsDialog* _pDialog{ nullptr };
};

#endif // TST_SIGNALSDIALOG_H
