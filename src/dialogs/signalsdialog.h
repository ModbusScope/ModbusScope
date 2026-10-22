#ifndef SIGNALSDIALOG_H
#define SIGNALSDIALOG_H

#include "customwidgets/centeredbox.h"
#include "models/device.h"

#include <QDialog>
#include <QList>

/* Forward declaration */
class GraphDataModel;
class AdapterHub;
class AdapterManager;
class DataPointDescriptionCache;
class DataPointService;
class SettingsModel;
class SignalEditorPanel;
class SignalListDraft;
class ValueAxisDelegate;
class ActionButtonDelegate;

namespace Ui {
class SignalsDialog;
}

class SignalsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SignalsDialog(GraphDataModel* pGraphDataModel,
                           SettingsModel* pSettingsModel,
                           AdapterHub* pAdapterHub,
                           QWidget* parent = nullptr);
    ~SignalsDialog();

public slots:
    void accept() override;

private slots:
    void startAddSignal();
    void removeSelectedSignals();
    void activatedCell(QModelIndex modelIndex);
    void onSignalInserted(const QModelIndex& parent, int first, int last);
    void handleExpressionEdit(const QModelIndex& index);
    void requestDefaultExpression();
    void onDefaultExpressionBuilt(quint64 ticket, const QString& expression);
    void updateDataPointLimitIndication();

private:
    void initDefaultExpressionTarget(const QList<deviceId_t>& deviceIds);
    void setupEditorPanel();
    void setupDataPointLimitIndication();
    QString dataPointLimitWarningMessage() const;

    Ui::SignalsDialog* _pUi;

    SignalListDraft* _pDraft;
    SettingsModel* _pSettingsModel;
    AdapterHub* _pAdapterHub;
    DataPointService* _pDataPointService;
    DataPointDescriptionCache* _pDescriptionCache;
    SignalEditorPanel* _pPanel{ nullptr };
    AdapterManager* _pDefaultExpressionManager{ nullptr };
    deviceId_t _defaultExpressionDeviceId{ 0 };
    quint64 _defaultExpressionTicket{ 0 };

    CenteredBoxProxyStyle _centeredBoxStyle;

    std::unique_ptr<ValueAxisDelegate> _valueAxisDelegate;
    std::unique_ptr<ActionButtonDelegate> _expressionDelegate;

    static constexpr int cMinimumWidth = 680;
    static constexpr int cMinimumHeight = 400;
};

#endif // SIGNALSDIALOG_H
