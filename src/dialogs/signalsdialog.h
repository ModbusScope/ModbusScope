#ifndef SIGNALSDIALOG_H
#define SIGNALSDIALOG_H

#include "customwidgets/centeredbox.h"
#include "models/device.h"

#include <QDialog>
#include <QList>
#include <QWidgetAction>

/* Forward declaration */
class GraphDataModel;
class AdapterHub;
class AdapterManager;
class SettingsModel;
class ValueAxisDelegate;
class ActionButtonDelegate;
class GraphData;

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

private slots:
    void addSignal(const GraphData& graphData);
    void addDefaultSignal();
    void removeSelectedSignals();
    void activatedCell(QModelIndex modelIndex);
    void onSignalInserted(const QModelIndex& parent, int first, int last);
    void handleExpressionEdit(const QModelIndex& index);
    void requestDefaultExpression();
    void onDefaultExpressionBuilt(int requestId, const QString& expression);
    void updateDataPointLimitIndication();

private:
    int selectedRowAfterDelete(int deletedStartIndex, int rowCnt);
    void initDefaultExpressionTarget(const QList<deviceId_t>& deviceIds);
    void setupDataPointLimitIndication();
    QString dataPointLimitWarningMessage() const;
    static bool sortRegistersLastFirst(const QModelIndex& s1, const QModelIndex& s2);

    Ui::SignalsDialog* _pUi;

    GraphDataModel* _pGraphDataModel;
    SettingsModel* _pSettingsModel;
    AdapterHub* _pAdapterHub;
    AdapterManager* _pDefaultExpressionManager;
    deviceId_t _defaultExpressionDeviceId{ 0 };

    CenteredBoxProxyStyle _centeredBoxStyle;

    std::unique_ptr<ValueAxisDelegate> _valueAxisDelegate;
    std::unique_ptr<ActionButtonDelegate> _expressionDelegate;
    std::unique_ptr<QWidgetAction> _addSignalAction;
};

#endif // SIGNALSDIALOG_H
